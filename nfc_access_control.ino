/*******************************************************************************
 * The MIT License (MIT)
 * Copyright (c) 2015 Diego Fernández Barrera (bigomby@gmail.com)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 ******************************************************************************/

#include <EEPROM.h>
#include <Adafruit_NFCShield_I2C.h>
#include <Wire.h>
#include <NfcAccessControl.h>

#define IRQ     (2)
#define RESET   (3)
#define SRX     (8)
#define STX     (9)

#define INIT_NORMAL     100
#define NORMAL          101
#define INIT_MASTER     200
#define MASTER          201
#define INIT_LEARN      300
#define LEARN           301
#define INIT_CLEAR      400
#define CLEAR           401

#define UID_BYTES       4
#define READ_INTERVAL   2000

/*****************************************************************************
* External libraries
******************************************************************************/
Adafruit_NFCShield_I2C nfc (IRQ, RESET);
NfcAccessControl access;

/*****************************************************************************
* Globals
******************************************************************************/
int buzzer = 11;            // Buzzer
int greenLed = 12;          // Green LED pin
int redLed = 13;            // Red LED pin

char uid[UID_LENGTH];       // Store the last readed UID
char uid_aux[UID_LENGTH];   // Used for confirm the last readed UID
char suid[UID_LENGTH + 1];  // String to print UIDs (1 more char for '\0')
char *puid = uid;           // Point to uid
char *puid_aux = uid_aux;   // Point to uid_aux

long readMillis = 0;        // Temporizing for reading function
long feedbackMillis = 0;    // Temporizing for feedback functions

int mode = 100;             // Starting in normal mode

/*****************************************************************************
* Setting up
******************************************************************************/
void setup() {

	Serial.begin (9600);

	nfc.begin();

	uint32_t versiondata = nfc.getFirmwareVersion();
	if (! versiondata) {
		Serial.print ("NFC Reader not found");
	}

	Serial.print ("Found PN5"); Serial.println ((versiondata >> 24) & 0xFF, HEX);
	Serial.print ("Firmware ver. "); Serial.print ((versiondata >> 16) & 0xFF, DEC);
	Serial.print ('.'); Serial.println ((versiondata >> 8) & 0xFF, DEC);

	nfc.setPassiveActivationRetries (1);

	nfc.SAMConfig();

	pinMode (11, OUTPUT);
	pinMode (12, OUTPUT);
	pinMode (13, OUTPUT);

	digitalWrite (11, HIGH);
	digitalWrite (12, HIGH);
	digitalWrite (13, HIGH);
}

/*****************************************************************************
* Loop function
******************************************************************************/
void loop() {
	switch (mode) {
	case INIT_NORMAL:
		init_normal();
		mode = NORMAL;
		break;
	case NORMAL:
		mode_normal();
		break;
	case INIT_LEARN:
		init_learn();
		mode = LEARN;
		break;
	case LEARN:
		mode_learn_feedback();
		mode_learn();
		break;
	case INIT_MASTER:
		init_master();
		mode = MASTER;
		break;
	case MASTER:
		mode_master_feedback();
		mode_master();
		break;
	case INIT_CLEAR:
		init_clear();
		mode = CLEAR;
		break;
	case CLEAR:
		mode_clear_feedback();
		mode_clear();
		break;
	default:
		mode = INIT_NORMAL;
	}
}

/*****************************************************************************
 * Function that handles the reading from the NFC READER
 *****************************************************************************/
uint8_t readUid (char **r_uid) {

	boolean success;
	uint8_t uidLength;
	int ret = 0;
	char *uid_byte = NULL;
	char *tmp_uid = NULL;
	uint8_t *buff = NULL;

	if (r_uid != NULL) {

		if (millis() - readMillis > READ_INTERVAL) {
			buff = (uint8_t *) calloc (UID_LENGTH, sizeof (uint8_t));
			success = nfc.readPassiveTargetID (PN532_MIFARE_ISO14443A, &buff[0],
			                                   &uidLength);

			if (success && uidLength == UID_BYTES) {
				uid_byte = (char *) calloc (4, sizeof (char));
				tmp_uid = (char *) calloc (12, sizeof (char));

				for (uint8_t i = 0 ; i < 4 ; i++) {
					snprintf (uid_byte, 3 , "%02X", buff[i]);
					strncat (tmp_uid, uid_byte, 2);
				}

				strncpy (*r_uid, tmp_uid, UID_LENGTH);
				free (uid_byte);
				free (tmp_uid);
				ret = 1;
				readMillis = millis();
			} else {
				ret = 0;
			}

			free (buff);
		}
	} else {
		Serial.println ("memory error at readUid()");
		mode = INIT_NORMAL;
	}

	return ret;
}

/*****************************************************************************
 * Check the access permission of the last readed UID.
 *****************************************************************************/
void checkUid() {

	switch (access.checkUid (uid)) {
	case UNAUTHORIZED:
		fail();
		break;
	case AUTHORIZED:
		success();
		break;
	case MASTER_ACCESS:
		mode = INIT_MASTER;
		break;
	}
}

/*****************************************************************************
 *                                  MODES
 *****************************************************************************/


/*****************************************************************************
 * MODE: NORMAL
 * In this mode, the system is waiting for a TAG to be read.
 *****************************************************************************/

void init_normal() {

	// Turn off all the LEDs and Buzzer
	digitalWrite (redLed, HIGH);
	digitalWrite (greenLed, HIGH);
	digitalWrite (buzzer, HIGH);

	Serial.print ("There are ");
	Serial.print (access.getAllowedTagsCount());
	Serial.print (" allowed TAGs.");
	Serial.println ("");

	Serial.println ("Waiting for TAG:");
}

void mode_normal() {

	if (puid != NULL) {
		if (readUid (&puid) > 0) {
			strncpy (suid, uid, UID_LENGTH);
			Serial.print ("UID: "); Serial.println (suid);
			checkUid();
		}
	} else {
		Serial.println ("memory error at mode_normal()");
		mode = INIT_NORMAL;
	}
}

void mode_normal_feedback() {}

/*****************************************************************************
 * MODE: MASTER
 * In this mode is possible to enter in LEARN or CLEAR mode.
 *****************************************************************************/

void init_master() {

	uint8_t allowed_tags_count = access.getAllowedTagsCount();

	Serial.println ("MODE: MASTER");
	Serial.println ("");

	Serial.print ("There are "); Serial.print (allowed_tags_count);
	Serial.print (" allowed TAGs.");
	Serial.println ("");

	for (uint8_t i = 0 ; i < allowed_tags_count ; i++) {
		strncpy (suid, access.getUid (i), UID_LENGTH);
		Serial.println (suid);
	}
}

void mode_master() {

	if (uid != NULL) {

		if (readUid (&puid) > 0) {

			switch (access.checkUid (uid)) {

			case UNAUTHORIZED:
				mode = INIT_LEARN;
				break;
			/*case AUTHORIZED:
			    break;*/
			case MASTER_ACCESS:
				mode = INIT_CLEAR;
				break;
			default :
				mode = INIT_NORMAL;
			}
		}
	} else {
		Serial.println ("memory error at mode_master()");
		mode = INIT_NORMAL;
	}
}

void mode_master_feedback() {

	if (millis () - feedbackMillis > 0 && millis() - feedbackMillis < 250) {
		digitalWrite (greenLed, HIGH);
		digitalWrite (redLed, HIGH);
	}

	if (millis () - feedbackMillis > 250 && millis() - feedbackMillis < 500) {
		digitalWrite (greenLed, LOW);
		digitalWrite (redLed, LOW);
	}

	if (millis() - feedbackMillis > 500) {
		feedbackMillis = millis();
	}
}

/*****************************************************************************
 * MODE: LEARNING
 *****************************************************************************/

void init_learn() {

	if (access.getAllowedTagsCount() < MAX_AUTHORIZED_TAGS) {
		strncpy (suid, uid, UID_LENGTH);
		Serial.print ("UID: "); Serial.println (suid);
		Serial.println ("Put this TAG again to add to the allowed TAGs list");
	} else {
		Serial.println ("Allowed TAGs limit reached.");
		mode = INIT_NORMAL; // Vuelve a modo normal
	}
}

void mode_learn() {

	if (puid != NULL && puid_aux != NULL) {

		if (readUid (&puid_aux) > 0) {

			if ( puid_aux != NULL ) {

				if (!strncmp (uid, uid_aux, UID_LENGTH)) {

					access.addUid (uid);

					strncpy (suid, uid, UID_LENGTH);
					Serial.println ("Added TAG!");
					Serial.print ("UID: "); Serial.println (suid);

					digitalWrite (redLed, HIGH);
					digitalWrite (greenLed, LOW);
					digitalWrite (buzzer, LOW);
					delay (500);
				} else {
					Serial.println ("Canceled!");

					digitalWrite (redLed, LOW);
					digitalWrite (greenLed, HIGH);
					digitalWrite (buzzer, LOW);
					delay (500);
				}
				mode = INIT_NORMAL;
			} else {
				Serial.println ("memory error at mode_learn()");
				mode = INIT_NORMAL;
			}
		}
	}  else {
		Serial.println ("memory error at mode_learn()");
		mode = INIT_NORMAL;
	}
}

void mode_learn_feedback() {

	if (millis () - feedbackMillis > 0 && millis() - feedbackMillis < 250) {
		digitalWrite (greenLed, LOW);
		digitalWrite (redLed, HIGH);
	}

	if (millis () - feedbackMillis > 250 && millis() - feedbackMillis < 500) {
		digitalWrite (greenLed, LOW);
		digitalWrite (redLed, LOW);
	}

	if (millis() - feedbackMillis > 500) {
		feedbackMillis = millis();
	}
}

/*****************************************************************************
 * MODE: CLEAR
 *****************************************************************************/

void init_clear() {
	Serial.println ("Put the MASTER TAG again to clear the allowed TAGs list.");
}

void mode_clear() {

	if (puid != NULL && puid_aux != NULL) {

		if (readUid (&puid_aux) > 0) {

			if (!strncmp (uid, uid_aux, UID_LENGTH)) {
				Serial.println ("Clearing allowed TAGs...");
				access.clearUids();
			} else {
				Serial.println ("Canceled!");
			}
			mode = INIT_NORMAL;
		}
	} else {
		Serial.println ("memory error at mode_clear()");
		mode = INIT_NORMAL;
	}
}

void mode_clear_feedback() {

	if (millis () - feedbackMillis > 0 && millis() - feedbackMillis < 500) {
		digitalWrite (greenLed, HIGH);
		digitalWrite (redLed, HIGH);
	}

	if (millis () - feedbackMillis > 500 && millis() - feedbackMillis < 1000) {
		digitalWrite (greenLed, HIGH);
		digitalWrite (redLed, LOW);
	}

	if (millis() - feedbackMillis > 1000) {
		feedbackMillis = millis();
	}
}

/*****************************************************************************
 *                  FEEDBACK LEDS AND BUZZER FUNCTIONS
 *****************************************************************************/

void success() {

	Serial.println ("");
	Serial.println ("--------- WELCOME! ---------");
	Serial.println ("");

	digitalWrite (greenLed, LOW);
	digitalWrite (buzzer, LOW);
	delay (750);
	digitalWrite (greenLed, HIGH);
	digitalWrite (buzzer, HIGH);
}

void fail() {

	Serial.println ("");
	Serial.println ("!!!!!! ACCESS DENIED! !!!!!!");
	Serial.println ("");

	digitalWrite (redLed, LOW);
	digitalWrite (buzzer, LOW);
	delay (100);
	digitalWrite (redLed, HIGH);
	digitalWrite (buzzer, HIGH);

	delay (75);

	digitalWrite (redLed, LOW);
	digitalWrite (buzzer, LOW);
	delay (100);
	digitalWrite (redLed, HIGH);
	digitalWrite (buzzer, HIGH);

	delay (75);

	digitalWrite (redLed, LOW);
	digitalWrite (buzzer, LOW);
	delay (100);
	digitalWrite (redLed, HIGH);
	digitalWrite (buzzer, HIGH);
}