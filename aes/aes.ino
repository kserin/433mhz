#include "aes.h"

AES aes = AES();

void setup() {
  Serial.begin(9600);
}

void loop() {
  char key[] = {'2', '5', '6', '2', '5', '6', '2', '5', '6', '2', '5', '6', '2', '5', '6', '2', '5', '6', '2', '5', '6', '2', '5', '6', '2', '5', '6', '2', '5', '6', '2', '5'};
  char nonce[] = {'1', '2', '3', '4', '5', '6', '7', '8', 0, 0, 0, 0, 0, 0, 0, 1};
  
  byte text[48] = { 0 };
  char* value = "this is my super long message to be encrypted";
  memcpy(text, value, 45);

  Serial.println((char*) text);
  aes.ctr((void*) nonce, (void*) key, (void*) text, 45);
  Serial.println((char*) text);
  for (int i = 0; i < 46; i++) {
    print_hex(text[i]);
    Serial.print(" ");
  }
  Serial.println("");
  delay(10000);
}

void print_hex(byte value) {
  byte nib1 = (value >> 4) & 0x0F;
  byte nib2 = (value >> 0) & 0x0F;
  char v1 = nib1  < 0xA ? '0' + nib1  : 'A' + nib1  - 0xA;
  char v2 = nib2  < 0xA ? '0' + nib2  : 'A' + nib2  - 0xA;
  Serial.print(v1);
  Serial.print(v2);
}
