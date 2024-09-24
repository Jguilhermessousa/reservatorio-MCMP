#include <Arduino.h>

#define BLYNK_TEMPLATE_ID "TMPL22GKXJn9j"
#define BLYNK_TEMPLATE_NAME "ESP32 WaterLevel"
#define BLYNK_AUTH_TOKEN "nR28VRP5izzhm_I6UypehIXd_aoD5c2C"

// #include <BLEDevice.h>
// #include <BLEServer.h>
// #include <BLEUtils.h>
// #include <BLE2902.h>

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h> 

// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = "REINALDO oi fibra 2.4G";
char pass[] = "nina1409";

// BLEServer* pServer = NULL;
// BLECharacteristic* pCharacteristic = NULL;
// BLECharacteristic* pCharacteristic_2 = NULL;
// BLEDescriptor *pDescr;
// BLE2902 *pBLE2902;

// bool deviceConnected = false;
// bool oldDeviceConnected = false;
// uint32_t value = 0;

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/

// #define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
// #define CHAR1_UUID          "beb5483e-36e1-4688-b7f5-ea07361b26a8"
// #define CHAR2_UUID          "e3223119-9445-4e96-a4a1-85358c4046a2"

// Ultrasonic sensor pins
#define TRIG_PIN 5   // Update with your TRIG pin
#define ECHO_PIN 18  // Update with your ECHO pin

#define wifiLed    2   //D2

//Change the virtual pins according the rooms
#define VPIN_BUTTON_1    V1 
#define VPIN_BUTTON_2    V2

//Set Water Level Distance in CM
int emptyTankDistance = 15 ;  //Distance when tank is empty
int fullTankDistance =  5 ;  //Distance when tank is full

//Set trigger value in percentage
int triggerPer =   10 ;  //alarm will start when water level drop below triggerPer

float duration;
float distance;
int   waterLevelPer;
bool  toggleBuzzer = HIGH; //Define to remember the toggle state

char auth[] = BLYNK_AUTH_TOKEN;

BlynkTimer timer;

// class MyServerCallbacks: public BLEServerCallbacks {
//     void onConnect(BLEServer* pServer) {
//       deviceConnected = true;
//     };

//     void onDisconnect(BLEServer* pServer) {
//       deviceConnected = false;
//       BLEDevice::startAdvertising();
//     }
// };

// class CharacteristicCallBack: public BLECharacteristicCallbacks {
//   void onWrite(BLECharacteristic *pChar) override { 
//     std::string pChar2_value_stdstr = pChar->getValue();
//     String pChar2_value_string = String(pChar2_value_stdstr.c_str());
//     int pChar2_value_int = pChar2_value_string.toInt();
//     Serial.println("pChar2: " + String(pChar2_value_int)); 
//   }
// };

void checkBlynkStatus() { // called every 3 seconds by SimpleTimer

  bool isconnected = Blynk.connected();
  if (isconnected == false) {
    //Serial.println("Blynk Not Connected");
    digitalWrite(wifiLed, LOW);
  }
  if (isconnected == true) {
    digitalWrite(wifiLed, HIGH);
    //Serial.println("Blynk Connected");
  }
}

BLYNK_CONNECTED() {
  Blynk.syncVirtual(VPIN_BUTTON_1);
  Blynk.syncVirtual(VPIN_BUTTON_2);
}

void setup() {
    Serial.begin(115200);

    // Setup ultrasonic sensor pins
    pinMode(TRIG_PIN, OUTPUT);
    pinMode(ECHO_PIN, INPUT);
    pinMode(wifiLed, OUTPUT);

    digitalWrite(wifiLed, LOW);

    WiFi.begin(ssid, pass);
    timer.setInterval(2000L, checkBlynkStatus); // check if Blynk server is connected every 2 seconds
    Blynk.config(auth);
    delay(1000);

    // // Create the BLE Device
    // BLEDevice::init("TesteBLE");

    // // Create the BLE Server
    // pServer = BLEDevice::createServer();
    // pServer->setCallbacks(new MyServerCallbacks());

    // // Create the BLE Service
    // BLEService *pService = pServer->createService(SERVICE_UUID);

    // // Create a BLE Characteristic
    // pCharacteristic = pService->createCharacteristic(
    //                     CHAR1_UUID,
    //                     BLECharacteristic::PROPERTY_NOTIFY
    //                     );                   

    // pCharacteristic_2 = pService->createCharacteristic(
    //                     CHAR2_UUID,
    //                     BLECharacteristic::PROPERTY_READ   |
    //                     BLECharacteristic::PROPERTY_WRITE  
    //                     );  

    // // Create a BLE Descriptor
    
    // pDescr = new BLEDescriptor((uint16_t)0x2901);
    // pDescr->setValue("A very interesting variable");
    // pCharacteristic->addDescriptor(pDescr);
    
    // pBLE2902 = new BLE2902();
    // pBLE2902->setNotifications(true);
    
    // // Add all Descriptors here
    // pCharacteristic->addDescriptor(pBLE2902);
    // pCharacteristic_2->addDescriptor(new BLE2902());
    
    // // After defining the desriptors, set the callback functions
    // pCharacteristic_2->setCallbacks(new CharacteristicCallBack());
    
    // // Start the service
    // pService->start();

    // // Start advertising
    // BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
    // pAdvertising->addServiceUUID(SERVICE_UUID);
    // pAdvertising->setScanResponse(false);
    // pAdvertising->setMinPreferred(0x0);  // set value to 0x00 to not advertise this parameter
    // BLEDevice::startAdvertising();
    // Serial.println("Waiting a client connection to notify...");
    }

    float readUltrasonicDistance() {
        digitalWrite(TRIG_PIN, LOW);
        delayMicroseconds(2);
        digitalWrite(TRIG_PIN, HIGH);
        delayMicroseconds(10);
        digitalWrite(TRIG_PIN, LOW);

        long duration = pulseIn(ECHO_PIN, HIGH);

        float distance = ((duration / 2) * 0.343)/10;

        // if (distance > (fullTankDistance - 10)  && distance < emptyTankDistance ){
            waterLevelPer = map((int)distance ,emptyTankDistance, fullTankDistance, 0, 100);
            Blynk.virtualWrite(VPIN_BUTTON_1, waterLevelPer);
            Blynk.virtualWrite(VPIN_BUTTON_2, (String(distance) + " cm"));

            // Print result to serial monitor
            Serial.print("Distance: ");
            Serial.print(distance);
            Serial.println(" cm");   
        // }
        
        // Delay before repeating measurement
        delay(100);

        return distance;
    }

void loop() {

    readUltrasonicDistance();

    // // notify changed value
    // if (deviceConnected) {
    //     float distance = readUltrasonicDistance();
    //     String distanceStr = String(distance, 2);
    //     pCharacteristic->setValue(distanceStr.c_str());
    //     pCharacteristic->notify();
    //     Serial.printf("%.2f", distance);
    //     delay(1000);
    // }
    // // disconnecting
    // if (!deviceConnected && oldDeviceConnected) {
    //     delay(500); // give the bluetooth stack the chance to get things ready
    //     pServer->startAdvertising(); // restart advertising
    //     Serial.println("start advertising");
    //     oldDeviceConnected = deviceConnected;
    // }
    // // connecting
    // if (deviceConnected && !oldDeviceConnected) {
    //     // do stuff here on connecting
    //     oldDeviceConnected = deviceConnected;
    // }

    Blynk.run();
    timer.run(); // Initiates SimpleTimer
}