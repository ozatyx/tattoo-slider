#include <Arduino.h>
#include "CapacitiveSensor.h"
#include "OneWire.h"

#define sendPin0 3
#define sendPin1 6

#define sensePin0 4
#define sensePin1 7

long output0;
long output1;

CapacitiveSensor cap0 = CapacitiveSensor(sendPin0, sensePin0);
CapacitiveSensor cap1 = CapacitiveSensor(sendPin1, sensePin1);


void setup() {

  Serial.begin(115200);
  delay(300);
  Serial.println("begin");
  
}

void loop() {

  output0 = cap0.capacitiveSensor(30);
  output1 = cap1.capacitiveSensor(30);

  Serial.println(output0);
  Serial.println(output1);

  vTaskDelay(200 / portTICK_PERIOD_MS);
}
