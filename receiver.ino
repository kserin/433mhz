#include "lib433.h"

Lib433 lib433 = Lib433(0x02, -1, -1, true);
unsigned long nextSequenceNumber = 0;

void setup() {
  Serial.begin(9600);
}

int dataSize = 0;
byte data[RF433_LAYER2_MAX_DATA_SIZE + 1];

unsigned short secretId = 0xA000;
byte key[32] = {0x62, 0x50, 0x65, 0x53, 0x68, 0x56, 0x6B, 0x59, 0x70, 0x33, 0x73, 0x36, 0x76, 0x39, 0x79, 0x24, 0x42, 0x26, 0x45, 0x29, 0x48, 0x40, 0x4D, 0x63, 0x51, 0x66, 0x54, 0x6A, 0x57, 0x6E, 0x5A, 0x71};

void loop() {
  if ((dataSize = lib433.receive(data, secretId, key, &checkSequenceNumber)) > 0) {
    Serial.print("Received size: ");
    Serial.println(dataSize);

    data[dataSize] = 0;
    Serial.println((char*) data);
  }
  delay(1);
}

bool checkSequenceNumber(unsigned long sequenceNumber, byte sender) {
  if (sequenceNumber < nextSequenceNumber) {
    return false;
  }
  nextSequenceNumber = sequenceNumber + 1;
  return true;
}
