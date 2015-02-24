#ifndef NFC_ACCESS_CONTROL_H
#define NFC_ACCESS_CONTROL_H

#define MASTER_TAG "A4CA3ECF"       // Set to your UID MASTER TAG
#define UID_LENGTH          8       // 8 chars for a 4 bytes UID
#define MAX_AUTHORIZED_TAGS 16      // Maximum number of athorized TAGs
#define UNAUTHORIZED        0
#define AUTHORIZED          1
#define MASTER_ACCESS       2

#include <Arduino.h>
#include <EEPROM.h>

class NfcAccessControl {

public:
	NfcAccessControl();
	void addUid(char *uid);
	uint8_t checkUid(char *uid);
	void clearUids();
	uint8_t getAllowedTagsCount();
	char *getUid(uint8_t index);

private:
	uint8_t allowed_tags_count;
	char allowed_tags[MAX_AUTHORIZED_TAGS][UID_LENGTH];

	void loadUids();
};

#endif