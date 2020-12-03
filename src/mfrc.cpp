#include "basic.hpp"
#include "mfrc.hpp"
#include <SPI.h>
#include <MFRC522.h>

// 허용되는 키들
const MFRC522::PICC_Type allowedKey[] = {MFRC522::PICC_TYPE_MIFARE_MINI, MFRC522::PICC_TYPE_MIFARE_1K, MFRC522::PICC_TYPE_MIFARE_4K};

MFRC522 rfid(SS_PIN, RST_PIN);
MFRC522::MIFARE_Key key;

uint cardUUID = 0;

namespace Timestamp {
  uint mfrcCheck = 0;
}

//16진수로 변환하는 함수
void printHex(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], HEX);
  }
}

//10진수로 변환하는 함수
void printDec(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], DEC);
  }
}

bool equalByteArr(byte *a, byte *b, uint16_t ln) {
  for (uint16_t i = 0; i < ln; i += 1) {
    if (a[i] != b[i]) {
      return true;
    }
  }
  return false;
}
// UID를 4byte 로 변환
uint UID4ToUInt(byte uidbyte[10]) {
  // 으메이징 흑마법
  uint *i = (uint *)uidbyte;
  return *i;
}

void initMFRC() {
  Serial.println("This code scan the MIFARE Classsic NUID.");
  SPI.begin();
  rfid.PCD_Init();
}

void taskMFRC() {
  const uint time = millis();
  if (time - Timestamp::mfrcCheck < RFID_INTERVAL) {
    return;
  }
  Timestamp::mfrcCheck = time;
  // RFID 리더에 카드가 없을 때 취소
  if (!rfid.PICC_IsNewCardPresent()) {
    // Serial.println("No new card");
    return;
  }
  // RFID 리더에 NUID 안읽히면 취소
  if (!rfid.PICC_ReadCardSerial()) {
    // Serial.println("Read incorrectly");
    return;
  }
  // 키 검사
  MFRC522::PICC_Type cardType = rfid.PICC_GetType(rfid.uid.sak);
  bool hasKey = false;
  for (byte i = 0; i < ALLOWED_KEYS; i += 1) {
    if (allowedKey[i] == cardType) {
      hasKey = true;
      break;
    }
  }
  if (!hasKey) {
    Serial.println("Wrong card type");
    return;
  }
  // 키 읽기
  uint keyUUID = UID4ToUInt(&rfid.uid.uidByte[0]);
  cardUUID = keyUUID;
  Serial.print("Card UUID: ");
  Serial.print(keyUUID, HEX);
  Serial.print("(");
  Serial.print(keyUUID, DEC);
  Serial.println(")");

  // 마무리
  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
}

uint getCardUUID() {
  return cardUUID;
}

void setCardUUID(uint uuid) {
  cardUUID = uuid;
}