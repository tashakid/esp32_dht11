#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>
#include <ArduinoJson.h>

// Pin Definitions
#define DHTPIN 4     // DHT11 data pin connected to GPIO 4
#define DHTTYPE DHT11

// WiFi credentials
const char* ssid = "WirelessNet";
const char* password = "DBSB3272 GLOBEABB";  // Replace with your actual password

// MQTT Broker settings
const char* mqtt_server = "10.192.12.29";
const int mqtt_port = 1883;  // Standard MQTT port, not Node-RED web port
const char* mqtt_topic = "sensors/dht11";

// Initialize DHT sensor
DHT dht(DHTPIN, DHTTYPE);

// Initialize WiFi and MQTT clients
WiFiClient espClient;
PubSubClient client(espClient);

// Variables
unsigned long lastReconnectAttempt = 0;
const unsigned long reconnectInterval = 5000;  // Try every 5 seconds

// Function declarations
void setup_wifi();
boolean reconnect();
void publishSensorData();

void setup() {
    Serial.begin(115200);
    
    // Initialize DHT sensor
    dht.begin();
    
    // Setup WiFi
    setup_wifi();
    
    // Configure MQTT
    client.setServer(mqtt_server, mqtt_port);
}

void loop() {
    // Handle MQTT connection
    if (!client.connected()) {
        unsigned long now = millis();
        if (now - lastReconnectAttempt > reconnectInterval) {
            lastReconnectAttempt = now;
            if (reconnect()) {
                lastReconnectAttempt = 0;
            }
        }
    } else {
        client.loop();
    }
    
    // Publish sensor data every 5 seconds
    static unsigned long lastMsg = 0;
    unsigned long now = millis();
    if (now - lastMsg > 5000) {
        lastMsg = now;
        publishSensorData();
    }
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

    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
}

boolean reconnect() {
    Serial.print("Attempting MQTT connection...");
    String clientId = "ESP32Client-";
    clientId += String(random(0xffff), HEX);
    
    if (client.connect(clientId.c_str())) {
        Serial.println("connected");
        return true;
    } else {
        Serial.print("failed, rc=");
        Serial.print(client.state());
        Serial.println(" try again later");
        return false;
    }
}

void publishSensorData() {
    float humidity = dht.readHumidity();
    float temperature = dht.readTemperature();

    // Validate sensor readings
    if (isnan(humidity) || isnan(temperature)) {
        Serial.println("Failed to read from DHT sensor!");
        return;
    }
    
    // Optional: range validation
    if (humidity < 0 || humidity > 100 || temperature < -40 || temperature > 80) {
        Serial.println("Sensor readings out of reasonable range!");
        return;
    }

    // Create JSON document
    StaticJsonDocument<200> doc;
    doc["temperature"] = temperature;
    doc["humidity"] = humidity;
    doc["device_id"] = "ESP32_DHT11";  // Added device identifier
    
    char buffer[200];
    serializeJson(doc, buffer);
    
    // Publish to MQTT
    boolean success = client.publish(mqtt_topic, buffer);
    
    // Debug output
    Serial.print("Temperature: ");
    Serial.print(temperature);
    Serial.print("°C, Humidity: ");
    Serial.print(humidity);
    Serial.print("%, Publish ");
    Serial.println(success ? "successful" : "failed");
}