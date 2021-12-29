#ifndef LIB433_h
#define LIB433_h

#include "Arduino.h"
#include "layer1.h"
#include "layer2.h"

class Lib433 {
  using SequenceNumberChecker = bool (*)(unsigned long sequenceNumber, byte sender);
  using SequenceNumberGenerator = unsigned long (*)(byte receiver);

  public:
    Lib433(byte myAddress, int emitPin = -1, int repeat = -1, bool enableReceive = false);
    ~Lib433();
    int receive(byte *data, unsigned short mySecretId, byte *myAesKey, SequenceNumberChecker sequenceNumberCheck);
    void send(byte receiver, byte *data, byte size, unsigned short destSecretId, byte *destAesKey, SequenceNumberGenerator sequenceNumber);

  private:
    Layer2* layer2;
};

#endif
