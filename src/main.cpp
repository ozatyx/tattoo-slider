#include <Arduino.h>
#include "CapacitiveSensor.h"
#include "NimBLEDevice.h"

#define sendPin0 D1
#define sendPin1 D4

#define sensePin0 D2
#define sensePin1 D5

#define ledPin0 D3
#define ledPin1 D6

#define caliButton D10

long output0;
long output1;
long touchMax;
long touchMin;

CapacitiveSensor cap0 = CapacitiveSensor(sendPin0, sensePin0);
CapacitiveSensor cap1 = CapacitiveSensor(sendPin1, sensePin1);

SemaphoreHandle_t sensorSemaphore = NULL;

void IRAM_ATTR calibrate() {
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;
  xSemaphoreGiveFromISR(sensorSemaphore, &xHigherPriorityTaskWoken);

}

void sensorTask(void* params){


}

void setup() {

  Serial.begin(115200);
  delay(300);
  Serial.println("begin");
  attachInterrupt(caliButton, calibrate, RISING);
  xTaskCreate(sensorTask, "sensor", 10000, NULL, 1, NULL);


  
}

void loop() {

  while(true) {
    output0 = cap0.capacitiveSensor(30);
    output1 = cap1.capacitiveSensor(30);

    Serial.println(output0);
    Serial.println(output1);

    vTaskDelay(200 / portTICK_PERIOD_MS);
  }
}
