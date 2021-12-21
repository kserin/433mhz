#include "Arduino.h"
#include "RCSwitch.h"
#include "layer1.h"

Layer1::Layer1() {
  this->rf = RCSwitch();
}

void Layer1::enableReceive() {
  this->rf.enableReceive(0);
}

void Layer1::enableEmit(int emitPin, int nbRepeatTransmit) {
  this->rf.enableTransmit(emitPin);
  this->rf.setRepeatTransmit(nbRepeatTransmit);
}

int Layer1::receive(byte *data) {
  static byte readed = 0;
  static byte totalSize = 0;

  Rf433Layer1 packet;

  if (!this->receiveData(&packet)) {
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

void Layer1::emit(byte *data, long size) {
  if (size > RF433_LAYER1_MAX_DATA_SIZE) {
    size = RF433_LAYER1_MAX_DATA_SIZE;
  }

  long readed = 0;
  byte bytesReaded = 0;
  byte fragmentNumber = 0;
  byte maxBytesPerFragment = 2;
  Rf433Layer1 packet;
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
      packet.crc = this->crc4(packet.data);
      this->sendData(&packet);

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
    packet.crc = this->crc4(packet.data);
    this->sendData(&packet);
  }
}

byte Layer1::crc4(long value24bits) {
  value24bits = value24bits & 0x00FFFFFF; // remove most significant byte
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

bool Layer1::receiveData(Rf433Layer1* packet) {
  static byte nextFragmentNumber = 0;
  static byte lastFragmentNumber = 0;
  static unsigned long lastReceivedPacketMillis = 0;

  if (this->rf.available()) {
    unsigned long value = this->rf.getReceivedValue();
    packet->fragmentNumber = value >> 28;
    packet->crc = (value & 0xFF000000) >> 24;
    packet->data = value & 0xFFFFFF;
    
    this->rf.resetAvailable();

    long elapsedTime = millis() - lastReceivedPacketMillis;
    if (elapsedTime < 0 || elapsedTime >= 1000) {
      nextFragmentNumber = 0;
    }

    if (packet->fragmentNumber != nextFragmentNumber) {
      return false;
    }

    if (this->crc4(packet->data) != packet->crc) {
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

void Layer1::sendData(Rf433Layer1* packet) {
  long value = packet->fragmentNumber;
  value = (value << 4 ) | packet->crc;
  value = (value << 24) | packet->data;

  this->rf.send(value, sizeof(long) * 8);
}

