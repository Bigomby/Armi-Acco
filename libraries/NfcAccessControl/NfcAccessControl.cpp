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
		Serial1.begin(9600);
		xbee = XBee();
		xbee.setSerial(Serial1);
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
	char resource[] = "auth";
	char aux_query[] = "uid=";
	char * query = (char*) calloc(sizeof(aux_query) + sizeof(uid) + 1 , sizeof(char*));

	strncat(query, aux_query, 4);
	strncat(query, uid, UID_LENGTH);
	size_t query_len = 4 + UID_LENGTH;

	// CoaP
	uint8_t packetbuf[24];
	size_t rsplen = sizeof(packetbuf);
	int rc;
	coap_packet_t req;
	uint8_t ret = UNAUTHORIZED;

	if (packetbuf != NULL) {

		// Header
		req.hdr.ver = 0x01;
		req.hdr.t = COAP_TYPE_NONCON;
		req.hdr.tkl = 0x00;
		req.hdr.code = COAP_METHOD_GET;
		req.hdr.id[0] = 0x00;
		req.hdr.id[1] = 0x00;

		// Token
		req.tok.len = 0x00;

		// Options
		req.numopts = 2;

		req.opts[0].num = 11; // URI-PATH
		req.opts[0].buf.p = (uint8_t*)resource;
		req.opts[0].buf.len = sizeof(resource);

		req.opts[1].num = 15; // URI-QUERY
		req.opts[1].buf.p = (uint8_t*)query;
		req.opts[1].buf.len = query_len;

		// Payload
		req.payload.len = 0;

		if (0 != (rc = coap_build(packetbuf, &rsplen, &req))) {
			Serial.print("rc=");
			Serial.println(rc, DEC);
		} else {
			ZBTxRequest zbTx = ZBTxRequest(addr64, packetbuf, rsplen);
			Serial.print("rc=");
			Serial.println(rc, DEC);
			//coap_dump(packetbuf, rsplen, true);

			xbee.send(zbTx);

			if (xbee.readPacket(100)) {

				if (xbee.getResponse().getApiId() == ZB_TX_STATUS_RESPONSE) {
					xbee.getResponse().getZBTxStatusResponse(txStatus);

					if (txStatus.getDeliveryStatus() == SUCCESS) {
						Serial.println("Sent");
						int code = recvpacket();
						if (code == 0x43) {
							ret = AUTHORIZED;
						}
					}

				} else {
					Serial.println("Error");
				}

			} else if (xbee.getResponse().isError()) {
				Serial.println(xbee.getResponse().getErrorCode());
			} else {
				Serial.println("Local error");
			}
		}
	} else {
		memErr();
	}

	return ret;
}

uint8_t NfcAccessControl::remoteWifiAuth(char *uid) {

	return 0;
}

uint8_t NfcAccessControl::memErr() {
	Serial.println("Memmory error");
}

uint8_t NfcAccessControl::recvpacket() {

	ZBRxResponse rx = ZBRxResponse();
	ModemStatusResponse msr = ModemStatusResponse();
	coap_packet_t resp;
	uint8_t buff[24];

	xbee.readPacket(1000);

	uint8_t ret = 0;

	if (xbee.getResponse().isAvailable()) {
		// got something

		if (xbee.getResponse().getApiId() == ZB_RX_RESPONSE) {
			// got a zb rx packet

			// now fill our zb rx class
			xbee.getResponse().getZBRxResponse(rx);

			if (rx.getOption() == ZB_PACKET_ACKNOWLEDGED) {
				// the sender got an ACK
				Serial.println("the sender got an ACK");
			} else {
				// we got it (obviously) but sender didn't get an ACK
				Serial.println("the sender didn't get an ACK");
			}

			Serial.print("checksum is ");
			Serial.println(rx.getChecksum(), HEX);

			Serial.print("packet length is ");
			Serial.println(rx.getPacketLength(), DEC);

			for (int i = 0; i < rx.getDataLength(); i++) {
				Serial.print("payload [");
				Serial.print(i, DEC);
				Serial.print("] is ");
				Serial.println(rx.getData()[i], HEX);
				buff[i] = rx.getData(i);
			}
			int rc;
			uint8_t len = rx.getDataLength();
			if (0 != (rc = coap_parse(&resp, buff, len)))
			{
				Serial.print("Bad packet rc=");
				Serial.println(rc, DEC);
			}
			else
			{
				coap_dumpPacket(&resp);
				uint16_t clas = resp.hdr.code;
				uint16_t det = resp.hdr.code;
				clas = (clas & 0xF0) >> 5;
				det = det & 0x0F;
				Serial.print(clas); Serial.print("."); Serial.println(det);
				ret = resp.hdr.code;
			}
		} else if (xbee.getResponse().getApiId() == MODEM_STATUS_RESPONSE) {
			xbee.getResponse().getModemStatusResponse(msr);
			// the local XBee sends this response on certain events, like association/dissociation

			if (msr.getStatus() == ASSOCIATED) {
				// yay this is great.  flash led
				Serial.println("Associated");
			} else if (msr.getStatus() == DISASSOCIATED) {
				// this is awful.. flash led to show our discontent
				Serial.println("Dissasociated");
			} else {
				// another status
				Serial.println("Status");
			}
		} else {
			// not something we were expecting
			Serial.println("not something we were expecting");
		}
	} else if (xbee.getResponse().isError()) {
		Serial.print("Error reading packet.  Error code: ");
		Serial.println(xbee.getResponse().getErrorCode());
	}

	return ret;
}