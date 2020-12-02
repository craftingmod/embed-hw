#pragma once

#include "basic.hpp"
#include <SPI.h>
#include <MFRC522.h>

#define RFID_INTERVAL 50
#define SS_PIN 10
#define RST_PIN 9

#define ALLOWED_KEYS 3

void printHex(byte *buffer, byte bufferSize);

void printDec(byte *buffer, byte bufferSize);

bool equalByteArr(byte *a, byte *b, uint16_t ln);

uint UID4ToUInt(byte uidbyte[10]);

void initMFRC();

void taskMFRC(const uint32_t &time, const uint32_t &microTime);