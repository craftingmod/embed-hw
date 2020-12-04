#pragma once

#include "basic.hpp"

#define RFID_INTERVAL 50
#define SS_PIN 10
#define RST_PIN 9

#define ALLOWED_KEYS 3

// 부우저
#define BOOSER_DURATION 200
// 카드1 hz
#define CARD1_HZ 784
// 카드2 hz
#define CARD2_HZ 1760

void printHex(byte *buffer, byte bufferSize);

void printDec(byte *buffer, byte bufferSize);

bool equalByteArr(byte *a, byte *b, uint16_t ln);

uint UID4ToUInt(byte uidbyte[10]);

void initMFRC();

void taskMFRC();

uint getCardUUID();
void setCardUUID(uint uuid);