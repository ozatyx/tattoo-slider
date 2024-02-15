#include <Arduino.h>
#include "CapacitiveSensor.h"

#define highRes0 4    //High resistor value goes out of this GPIO
#define highRes1 7
#define senseRes0 3   // Low resistor goes into this GPIO
#define senseRes1 6
#define meanCount 11  // this many readings before averaging
#define ledPin0 21 
#define ledPin1 5    // should dim based on the position of the slider

CapacitiveSensor   cap0 = CapacitiveSensor(highRes0, senseRes0);  
CapacitiveSensor   cap1 = CapacitiveSensor(highRes1, senseRes1);

long reading0;
long reading1; 
long meanNum0;
long meanNum1;
double sliderValue;
long output0;
long output1;
unsigned long timeOut = 100000;



void setup() {
  cap0.set_CS_AutocaL_Millis(0xFFFFFFFF);
  cap1.set_CS_AutocaL_Millis(0xFFFFFFFF);     // turn off autocalibrate on channel 1 - just as an example
  cap0.set_CS_Timeout_Millis(timeOut);
  cap1.set_CS_Timeout_Millis(timeOut);
  Serial.begin(115200);
  pinMode(ledPin0, OUTPUT);
  pinMode(ledPin1, OUTPUT);

  digitalWrite(ledPin0, HIGH);
  delay(500);
  digitalWrite(ledPin0, LOW);
  digitalWrite(ledPin1, HIGH);
  delay(500);
  digitalWrite(ledPin1, LOW);
  delay(500);
  digitalWrite(ledPin0, HIGH);
  digitalWrite(ledPin1, HIGH);
  delay(300);
  digitalWrite(ledPin0, LOW);
  digitalWrite(ledPin1, LOW);
  delay(300);
  digitalWrite(ledPin0, HIGH);
  digitalWrite(ledPin1, HIGH);
  delay(300);
  digitalWrite(ledPin0, LOW);
  digitalWrite(ledPin1, LOW);
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


  meanNum0 = 1+(meanNum0/meanCount);              // averages out meanCount amount of readings to reduce noise
  meanNum1 = 1+(meanNum1/meanCount);              // adds 1 to not divide by 0
  

  if (meanNum0 < 50){
    meanNum0 = 1;

  } else if (meanNum0 > 350){
      meanNum0 = 350;
  }

  if (meanNum1 < 50){
    meanNum1 = 1;

  } else if (meanNum1 > 350){
      meanNum1 = 350;
  }

  sliderValue = meanNum1 - meanNum0 + 350;
  
  if (sliderValue < 70){
    sliderValue = 1;
  }
  //output0 = map(meanNum1, 1, 350, 0, 255);


  output0 = map(sliderValue, 1, 699, 0, 255);

  output1 = map(meanNum0, 1, 350, 0, 255);
  //Serial.println(output);
  analogWrite(ledPin0, output0);
  //analogWrite(ledPin1, output1);

  

  //Serial.println("");
  //Serial.println("Sensor 1:");
  //Serial.println(meanNum0);

  //Serial.println("");
  //Serial.println("Sensor 2:");
  //Serial.println(meanNum1);

  Serial.println("proportion:");
  Serial.println(sliderValue);
  
  //Serial.print(">sensor1:");
  //Serial.println(meanNum0);

  //Serial.print(">sensor2:");
  //Serial.println(meanNum1);


  delay(100);  
}
