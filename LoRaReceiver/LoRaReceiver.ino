#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>
#include <SPI.h>
#include <LoRa.h>
#include <ArduinoJson.h>

// define the pins used by the transceiver module
#define ss 5
#define rst 14
#define dio0 2

const char *ssid = "MyNetwork";
const char *password = "ESP32Password";
const int serverPort = 80;

WiFiClient client;
AsyncWebServer server(serverPort);

class SensorData
{
public:
    int sensorNode;
    int heartRate;
    float temperatureCelsius;
    float temperatureFahrenheit;

    SensorData() : sensorNode(0), heartRate(0), temperatureCelsius(0), temperatureFahrenheit(0) {}

    String toJson()
    {
        StaticJsonDocument<128> doc;
        doc["sensorNode"] = sensorNode;
        doc["heartRate"] = heartRate;
        doc["temperatureCelsius"] = temperatureCelsius;
        doc["temperatureFahrenheit"] = temperatureFahrenheit;
        String output;
        serializeJson(doc, output);
        return output;
    }

    void deserialize(const char *jsonString)
    {
        StaticJsonDocument<128> doc;
        deserializeJson(doc, jsonString);
        sensorNode = doc["sensorNode"];
        heartRate = doc["heartRate"];
        temperatureCelsius = doc["temperatureCelsius"];
        temperatureFahrenheit = doc["temperatureFahrenheit"];
    }
};

SensorData sensorData1;
SensorData sensorData2;

void setup()
{
    Serial.begin(115200);
    SPI.begin();
    while (!Serial)
        ;
    delay(5000);
    Serial.println("LoRa Receiver");

    LoRa.setPins(ss, rst, dio0);

    while (!LoRa.begin(433E6))
    {
        Serial.println("Starting LoRa failed!");
        delay(500);
    }

    LoRa.setSyncWord(0xF3);
    Serial.println("LoRa Initializing OK!");

    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(1000);
        Serial.println("Connecting to WiFi...");
    }
    Serial.println("Connected to WiFi");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());

    if (!SPIFFS.begin())
    {
        Serial.println("An error occurred while mounting SPIFFS");
        return;
    }

    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send(SPIFFS, "/index.html"); });

    server.on("/heartRate1", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send(200, "application/json", String(sensorData1.heartRate)); });

    server.on("/temperatureCelsius1", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send(200, "application/json", String(sensorData1.temperatureCelsius)); });

    server.on("/temperatureFahrenheit1", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send(200, "application/json", String(sensorData1.temperatureFahrenheit)); });

    server.on("/heartRate2", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send(200, "application/json", String(sensorData2.heartRate)); });

    server.on("/temperatureCelsius2", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send(200, "application/json", String(sensorData2.temperatureCelsius)); });

    server.on("/temperatureFahrenheit2", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send(200, "application/json", String(sensorData2.temperatureFahrenheit)); });

    server.begin();
}

void loop()
{
    int packetSize = LoRa.parsePacket();
    if (packetSize)
    {
        char receivedData[packetSize + 1];
        for (int i = 0; i < packetSize; i++)
        {
            receivedData[i] = (char)LoRa.read();
        }
        receivedData[packetSize] = '\0'; // Null-terminate the string

        SensorData tempData;
        tempData.deserialize(receivedData);

        if (tempData.sensorNode == 1)
        {
            sensorData1 = tempData;
            Serial.println("Received data from sensor node 1");
            Serial.print("Heart Rate: ");
            Serial.println(sensorData1.heartRate);
            Serial.print("Temperature in Celsius: ");
            Serial.print(sensorData1.temperatureCelsius);
            Serial.println(" *C");
            Serial.print("Temperature in Fahrenheit: ");
            Serial.print(sensorData1.temperatureFahrenheit);
            Serial.println(" *F");
        }
        if (tempData.sensorNode != 1)
        {
            sensorData2 = tempData;
            Serial.println("Received data from sensor node 2");
            Serial.print("Heart Rate: ");
            Serial.println(sensorData2.heartRate);
            Serial.print("Temperature in Celsius: ");
            Serial.print(sensorData2.temperatureCelsius);
            Serial.println(" *C");
            Serial.print("Temperature in Fahrenheit: ");
            Serial.print(sensorData2.temperatureFahrenheit);
            Serial.println(" *F");
        }

        Serial.println("=========================");
    }
}
