#include <RCSwitch.h>

// 15 fragments of 3 bytes + 1st fragment of 2 bytes
#define MAX_DATA_SIZE 47

RCSwitch rf = RCSwitch();

typedef struct {
  byte fragmentNumber : 4;
  byte crc : 4;
  long data : 24;
} RfPacket;

void setup() {
  Serial.begin(9600);
  rf.enableTransmit(10);
  rf.setRepeatTransmit(5);
}

void loop() {
  char* string = "Hello World !";
  sendData(rf, (byte*) string, 13);
  delay(5000);

  string = "1";
  sendData(rf, (byte*) string, 1);
  delay(5000);


  string = "12";
  sendData(rf, (byte*) string, 2);
  delay(5000);

  string = "12345";
  sendData(rf, (byte*) string, 5);
  delay(5000);


  string = "123456";
  sendData(rf, (byte*) string, 6);
  delay(30000);
}

byte crc4(long value24bits) {
  value24bits = value24bits & 0x00FFFFFF; // remove most significant bytes
  long polynomial = 0b10011;
  
  long result = value24bits << 4;
  polynomial = polynomial << 23;

  for (int i = 0; i < 24; i++) {
    long msb = result & ((long) 1 << (27 - i));
    if (msb > 0) {
      result = result ^ polynomial;
    }
    polynomial = polynomial >> 1;
  }

  return (byte) result;
}

void sendData(RCSwitch rf, byte *data, long size) {
  if (size > MAX_DATA_SIZE) {
    size = MAX_DATA_SIZE;
  }

  long readed = 0;
  byte bytesReaded = 0;
  byte fragmentNumber = 0;
  byte maxBytesPerFragment = 2;
  RfPacket packet;
  for (int i = 0; i < size; i++) {
    byte b = *(data + i);
    readed = (readed << 8) | b;
    bytesReaded++;

    if (bytesReaded == maxBytesPerFragment) {
      /* First fragment only contains 2 bytes because 
       * the first byte of data contains the total length */
      if (fragmentNumber == 0) {
        packet.data = (size << 16) | readed;
        maxBytesPerFragment = 3;
      } else {
        packet.data = readed;
      }

      packet.fragmentNumber = fragmentNumber;
      packet.crc = crc4(packet.data);
      sendPacket(rf, &packet);

      fragmentNumber++;
      bytesReaded = 0;
      readed = 0;
    }
  }
  if (bytesReaded > 0) {
    for (int i = bytesReaded; i < maxBytesPerFragment; i++) {
      readed = readed << 8;
    }
    if (fragmentNumber == 0) {
      packet.data = (size << 16) | readed;
    } else {
      packet.data = readed;
    }
    packet.fragmentNumber = fragmentNumber;
    packet.crc = crc4(packet.data);
    sendPacket(rf, &packet);
  }
}

void sendPacket(RCSwitch rf, RfPacket* packet) {
  long value = packet->fragmentNumber;
  value = (value << 4 ) | packet->crc;
  value = (value << 24) | packet->data;

  rf.send(value, sizeof(long) * 8);
}

