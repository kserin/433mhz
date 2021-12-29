#include "Arduino.h"
#include "layer1.h"
#include "layer2.h"
#include "lib433.h"

Lib433::Lib433(byte myAddress, int emitPin, int repeat, bool enableReceive) {
  Layer1 layer1  = Layer1();
  if (emitPin >= 0 && repeat >= 1) {
    layer1.enableEmit(10, 5);
  }
  if (enableReceive) {
    layer1.enableReceive();
  }

  this->layer2 = new Layer2(layer1, myAddress);
}

Lib433::~Lib433() {
  delete this->layer2;
}

int Lib433::receive(byte *data, unsigned short mySecretId, byte *myAesKey, SequenceNumberChecker sequenceNumberCheck) {
  return this->layer2->receive(data, mySecretId, myAesKey, sequenceNumberCheck);
}

void Lib433::send(byte receiver, byte *data, byte size, unsigned short destSecretId, byte *destAesKey, SequenceNumberGenerator sequenceNumber) {
  this->layer2->send(receiver, data, size, destSecretId, destAesKey, sequenceNumber);
}
