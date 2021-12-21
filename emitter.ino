#include "layer1.h"

Layer1 layer1  = Layer1();

void setup() {
  Serial.begin(9600);
  layer1.enableEmit(10, 5);
}

void loop() {
  char* string = "Hello World !";
  layer1.emit((byte*) string, 13);
  delay(5000);

  string = "1";
  layer1.emit((byte*) string, 1);
  delay(5000);


  string = "12";
  layer1.emit((byte*) string, 2);
  delay(5000);

  string = "12345";
  layer1.emit((byte*) string, 5);
  delay(5000);


  string = "123456";
  layer1.emit((byte*) string, 6);
  delay(30000);
}

