#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <ArduinoJson.h>
#include "config.h"  // Contains definitions for WIFI_SSID and WIFI_PASSWORD

// WiFi credentials (from config.h)
const char* ssid = WIFI_SSID;
const char* password = WIFI_PASSWORD;

// Command endpoint on your Lightning Studio server
const char* commandEndpoint = API_ENDPOINT;

// Initialize I2C LCD (example for a 16x2 LCD with I2C backpack)
// Adjust the I2C address if needed (commonly 0x27 or 0x3F)
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Function prototypes
void setup_wifi();
void pollForCommand();
void displayMessage(const char* message);

void setup() {
  Serial.begin(115200);
  delay(1000); // Give time for Serial Monitor to initialize
  Serial.println("Program started - Command Poller");

  // Initialize I2C on SDA = 2 and SCL = 3 (update to match your wiring)
  Wire.begin(2, 3);

  // Initialize the LCD
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Connecting WiFi");

  // Connect to WiFi
  setup_wifi();

  // Once connected, display success message
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("WiFi Connected");
  delay(2000);
  lcd.clear();
}

void loop() {
  // Poll for commands every 15 seconds
  static unsigned long lastCommandPoll = 0;
  if (millis() - lastCommandPoll > 15000) {
    lastCommandPoll = millis();
    pollForCommand();
  }
  delay(100); // Prevent busy looping
}

// Connect to WiFi
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

// Poll the server for command via HTTP GET
void pollForCommand() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(commandEndpoint);
    int httpResponseCode = http.GET();
    if (httpResponseCode == 200) {
      String response = http.getString();
      Serial.println("Received command payload:");
      Serial.println(response);

      // Parse JSON command
      StaticJsonDocument<256> doc;
      DeserializationError error = deserializeJson(doc, response);
      if (!error) {
        const char* cmd = doc["command"];
        if (strcmp(cmd, "display") == 0) {
          JsonArray messages = doc["messages"].as<JsonArray>();
          for (JsonVariant v : messages) {
            const char* msg = v.as<const char*>();
            displayMessage(msg);
            delay(2000); // 2-second delay between messages
          }
        } else {
          Serial.print("Unknown command received: ");
          Serial.println(cmd);
        }
      } else {
        Serial.print("JSON parse error: ");
        Serial.println(error.c_str());
      }
    } else {
      Serial.print("GET command failed, HTTP code: ");
      Serial.println(httpResponseCode);
    }
    http.end();
  } else {
    Serial.println("WiFi not connected for GET command");
  }
}

// Display a message on the LCD
void displayMessage(const char* message) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Cmd: Display");
  lcd.setCursor(0, 1);
  lcd.print(message);
  Serial.print("Displaying message: ");
  Serial.println(message);
}
