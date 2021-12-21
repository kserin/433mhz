#include "layer1.h"

Layer1 layer1  = Layer1();

void setup() {
  Serial.begin(9600);
  layer1.enableReceive();
}

int dataSize = 0;
byte data[RF433_LAYER1_MAX_DATA_SIZE + 1];

void loop() {
  if ((dataSize = layer1.receive(data)) > 0) {
    Serial.print("Received size: ");
    Serial.println(dataSize);

    data[dataSize] = 0;
    Serial.println((char*) data);
  }
  delay(1);
}
