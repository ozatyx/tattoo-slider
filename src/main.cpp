#include <Arduino.h>
#include "CapacitiveSensor.h"

#define highRes0 4 //High resistor value goes out of this GPIO
#define senseRes0 3 // Low resistor goes into this GPIO
#define senseRes1 6
#define meanCount 11 // puts this many readings into a list before averaging
#define ledPin D3     // should dim based on the position of the slider

CapacitiveSensor   cap4_3 = CapacitiveSensor(highRes0, senseRes0);  
CapacitiveSensor   cap4_6 = CapacitiveSensor(highRes0, senseRes1);

long reading0;
long reading1; 
long meanNum0;
long meanNum1;
double sliderValue;
long output;


void setup() {
  cap4_3.set_CS_AutocaL_Millis(0xFFFFFFFF);
  cap4_6.set_CS_AutocaL_Millis(0xFFFFFFFF);     // turn off autocalibrate on channel 1 - just as an example
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);
}

void loop() {
  meanNum0 = 0;
  meanNum1 = 0;

  for(int x = 0; x < meanCount; x++){         
    reading0 = cap4_3.capacitiveSensor(30);   
    reading1 = cap4_6.capacitiveSensor(30);

    meanNum0 =+ reading0;
    meanNum1 =+ reading1;
  }

  meanNum0 = 1+(meanNum0/meanCount);              // averages out meanCount amount of readings to reduce noise
  meanNum1 = 1+(meanNum1/meanCount);              // adds 1 to not divide by 0

  sliderValue = static_cast<double>(meanNum1)/static_cast<double>(meanNum0); //doubles to get decimals when dividing
  output = map(sliderValue, 0, 11, 0, 255);
  Serial.println(output);
  analogWrite(ledPin, output);

  //Serial.println("");
  //Serial.println("Sensor 1:");
  //Serial.println(meanNum0);

  //Serial.println("");
  //Serial.println("Sensor 2:");
  //Serial.println(meanNum1);

  //Serial.println("");
  //Serial.println(sliderValue);
  
  
  delay(100);  
}
