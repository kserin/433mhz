#ifndef LAYER1_h
#define LAYER1_h

#include "Arduino.h"
#include "RCSwitch.h"

// 15 fragments of 3 bytes + 1st fragment of 2 bytes
#define RF433_LAYER1_MAX_DATA_SIZE 47

class Layer1 {
  public:
    Layer1();
    
    void enableReceive();
    int receive(byte *data);
    
    void enableEmit(int emitPin, int nbRepeatTransmit);
    void emit(byte *data, long size);

    struct Rf433Layer1 {
      byte fragmentNumber : 4;
      byte crc : 4;
      long data : 24;
    };

  private:
    RCSwitch rf;
    byte crc4(long value24bits);
    bool receiveData(Rf433Layer1* packet);
    void sendData(Rf433Layer1* packet);
};

#endif
