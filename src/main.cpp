#include <Arduino.h>
#include "CapacitiveSensor.h"
#include "BLEDevice.h"
#include "BLEHIDDevice.h"
#include "HIDTypes.h"
#include "HIDKeyboardTypes.h"

#define highRes0 4 //High resistor value goes out of this GPIO
#define highRes1 5

#define senseRes0 3 // Low resistor goes into this GPIO
#define senseRes1 6
#define meanCount 11 // puts this many readings into a list before averaging

//random things to make the esp32 control
#define ledPin D3     // should dim based on the position of the slider
#define US_KEYBOARD 1 // trying to control volume here

//hid definitions
#define KEY_VOLUP 0x80
#define KEY_VOLDOWN 0x81







CapacitiveSensor   cap0 = CapacitiveSensor(highRes0, senseRes0);  
CapacitiveSensor   cap1 = CapacitiveSensor(highRes0, senseRes1);

long reading0;
long reading1; 
long meanNum0;
long meanNum1;
double sliderValue;
long output;



















void setup() {
  cap0.set_CS_AutocaL_Millis(0xFFFFFFFF);
  cap1.set_CS_AutocaL_Millis(0xFFFFFFFF);     // turn off autocalibrate on channel 1 - just as an example
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);
}

void loop() {
  meanNum0 = 0;
  meanNum1 = 0;

  for(int x = 0; x < meanCount; x++){         
    reading0 = cap0.capacitiveSensor(30);   
    reading1 = cap1.capacitiveSensor(30);

    meanNum0 =+ reading0;
    meanNum1 =+ reading1;
  }

  meanNum0 = 1+(meanNum0/meanCount); 

  output = map(meanNum0, 0, 310, 0, 255);
  Serial.println(output);
  analogWrite(ledPin, output);

  
  
  
  
  delay(100);  
  
  //DEBUGGING\\\


  //Serial.println("");
  //Serial.println("Sensor 1:");
  //Serial.println(meanNum0);

  //Serial.println("");
  //Serial.println("Sensor 2:");
  //Serial.println(meanNum1);

  //Serial.println("");
  //Serial.println(sliderValue);
}
