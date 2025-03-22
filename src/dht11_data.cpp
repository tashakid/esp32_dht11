#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <DHT.h>
#include <ArduinoJson.h>
#include "config.h"
#include <WiFiClientSecure.h>

// Pin Definitions
#define DHTPIN 4     // DHT11 data pin connected to GPIO 4
#define DHTTYPE DHT11

// WiFi credentials (from config.h)
const char* ssid = WIFI_SSID;
const char* password = WIFI_PASSWORD;

// API Endpoint (Lightning Studio API)
const char* apiEndpoint = API_ENDPOINT;

// Initialize DHT sensor
DHT dht(DHTPIN, DHTTYPE);

// Function declarations
void setup_wifi();
void sendSensorData();

void setup() {
  Serial.begin(115200);
  delay(1000); // Give time for the Serial Monitor to connect
  Serial.println("Program started");

  // Initialize DHT sensor
  dht.begin();
  
  // Setup WiFi
  setup_wifi();
}

void loop() {
  // Read sensor data and send via HTTP POST every 10 seconds
  sendSensorData();
  delay(10000);
}

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println();
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void sendSensorData() {
  // Read sensor data from the DHT11 sensor
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();

  // Check for sensor read errors
  if (isnan(humidity) || isnan(temperature)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }
  
  // Optional: Validate the readings (adjust ranges as needed)
  if (humidity < 0 || humidity > 100 || temperature < -40 || temperature > 80) {
    Serial.println("Sensor readings out of reasonable range!");
    return;
  }
  
  // Create JSON document for payload
  StaticJsonDocument<200> doc;
  doc["temperature"] = temperature;
  doc["humidity"] = humidity;
  doc["device_id"] = "ESP32_DHT11";
  char payload[200];
  serializeJson(doc, payload);
  
  // Debug print sensor values and payload
  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.print("°C, Humidity: ");
  Serial.print(humidity);
  Serial.println("%");
  Serial.print("Payload: ");
  Serial.println(payload);
  
  // Make sure WiFi is connected
  if (WiFi.status() == WL_CONNECTED) {
    WiFiClientSecure secureClient;
    HTTPClient http;

    secureClient.setInsecure();  // Disable certificate validation
    http.begin(secureClient, apiEndpoint);
    http.addHeader("Content-Type", "application/json");
    
    int httpResponseCode = http.POST(payload);
    
    if (httpResponseCode > 0) {
      String response = http.getString();
      Serial.print("HTTP Response Code: ");
      Serial.println(httpResponseCode);
      Serial.print("Response: ");
      Serial.println(response);
    } else {
      Serial.print("Error on sending POST: ");
      Serial.println(httpResponseCode);
    }
    http.end();
  } else {
    Serial.println("WiFi not connected");
  }
}
