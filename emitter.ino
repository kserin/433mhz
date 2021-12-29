#include "lib433.h"

Lib433 lib433 = Lib433(0x01, 10, 5);
byte destAddr = 0x02;
unsigned short destSecret = 0xA000;
byte destKey[32] = {0x62, 0x50, 0x65, 0x53, 0x68, 0x56, 0x6B, 0x59, 0x70, 0x33, 0x73, 0x36, 0x76, 0x39, 0x79, 0x24, 0x42, 0x26, 0x45, 0x29, 0x48, 0x40, 0x4D, 0x63, 0x51, 0x66, 0x54, 0x6A, 0x57, 0x6E, 0x5A, 0x71};
char* string = "Hello World !";

void setup() {
  Serial.begin(9600);
}

void loop() {
  if (Serial.available() > 0) {
    Serial.read();
    Serial.println("Emit ...");
    lib433.send(destAddr, (byte*) string, 13, destSecret, destKey, &generateSequenceNumber);
  }
  delay(10);
}

unsigned long generateSequenceNumber(byte receiver) {
  return 0;
}
