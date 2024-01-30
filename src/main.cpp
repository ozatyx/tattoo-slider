#include <Arduino.h>
#include "CapacitiveSensor.h"

#define highRes0 4 //High resistor value goes out of this GPIO
#define senseRes0 3 // Low resistor goes into this GPIO
#define senseRes1 15
#define meanCount 11 // puts this many readings into a list before averaging

CapacitiveSensor   cap4_3 = CapacitiveSensor(highRes0, senseRes0);  
CapacitiveSensor   cap4_5 = CapacitiveSensor(highRes0, senseRes1);

long reading0;
long reading1; 
long meanNum0;
long meanNum1;
float sliderValue;

void setup() {
  cap4_3.set_CS_AutocaL_Millis(0xFFFFFFFF);
  cap4_5.set_CS_AutocaL_Millis(0xFFFFFFFF);     // turn off autocalibrate on channel 1 - just as an example
  Serial.begin(9600);
}

void loop() {
  meanNum0 = 0;
  meanNum1 = 0;

  for(int x = 0; x < meanCount; x++){
    reading0 = cap4_3.capacitiveSensor(30);
    reading1 = cap4_5.capacitiveSensor(30);

    meanNum0 =+ reading0;
    meanNum1 =+ reading1;
  }

  meanNum0 = meanNum0/meanCount;
  meanNum1 = meanNum1/meanCount;

  sliderValue = meanNum0/meanNum1;

  Serial.println(sliderValue);
  
  
  delay(50);  
}
