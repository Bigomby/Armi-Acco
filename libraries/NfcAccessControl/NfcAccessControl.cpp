#include "NfcAccessControl.h"

/*****************************************************************************
 * Creating a NfcAccessControl object loads all the allowed UIDs stored in
 * the EEPROM.
 *****************************************************************************/
void NfcAccessControl::init(uint8_t auth_mode) {

	this->auth_mode = auth_mode;

	switch (auth_mode) {
	case REMOTE_WIFI_AUTH: {
		// TODO
		break;
	}
	case REMOTE_XBEE_AUTH: {
		SoftSerial = new SoftwareSerial(8, 9);
		SoftSerial->begin(9600);
		xbee = XBee();
		xbee.setSerial(*SoftSerial);
		//coap_setup();
		//endpoint_setup();
		break;
	}
	default: {
		loadUids();
	}
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
		memErr();
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
		memErr();
	}

	return access;
}

uint8_t NfcAccessControl::remoteXbeeAuth(char *uid) {

	// XBEE
	XBeeAddress64 addr64 = XBeeAddress64(0x0013A200, 0x40B96ED1);
	ZBTxStatusResponse txStatus = ZBTxStatusResponse();

	// CoaP
	uint8_t packetbuf[24];
	size_t rsplen = sizeof(packetbuf);
	int rc;
	coap_packet_t req;

	if (packetbuf != NULL) {

		req.hdr.ver = 0x01;
		req.hdr.t = COAP_TYPE_NONCON;
		req.hdr.tkl = 0x00;
		req.hdr.code = COAP_METHOD_POST;
		req.hdr.id[0] = 0x00;
		req.hdr.id[1] = 0x00;

		req.payload.len = UID_LENGTH;
		const uint8_t *payload = (uint8_t*) uid;
		req.payload.p = payload;
		req.tok.len = 0x00;
		req.numopts = 0x00;

		if (0 != (rc = coap_build(packetbuf, &rsplen, &req))) {
			Serial.print("rc=");
			Serial.println(rc, DEC);
		} else {
			ZBTxRequest zbTx = ZBTxRequest(addr64, packetbuf, rsplen);
			Serial.println("Sending...");
			xbee.send(zbTx);

			if (xbee.readPacket(100)) {

				if (xbee.getResponse().getApiId() == ZB_TX_STATUS_RESPONSE) {
					xbee.getResponse().getZBTxStatusResponse(txStatus);

					if (txStatus.getDeliveryStatus() == SUCCESS) {
						Serial.println("Success");

					} else {
						Serial.println("Error");

					}
				}
			} else if (xbee.getResponse().isError()) {
				Serial.print("Error code: ");
				Serial.println(xbee.getResponse().getErrorCode());
			} else {
				Serial.println("Local error");
			}
		}
	} else {
		memErr();
	}

	return 0;
}

uint8_t NfcAccessControl::remoteWifiAuth(char *uid) {

	return 0;
}

uint8_t NfcAccessControl::memErr() {
	Serial.println("Memmory error");
}