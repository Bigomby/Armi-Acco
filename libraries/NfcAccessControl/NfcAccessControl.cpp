#include "NfcAccessControl.h"

/*****************************************************************************
 * Creating a NfcAccessControl object loads all the allowed UIDs stored in
 * the EEPROM.
 *****************************************************************************/
NfcAccessControl::NfcAccessControl() {

	loadUids();
}

/*****************************************************************************
 * Add a new TAG's UID to EEPROM and reload the authorized list of UIDs.
 *****************************************************************************/
void NfcAccessControl::addUid(char *uid) {

	uint8_t pw;

	if (uid != NULL) {
		pw = (allowed_tags_count * UID_LENGTH ) + 1;
		EEPROM.write(0, allowed_tags_count + 1);

		for (uint8_t i = 0 ; i < UID_LENGTH ; i++) {
			EEPROM.write(pw++, uid[i]);
		}

		loadUids();

	} else {
		Serial.println("memory error at addUid()");
	}
}

/*****************************************************************************
 * Check if TAG's UID is authorized. There are three posibilities:
 *
 * 0: Not authorized
 * 1: Authorized
 * 2: Master UID
 *****************************************************************************/
uint8_t NfcAccessControl::checkUid(char *uid) {

	uint8_t access = 0;

	if (uid != NULL) {

		if (!strncmp(uid, MASTER_TAG, UID_LENGTH)) {
			access = 2;
		} else {

			for (uint8_t i = 0 ; i < allowed_tags_count ; i++) {

				if (!strncmp(uid, allowed_tags[i], UID_LENGTH)) {
					access = 1;
				}
			}
		}
	} else {
		Serial.println("memory error at checkUid()");
	}

	return access;
}

/*****************************************************************************
 * Clear the EEPROM removing all allowed UIDs.
 *****************************************************************************/
void NfcAccessControl::clearUids() {

	for (uint16_t i = 0 ; i < 512 ; i++) {
		EEPROM.write(i, 0);
	}

	loadUids();
}

/*****************************************************************************
 * Get the count of allowed TAGs.
 *****************************************************************************/
uint8_t NfcAccessControl::getAllowedTagsCount() {

	return allowed_tags_count;
}

/*****************************************************************************
 * Get UIDs for a given index.
 *****************************************************************************/
char *NfcAccessControl::getUid(uint8_t index) {

	return allowed_tags[index];
}

/*****************************************************************************
 * The allowed UIDs are read from the EEPROM.
 *****************************************************************************/
void NfcAccessControl::loadUids() {

	uint8_t pr = 0;

	allowed_tags_count = EEPROM.read(pr++);

	for (uint8_t i = 0 ; i < allowed_tags_count ; i++) {
		for (uint8_t j = 0 ; j < UID_LENGTH ; j++) {
			allowed_tags[i][j] = EEPROM.read(pr++);
		}
	}
}