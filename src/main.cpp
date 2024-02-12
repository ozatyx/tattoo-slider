//Code to dim an LED using the slider as a dimming switch
//Pins based off the Seeed Studio Xiao ESP32C3


#include <Arduino.h>
#include "CapacitiveSensor.h"

#define highRes0 D2    //High resistor value goes out of these GPIO.
#define highRes1 D5

#define senseRes0 D1   // Low resistor goes into these GPIO.
#define senseRes1 D4

#define ledPin0 D3     // Should dim based on the position of the slider.
#define ledPin1 D6     

#define meanCount 11   // Stores this many readings before averaging.





CapacitiveSensor   cap0 = CapacitiveSensor(highRes0, senseRes0); // The signals coming from the left sensor.
CapacitiveSensor   cap1 = CapacitiveSensor(highRes0, senseRes1); // The right sensor.

long reading0;
long reading1; 
long meanNum0;
long meanNum1;
double sliderValue;
long result0;
long result1;

bool isBleConnected = false;

void setup() {
  cap0.set_CS_AutocaL_Millis(0xFFFFFFFF);
  cap1.set_CS_AutocaL_Millis(0xFFFFFFFF);     //Turns off autocallibration, may need to turn back on in some cases.
  
  Serial.begin(115200);

  pinMode(ledPin0, OUTPUT);
  pinMode(ledPin1, OUTPUT);
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

  result0 = map(meanNum0, 0, 310, 0, 255);
  Serial.println(result0);
  analogWrite(ledPin0, result0);

  result1 = map(meanNum1, 0, 310, 0, 255);
  Serial.println(result1);
  analogWrite(ledPin1, result1);
  
  
  
  
  
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

