#ifndef NFC_ACCESS_CONTROL_H
#define NFC_ACCESS_CONTROL_H

#define MASTER_TAG "A4CA3ECF"       // Set to your UID MASTER TAG
#define UID_LENGTH          8       // 8 chars for a 4 bytes UID
#define MAX_AUTHORIZED_TAGS 8      // Maximum number of athorized TAGs
#define UNAUTHORIZED        0
#define AUTHORIZED          1
#define MASTER_ACCESS       2

#define LOCAL_AUTH          0
#define REMOTE_WIFI_AUTH    1
#define REMOTE_XBEE_AUTH    2

#include <Arduino.h>
#include <SoftwareSerial.h>
#include <EEPROM.h>
#include <XBee.h>
#include <coap.h>

class NfcAccessControl {

public:
	void init(uint8_t auth_mode = LOCAL_AUTH);
	void addUid(char *uid);
	uint8_t checkUid(char *uid);
	void clearUids();
	uint8_t getAllowedTagsCount();
	char *getUid(uint8_t index);

private:
	uint8_t auth_mode;
	uint8_t allowed_tags_count;
	char allowed_tags[MAX_AUTHORIZED_TAGS][UID_LENGTH];
	SoftwareSerial *SoftSerial;
	XBee xbee;

	uint8_t remoteAuth(char *uid);
	uint8_t localAuth(char *uid);
	uint8_t remoteXbeeAuth(char *uid);
	uint8_t remoteWifiAuth(char *uid);
	void loadUids();
	uint8_t memErr();
};

#endif