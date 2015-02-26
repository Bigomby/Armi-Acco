#include "NfcAccessControl.h"

/*****************************************************************************
 * Creating a NfcAccessControl object loads all the allowed UIDs stored in
 * the EEPROM.
 *****************************************************************************/
NfcAccessControl::NfcAccessControl(uint8_t auth_mode) {

	this->auth_mode = auth_mode;

	switch (auth_mode) {
	case REMOTE_WIFI_AUTH:
		Serial.println("MODO WIFI");
		// TODO
		break;
	case REMOTE_XBEE_AUTH:
		Serial.println("MODO XBEE");
		SoftSerial = new SoftwareSerial(8, 9);
		SoftSerial->begin(9600);
		xbee = XBee();
		xbee.setSerial(*SoftSerial);
		break;
	default:
		Serial.println("MODO LOCAL");
		loadUids();
	}
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

	uint8_t access = UNAUTHORIZED;

	Serial.print("Autorizando mediante: "); Serial.println(auth_mode);

	switch (auth_mode) {
	case REMOTE_WIFI_AUTH:
		access = remoteWifiAuth(uid);
		break;
	case REMOTE_XBEE_AUTH:
		access = remoteXbeeAuth(uid);
		break;
	default:
		access = localAuth(uid);
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

uint8_t NfcAccessControl::localAuth(char *uid) {

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
		Serial.println("memory error at localAuth()");
	}

	return access;
}

uint8_t NfcAccessControl::remoteXbeeAuth(char *uid) {

	uint8_t *payload = NULL;
	XBeeAddress64 addr64 = XBeeAddress64(0x0013A200, 0x40B96ED1);
	ZBTxStatusResponse txStatus = ZBTxStatusResponse();

	payload = (uint8_t *) calloc (UID_LENGTH, sizeof(uint8_t));
	memcpy(payload, uid, UID_LENGTH);
	ZBTxRequest zbTx = ZBTxRequest(addr64, payload, UID_LENGTH);

	Serial.println("Checking...");
	xbee.send(zbTx);

	if (xbee.readPacket(100)) {

		if (xbee.getResponse().getApiId() == ZB_TX_STATUS_RESPONSE) {
			xbee.getResponse().getZBTxStatusResponse(txStatus);

			if (txStatus.getDeliveryStatus() == SUCCESS) {
				Serial.println("success.  time to celebrate");

			} else {
				Serial.println("The remote XBee did not receive our packet. is it powered on?");

			}
		}
	} else if (xbee.getResponse().isError()) {
		Serial.print("Error reading packet.  Error code: ");
		Serial.println(xbee.getResponse().getErrorCode());
	} else {
		Serial.println("local XBee did not provide a timely TX Status Response -- should not happen");
	}

	free(payload);
	return 0;
}

uint8_t NfcAccessControl::remoteWifiAuth(char *uid) {

	return 0;
}