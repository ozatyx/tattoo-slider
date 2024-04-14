#include <Arduino.h>
#include "CapacitiveSensor.h"
#include "NimBLEDevice.h"

#define sendPin0 D1
#define sendPin1 D4       //send0 goes to sense0, etc
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

TaskHandle_t calibration = NULL;

void calibrate(void* params);
void calibrateHandler();
void sensorTask(void* params);

void IRAM_ATTR calibrateHandler() {
  uint32_t ulInterruptStatus;
  xTaskNotifyIndexedFromISR(calibration, 0, ulInterruptStatus, eSetBits, NULL);
}

void sensorTask(void* params){
  pinMode(caliButton, INPUT);
  attachInterrupt(digitalPinToInterrupt(caliButton), calibrateHandler, HIGH);

  while(true){
    output0 = cap0.capacitiveSensor(30);
    output1 = cap1.capacitiveSensor(30);

    Serial.println(output0);
    Serial.println(output1);

    vTaskDelay(200 / portTICK_PERIOD_MS);
    Serial.println(uxTaskGetStackHighWaterMark(NULL));
  }

  vTaskDelete(NULL);
}
  
void calibrate(void* params){
  uint32_t ulInterruptStatus;
  while(true){
    xTaskNotifyWaitIndexed( 0,                  /* Wait for 0th Notificaition */
                            0x00,               /* Do\n't clear any bits on entry. */
                            ULONG_MAX,          /* Clear all bits on exit. */
                            &ulInterruptStatus, /* Receives the notification value. */
                            portMAX_DELAY );

    if (( ulInterruptStatus & 0x01 ) != 0 ){
      for(int x = 0; x < 4; x++){
            digitalWrite(ledPin0, HIGH);
            vTaskDelay(200 / portTICK_PERIOD_MS);
            digitalWrite(ledPin0, LOW);
          }

      for(int x = 0; x < 4; x++){
        digitalWrite(ledPin1, HIGH);
        vTaskDelay(200 / portTICK_PERIOD_MS);
        digitalWrite(ledPin1, LOW);
      }

    }                           
    
  }
  
}


void setup() {
  Serial.begin(115200);
  delay(300);
  Serial.println("begin");
  pinMode(ledPin0, OUTPUT);
  pinMode(ledPin1, OUTPUT);
  
  xTaskCreate(sensorTask, "sensor", 10000, NULL, 1, NULL);
  xTaskCreate(calibrate, "calibration", 10000, NULL, 1, &calibration);
  
  
  


  
}

void loop() {

}
