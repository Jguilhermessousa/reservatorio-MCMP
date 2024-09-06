#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

BLEServer* pServer = NULL;
BLECharacteristic* pCharacteristic = NULL;
BLECharacteristic* pCharacteristic_2 = NULL;
BLEDescriptor *pDescr;
BLE2902 *pBLE2902;

bool deviceConnected = false;
bool oldDeviceConnected = false;

// UUIDs for BLE service and characteristics
// See the following for generating UUIDs:
// https://www.uuidgenerator.net/

#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHAR1_UUID          "beb5483e-36e1-4688-b7f5-ea07361b26a8"
#define CHAR2_UUID          "e3223119-9445-4e96-a4a1-85358c4046a2"

// Ultrasonic sensor pins
#define TRIG_PIN 5   // Update with your TRIG pin
#define ECHO_PIN 18  // Update with your ECHO pin

class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
        deviceConnected = true;
    };

    void onDisconnect(BLEServer* pServer) {
        deviceConnected = false;
    }
};

void setup() {
    Serial.begin(115200);

    // Setup ultrasonic sensor pins
    pinMode(TRIG_PIN, OUTPUT);
    pinMode(ECHO_PIN, INPUT);

    // Create the BLE Device
    BLEDevice::init("Ultrasonic");

    // Create the BLE Server
    pServer = BLEDevice::createServer();
    pServer->setCallbacks(new MyServerCallbacks());

    // Create the BLE Service
    BLEService *pService = pServer->createService(SERVICE_UUID);

    // Create BLE Characteristics
    pCharacteristic = pService->createCharacteristic(
                        CHAR1_UUID,
                        BLECharacteristic::PROPERTY_NOTIFY
                      );                   

    pCharacteristic_2 = pService->createCharacteristic(
                        CHAR2_UUID,
                        BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY
                      );  

    // Create a BLE Descriptor
    pDescr = new BLEDescriptor((uint16_t)0x2901);
    pDescr->setValue("Ultrasonic Sensor Data");
    pCharacteristic->addDescriptor(pDescr);
    
    pBLE2902 = new BLE2902();
    pBLE2902->setNotifications(true);

    // Add Descriptors
    pCharacteristic->addDescriptor(pBLE2902);
    pCharacteristic_2->addDescriptor(new BLE2902());

    // Start the service
    pService->start();

    // Start advertising
    BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->setScanResponse(false);
    pAdvertising->setMinPreferred(0x0);  // set value to 0x00 to not advertise this parameter
    BLEDevice::startAdvertising();
    Serial.println("Waiting for a client connection to notify...");
}

float readUltrasonicDistance() {
    // Send a 10us pulse to TRIG_PIN
    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);

    // Measure the pulse duration on ECHO_PIN
    long duration = pulseIn(ECHO_PIN, HIGH);

    // Calculate the distance in centimeters
    float distance = (duration * 0.0343) / 2;
    return distance;
}

void loop() {
    // Notify only when the device is connected
    if (deviceConnected) {
        float distance = readUltrasonicDistance();

        // Convert the distance to a string and set it as the value for the characteristic
        String distanceStr = String(distance, 2); // Convert distance to a string with 2 decimal places
        pCharacteristic_2->setValue(distanceStr.c_str()); // Send the value as a string
        pCharacteristic_2->notify(); // Notify the connected device
        Serial.printf("Distance: %.2f cm\n", distance);
        delay(1000);  // Adjust delay as needed
    }

    // Handling device disconnection
    if (!deviceConnected && oldDeviceConnected) {
        delay(500); // Give the Bluetooth stack time to get ready
        pServer->startAdvertising(); // Restart advertising
        Serial.println("Start advertising");
        oldDeviceConnected = deviceConnected;
    }

    // Handling device connection
    if (deviceConnected && !oldDeviceConnected) {
        // Do stuff here on connecting
        oldDeviceConnected = deviceConnected;
    }
}