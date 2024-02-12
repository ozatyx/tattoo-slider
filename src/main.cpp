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
#define MESSAGE "Placholder"

//ble definitions
#define DEVICE_NAME "ESP32_SLIDER"









CapacitiveSensor   cap0 = CapacitiveSensor(highRes0, senseRes0);  
CapacitiveSensor   cap1 = CapacitiveSensor(highRes0, senseRes1);

long reading0;
long reading1; 
long meanNum0;
long meanNum1;
double sliderValue;
long res;

void bluetoothTask(void*);
void typeText(const char* text);


bool isBleConnected = false;

void setup() {
  cap0.set_CS_AutocaL_Millis(0xFFFFFFFF);
  cap1.set_CS_AutocaL_Millis(0xFFFFFFFF);     // turn off autocalibrate on channel 1 - just as an example
  
  Serial.begin(115200);

  pinMode(ledPin, OUTPUT);

  xTaskCreate(bluetoothTask, "bluetooth", 20000, NULL, 5, NULL);
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

  res = map(meanNum0, 0, 310, 0, 255);
  Serial.println(res);
  analogWrite(ledPin, res);
  typeText(MESSAGE);
  
  
  
  
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

// Message (report) sent when a key is pressed or released
struct InputReport {
    uint8_t modifiers;	     // bitmask: CTRL = 1, SHIFT = 2, ALT = 4
    uint8_t reserved;        // must be 0
    uint8_t pressedKeys[6];  // up to six concurrenlty pressed keys
};

// Message (report) received when an LED's state changed
struct OutputReport {
    uint8_t leds;            // bitmask: num lock = 1, caps lock = 2, scroll lock = 4, compose = 8, kana = 16
};


// The report map describes the HID device (a keyboard in this case) and
// the messages (reports in HID terms) sent and received.
static const uint8_t REPORT_MAP[] = {
    USAGE_PAGE(1),      0x01,       // Generic Desktop Controls
    USAGE(1),           0x06,       // Keyboard
    COLLECTION(1),      0x01,       // Application
    REPORT_ID(1),       0x01,       //   Report ID (1)
    USAGE_PAGE(1),      0x07,       //   Keyboard/Keypad
    USAGE_MINIMUM(1),   0xE0,       //   Keyboard Left Control
    USAGE_MAXIMUM(1),   0xE7,       //   Keyboard Right Control
    LOGICAL_MINIMUM(1), 0x00,       //   Each bit is either 0 or 1
    LOGICAL_MAXIMUM(1), 0x01,
    REPORT_COUNT(1),    0x08,       //   8 bits for the modifier keys
    REPORT_SIZE(1),     0x01,       
    HIDINPUT(1),        0x02,       //   Data, Var, Abs
    REPORT_COUNT(1),    0x01,       //   1 byte (unused)
    REPORT_SIZE(1),     0x08,
    HIDINPUT(1),        0x01,       //   Const, Array, Abs
    REPORT_COUNT(1),    0x06,       //   6 bytes (for up to 6 concurrently pressed keys)
    REPORT_SIZE(1),     0x08,
    LOGICAL_MINIMUM(1), 0x00,
    LOGICAL_MAXIMUM(1), 0x65,       //   101 keys
    USAGE_MINIMUM(1),   0x00,
    USAGE_MAXIMUM(1),   0x65,
    HIDINPUT(1),        0x00,       //   Data, Array, Abs
    REPORT_COUNT(1),    0x05,       //   5 bits (Num lock, Caps lock, Scroll lock, Compose, Kana)
    REPORT_SIZE(1),     0x01,
    USAGE_PAGE(1),      0x08,       //   LEDs
    USAGE_MINIMUM(1),   0x01,       //   Num Lock
    USAGE_MAXIMUM(1),   0x05,       //   Kana
    LOGICAL_MINIMUM(1), 0x00,
    LOGICAL_MAXIMUM(1), 0x01,
    HIDOUTPUT(1),       0x02,       //   Data, Var, Abs
    REPORT_COUNT(1),    0x01,       //   3 bits (Padding)
    REPORT_SIZE(1),     0x03,
    HIDOUTPUT(1),       0x01,       //   Const, Array, Abs
    END_COLLECTION(0)               // End application collection
};


BLEHIDDevice* hid;
BLECharacteristic* input;
BLECharacteristic* output;

const InputReport NO_KEY_PRESSED = { };


/*
 * Callbacks related to BLE connection
 */
class BleKeyboardCallbacks : public BLEServerCallbacks {

    void onConnect(BLEServer* server) {
        isBleConnected = true;

        // Allow notifications for characteristics
        BLE2902* cccDesc = (BLE2902*)input->getDescriptorByUUID(BLEUUID((uint16_t)0x2902));
        cccDesc->setNotifications(true);

        Serial.println("Client has connected");
    }

    void onDisconnect(BLEServer* server) {
        isBleConnected = false;

        // Disallow notifications for characteristics
        BLE2902* cccDesc = (BLE2902*)input->getDescriptorByUUID(BLEUUID((uint16_t)0x2902));
        cccDesc->setNotifications(false);

        Serial.println("Client has disconnected");
    }
};


/*
 * Called when the client (computer, smart phone) wants to turn on or off
 * the LEDs in the keyboard.
 * 
 * bit 0 - NUM LOCK
 * bit 1 - CAPS LOCK
 * bit 2 - SCROLL LOCK
 */
 class OutputCallbacks : public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic* characteristic) {
        OutputReport* report = (OutputReport*) characteristic->getData();
        Serial.print("LED state: ");
        Serial.print((int) report->leds);
        Serial.println();
    }
};


void bluetoothTask(void*) {

    // initialize the device
    BLEDevice::init(DEVICE_NAME);
    BLEServer* server = BLEDevice::createServer();
    server->setCallbacks(new BleKeyboardCallbacks());

    // create an HID device
    hid = new BLEHIDDevice(server);
    input = hid->inputReport(1); // report ID
    output = hid->outputReport(1); // report ID
    output->setCallbacks(new OutputCallbacks());

    // set manufacturer name
    hid->manufacturer()->setValue("Maker Community");
    // set USB vendor and product ID
    hid->pnp(0x02, 0xe502, 0xa111, 0x0210);
    // information about HID device: device is not localized, device can be connected
    hid->hidInfo(0x00, 0x02);

    // Security: device requires bonding
    BLESecurity* security = new BLESecurity();
    security->setAuthenticationMode(ESP_LE_AUTH_BOND);

    // set report map
    hid->reportMap((uint8_t*)REPORT_MAP, sizeof(REPORT_MAP));
    hid->startServices();

    // set battery level to 100%
    hid->setBatteryLevel(100);

    // advertise the services
    BLEAdvertising* advertising = server->getAdvertising();
    advertising->setAppearance(HID_KEYBOARD);
    advertising->addServiceUUID(hid->hidService()->getUUID());
    advertising->addServiceUUID(hid->deviceInfo()->getUUID());
    advertising->addServiceUUID(hid->batteryService()->getUUID());
    advertising->start();

    Serial.println("BLE ready");
    delay(portMAX_DELAY);
};


void typeText(const char* text) {
    int len = strlen(text);
    for (int i = 0; i < len; i++) {

        // translate character to key combination
        uint8_t val = (uint8_t)text[i];
        if (val > KEYMAP_SIZE)
            continue; // character not available on keyboard - skip
        KEYMAP map = keymap[val];

        // create input report
        InputReport report = {
            .modifiers = map.modifier,
            .reserved = 0,
            .pressedKeys = {
                map.usage,
                0, 0, 0, 0, 0
            }
        };

        // send the input report
        input->setValue((uint8_t*)&report, sizeof(report));
        input->notify();

        delay(5);

        // release all keys between two characters; otherwise two identical
        // consecutive characters are treated as just one key press
        input->setValue((uint8_t*)&NO_KEY_PRESSED, sizeof(NO_KEY_PRESSED));
        input->notify();

        delay(5);
    }
}

void sendKeyCode(uint8_t keyCode) {
    // create input report
    InputReport report = {
        .modifiers = 0,
        .reserved = 0,
        .pressedKeys = { keyCode, 0, 0, 0, 0, 0 }
    };

    // send the input report
    input->setValue((uint8_t*)&report, sizeof(report));
    input->notify();
    delay(5);

    // release all keys
    input->setValue((uint8_t*)&NO_KEY_PRESSED, sizeof(NO_KEY_PRESSED));
    input->notify();
    delay(5);
}