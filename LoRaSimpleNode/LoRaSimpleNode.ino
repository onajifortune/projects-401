/*********
  Modified from the examples of the Arduino LoRa library
  More resources: https://randomnerdtutorials.com
*********/

#include <SPI.h>
#include <LoRa.h>
#include <M2M_LM75A.h>
#include <ArduinoJson.h>

M2M_LM75A lm75a;

//define the pins used by the transceiver module
#define ss 5
#define rst 14
#define dio0 2

// ANA: Antenna
// GND: GND
// DIO3: don’t connect
// DIO4: don’t connect
// 3.3V: 3.3V
// DIO0: GPIO 2
// DIO1: don’t connect
// DIO2: don’t connect
// GND: don’t connect
// DIO5: don’t connect
// RESET: GPIO 14
// NSS: GPIO 5
// SCK: GPIO 18
// MOSI: GPIO 23
// MISO: GPIO 19
// GND: don’t connect

const int HEART_RATE_SENSOR_PIN = 34;

float temperatureValueCel;
float temperatureValueFar;
int heartRateValue;

// Define your class
class SensorData {
public:
    int sensorNode;
    int heartRate;
    float temperatureCelsius;
    float temperatureFahrenheit;

    // Constructor
    SensorData(int sn, int hr, float tempC, float tempF) : sensorNode(sn), heartRate(hr), temperatureCelsius(tempC), temperatureFahrenheit(tempF) {}

    // Serialize the object into JSON
    String serializeToJson() {
        StaticJsonDocument<128> doc;
        doc["sensorNode"] = sensorNode;
        doc["heartRate"] = heartRate;
        doc["temperatureCelsius"] = temperatureCelsius;
        doc["temperatureFahrenheit"] = temperatureFahrenheit;
        String jsonStr;
        serializeJson(doc, jsonStr);
        return jsonStr;
    }
};


void setup() {
  //initialize Serial Monitor
  Serial.begin(115200);
  SPI.begin();
  lm75a.begin();
  while (!Serial);
  delay(5000);
  Serial.println("LoRa Sender");

  //setup LoRa transceiver module
  LoRa.setPins(ss, rst, dio0);
  
  //replace the LoRa.begin(---E-) argument with your location's frequency 
  //433E6 for Asia
  //866E6 for Europe
  //915E6 for North America
  while (!LoRa.begin(433E6)) {
    Serial.println(".");
    delay(500);
  }
   // Change sync word (0xF3) to match the receiver
  // The sync word assures you don't get LoRa messages from other LoRa transceivers
  // ranges from 0-0xFF
  LoRa.setSyncWord(0xF3);
  Serial.println("LoRa Initializing OK!");
}

void loop() {
  // if (lm75a.getTemperature() != -1000.00 ){
  int sensorValue = analogRead(HEART_RATE_SENSOR_PIN);
  
  if(sensorValue < 2000) {
    sensorValue = 0;
  }
  
  heartRateValue = sensorValue;
  temperatureValueCel = lm75a.getTemperature();
  temperatureValueFar = lm75a.getTemperatureInFarenheit();

  Serial.print("Sending packet: ");
  Serial.println(heartRateValue);
  // Temperature
  Serial.print(F("Temperature in Celsius: "));
  Serial.print(temperatureValueCel);
  Serial.println(F(" *C"));

  Serial.print(F("Temperature in Farenheit: "));
  Serial.print(temperatureValueFar);
  Serial.println(F(" *F"));
  Serial.println(F("==========================================="));
  Serial.println("");
  SensorData sensorData(1,heartRateValue, temperatureValueCel, temperatureValueFar);

  // Serialize the object to JSON
  String jsonData = sensorData.serializeToJson();

  // Convert JSON string to char array for LoRa transmission
  char jsonCharArr[jsonData.length() + 1];
  jsonData.toCharArray(jsonCharArr, jsonData.length() + 1);

  // Send the JSON data via LoRa
  LoRa.beginPacket();
  LoRa.print(jsonCharArr);
  LoRa.endPacket();
  Serial.print(jsonCharArr);

  // delay(1000);
  // }
}
