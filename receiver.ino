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
  rf.enableReceive(0);
}

int dataSize = 0;
byte data[MAX_DATA_SIZE + 1];

void loop() {
  if ((dataSize = receiveData(rf, data)) > 0) {
    Serial.print("Received size: ");
    Serial.println(dataSize);

    data[dataSize] = 0;
    Serial.println((char*) data);
  }
  delay(1);
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

int receiveData(RCSwitch rf, byte *data) {
  static byte readed = 0;
  static byte totalSize = 0;

  RfPacket packet;

  if (!receivePacket(rf, &packet)) {
    return -1;
  }

  if (packet.fragmentNumber == 0) {
    readed = 0;
    totalSize = (byte) (packet.data >> 16);  // First byte of data is the total size
  }

  byte bytesToRead = packet.fragmentNumber == 0 ? 2 : 3;
  for (int i = bytesToRead - 1; i >= 0 && readed < totalSize; i--) {
    data[readed] = (byte) (packet.data >> (i * 8));
    readed++;
  }

  if (readed == totalSize) {
    byte result = totalSize;
    readed = 0;
    totalSize = 0;
    return result;
  } else {
    return 0;
  }

  return readed == totalSize ? totalSize : 0;
}

bool receivePacket(RCSwitch rf, RfPacket* packet) {
  static byte nextFragmentNumber = 0;
  static byte lastFragmentNumber = 0;
  static unsigned long lastReceivedPacketMillis = 0;

  if (rf.available()) {
    unsigned long value = rf.getReceivedValue();
    packet->fragmentNumber = value >> 28;
    packet->crc = (value & 0xFF000000) >> 24;
    packet->data = value & 0xFFFFFF;
    
    rf.resetAvailable();

    long elapsedTime = millis() - lastReceivedPacketMillis;
    if (elapsedTime < 0 || elapsedTime >= 1000) {
      nextFragmentNumber = 0;
    }

    if (packet->fragmentNumber != nextFragmentNumber) {
      return false;
    }

    if (crc4(packet->data) != packet->crc) {
      return false;
    }

    if (packet->fragmentNumber == 0) {
      byte totalSize = packet->data >> 16;
      if (totalSize - 2 <= 0) {
        lastFragmentNumber = 0;
      } else {
        lastFragmentNumber = (totalSize - 2) / 3;
        if ((totalSize - 2) % 3 > 0) {
          lastFragmentNumber++;
        }
      }
    }
  
    lastReceivedPacketMillis = millis();
    if (packet->fragmentNumber == lastFragmentNumber) {
      nextFragmentNumber = 0;
    } else {
      nextFragmentNumber = packet->fragmentNumber + 1;
    }
    return true;
  }

  return false;
}

