#ifndef LAYER2_h
#define LAYER2_h

#include "Arduino.h"
#include "layer1.h"
#include "aes.h"

#define RF433_LAYER2_MAX_DATA_SIZE 38
#define RF433_LAYER2_PROTOCOL 0

class Layer2 {
  using SequenceNumberChecker = bool (*)(unsigned long sequenceNumber, byte sender);
  using SequenceNumberGenerator = unsigned long (*)(byte receiver);
  
  public:
    Layer2(Layer1 layer1, byte myAddress);

    int receive(byte *data, unsigned short mySecretId, byte *myAesKey, SequenceNumberChecker sequenceNumberCheck);
    void send(byte receiver, byte *data, byte size, unsigned short destSecretId, byte *destAesKey, SequenceNumberGenerator sequenceNumber);

    struct Rf433Layer2_Header {
      byte protocol: 2;
      byte size: 6;
      byte sender;
      byte receiver;
      unsigned long sequenceNumber;
    };

    struct Rf433Layer2_CipheredData {
      unsigned short secretId;
      byte data[RF433_LAYER2_MAX_DATA_SIZE];
    };

    struct Rf433Layer2_Packet {
      Rf433Layer2_Header header;
      Rf433Layer2_CipheredData cipheredData;
    };

  private:
    Layer1 layer1;
    AES aes;
    byte myAddress;
};

#endif
