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
long safetyVal = 1000;    //will need to do some testing to set this
long output;

CapacitiveSensor cap0 = CapacitiveSensor(sendPin0, sensePin0);
CapacitiveSensor cap1 = CapacitiveSensor(sendPin1, sensePin1);
TaskHandle_t calibration = NULL;
SemaphoreHandle_t sensorMutex;     //Either given to the calibration task, or the main task to access sensors

void calibrateHandler();
void calibrate(void* params);
void sensorTask(void* params);

struct extremeValues{     //Holds values for the min and max of a sensor
  long min;               //trying to find default values, but can be set
  long max;               //with setCalibrationValues()
} ext0, ext1, extremeValues;

void setCalibrationValues(){
  bool heldSensorLongEnough = false;
  int newTime = 0;
  int curTime;   
  int count;

  for(int x = 0; x < 4; x++){
    vTaskDelay(200 / portTICK_PERIOD_MS);   //Reads & averages the minimum values for sensor 0 while flashing.
    analogWrite(ledPin0, 255);            //Sensor should not be touched during this time,
    vTaskDelay(200 / portTICK_PERIOD_MS);   //otherwise values are thrown off and must be calibrated again.
    analogWrite(ledPin0, 0);
    
    output0 = cap0.capacitiveSensor(30);
    ext0.min += output0;

  }
  ext0.min = ext0.min/4;
  
  while(!heldSensorLongEnough){               //Now time to determine the max value. This is done by touching the sensor.
    curTime = millis();                       //This will keep looping until a value larger than min+safety is found
    count = 0;                                //and stays the same for more than 1.5 seconds. 

    while(cap0.capacitiveSensor(30) > ext0.min+safetyVal){
      newTime = millis();
      ext0.max += cap0.capacitiveSensor(30);
      count++;
      analogWrite(ledPin0, 255);
    }
    analogWrite(ledPin0, 0);
    ext0.max = ext0.max / count;
    
    if(newTime - curTime > 1500){
      heldSensorLongEnough = true;  
                
      for(int x = 0; x < 6; x++){
        vTaskDelay(100 / portTICK_PERIOD_MS);
        analogWrite(ledPin0, 255);          //Good job, calibrated succesfully! Will break the loop after this flashing.
        vTaskDelay(100 / portTICK_PERIOD_MS);
        analogWrite(ledPin0, 0);
      }
    }
  }
  heldSensorLongEnough = false;
  newTime = 0;

  for(int x = 0; x < 4; x++){               //calibrate the second sensor.
    analogWrite(ledPin1, 255);
    vTaskDelay(200 / portTICK_PERIOD_MS);  
    analogWrite(ledPin1, 0);
    vTaskDelay(200 / portTICK_PERIOD_MS); 

    output1 = cap1.capacitiveSensor(30);
    ext1.min += output1;
  }
  ext1.min = ext1.min/4;
  

  while(!heldSensorLongEnough){               
    curTime = millis();                      
    count = 0;                         
                                                  
    while(cap1.capacitiveSensor(30) > ext1.min+safetyVal){
      newTime = millis();
      ext1.max += cap1.capacitiveSensor(30);
      count++;
      analogWrite(ledPin1, 255);
    } 
    analogWrite(ledPin1, 0);
    ext1.max = ext1.max / count;
    
    if(newTime - curTime > 1500){
      heldSensorLongEnough = true;  
      
      for(int x = 0; x < 6; x++){
        vTaskDelay(100 / portTICK_PERIOD_MS);
        analogWrite(ledPin1, 255);          //Good job, calibrated succesfully! Will break the loop after this flashing.
        vTaskDelay(100 / portTICK_PERIOD_MS);
        analogWrite(ledPin1, 0);
      }
  
    }

    

  } 
  heldSensorLongEnough = false;
  extremeValues.max = (ext0.max+ext1.max)/2;
  extremeValues.min = (ext0.min+ext0.min)/2;
}

void IRAM_ATTR calibrateHandler() {
  vTaskNotifyGiveIndexedFromISR(  calibration,         /*Notify this task handle*/
                                  0,                   /*Notify this index*/
                                  NULL      
                                );   
}

long bounder (long sensorVal){
  if(sensorVal > extremeValues.max){
    sensorVal = extremeValues.max;
  }

  if(sensorVal < extremeValues.min){
    sensorVal = extremeValues.min;
  }
  return sensorVal;
}

void sensorTask(void* params){
  sensorMutex = xSemaphoreCreateMutex();

  while(true){
    if(xSemaphoreTake(sensorMutex, portMAX_DELAY) == pdTRUE){
        output0 = cap0.capacitiveSensor(30); //left sensor (in demo video)
        output0 = bounder(output0);
        output1 = cap1.capacitiveSensor(30); //right sensor
        output1 = bounder(output1);
        //output = output1 - output0 + extremeValues.max;
        //output = map(output, extremeValues.min, extremeValues.max, 0, 255);
        output = output1/(output0+output1);
        analogWrite(ledPin0, output);

        Serial.println(output);
        Serial.print(">output0:");
        Serial.println(output0);
        Serial.print(">output1:");
        Serial.println(output1);
        //Serial.println(uxTaskGetStackHighWaterMark(NULL));

        xSemaphoreGive(sensorMutex);
        
    }
    vTaskDelay(100 / portTICK_PERIOD_MS);

  }
  vTaskDelete(NULL);
}
  
void calibrate(void* params){
  uint32_t notificationValue = 0;

  while(true){    
    notificationValue = ulTaskNotifyTakeIndexed( 0,                     /*Wait for notification at index 0*/
                                                 pdTRUE,                /*Reset notification bit to 0 on task exit*/
                                                 pdMS_TO_TICKS(200) );  /*Wait this long for a notification*/
    
    if(notificationValue == 1){
      if(xSemaphoreTake(sensorMutex, portMAX_DELAY) == pdTRUE){
          analogWrite(ledPin0, 0);
          ext0.min = 0;
          ext1.min = 0;
          setCalibrationValues();
          xSemaphoreGive(sensorMutex);
      }
    }
  }
  vTaskDelete(NULL);
}

  



void setup() {
  Serial.begin(115200);
  //Serial.println("begin");

  pinMode(caliButton, INPUT);
  attachInterrupt(digitalPinToInterrupt(caliButton), calibrateHandler, HIGH);
  pinMode(ledPin0, OUTPUT);
  pinMode(ledPin1, OUTPUT);
  setCalibrationValues();

  Serial.println(ext0.min);
  Serial.println(ext0.max);
  Serial.println(ext1.min);
  Serial.println(ext1.max);

  xTaskCreate(sensorTask, "sensor", 20000, NULL, 2, NULL);
  xTaskCreate(calibrate, "calibration", 20000, NULL, 1, &calibration);
  
  
  


  
}

void loop() {

}

