/*
 * UNIFIED GREENHOUSE SENSOR MONITORING SYSTEM
 * IFS325 Group Project - ARC Smart Agriculture
 * MQTT Publishing Version
 *
 * This version sends sensor data to an MQTT broker instead of
 * using HTTP to connect directly to Oracle APEX.
 */

#include "Arduino.h"
#include <Wire.h>
#include <Adafruit_BMP280.h>
#include "DHT.h"
#include <WiFi.h>
#include <WiFiClient.h>      // Use standard WiFiClient
#include <PubSubClient.h>    // The MQTT library
#include <ArduinoJson.h>
#include <time.h>

// ============ PIN DEFINITIONS (Unchanged) ============
#define FLAME_SENSOR_PIN 35
#define DHT_PIN 16
#define LDR_PIN 4
#define DHTTYPE DHT22
#define SDA_PIN 21
#define SCL_PIN 22
#define MQ135_PIN 32
#define MQ2_PIN 33
#define MQ7_PIN 14

// ============ WIFI CREDENTIALS (Unchanged) ============
const char* ssid = "Durham12B-Room1"; // Your WiFi Name
const char* password = "BHANA64#"; // Your WiFi Password

// ============ MQTT & BROKER CREDENTIALS ============
const char* mqtt_server = "192.168.0.104"; // Public test broker
const int   mqtt_port = 1883;
const char* mqtt_topic = "greenhouse/sensor_data";
const char* mqtt_client_id = "esp32-greenhouse-monitor-123"; // Must be unique

// ============ SENSOR OBJECT INITIALIZATION ============
Adafruit_BMP280 bmp;
DHT dht(DHT_PIN, DHTTYPE);
WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

// ============ CALIBRATION & BASELINE VALUES (Unchanged) ============
const float PRESSURE_NORMAL = 1013.25;
float baselinePressure = PRESSURE_NORMAL;
bool baselineSet = false;
int mq135_baseline = 0;
int mq2_baseline = 0;
int mq7_baseline = 0;
bool gas_sensors_calibrated = false;
const int FLAME_THRESHOLD = 2000;

// ============ TIMING VARIABLES (Unchanged) ============
unsigned long lastReadTime = 0;
const long READ_INTERVAL = 10000;

// ============ JSON PAYLOAD STRUCTURE (Unchanged) ============
struct SensorPayload {
  float temperature_bmp280, temperature_dht22, pressure, altitude, humidity;
  int flame_raw, light_raw;
  int mq135_raw, mq135_baseline, mq135_drop;
  int mq2_raw, mq2_baseline, mq2_drop;
  int mq7_raw, mq7_baseline, mq7_drop;
  long timestamp;
};
SensorPayload sensorData;


// ============ SETUP ============
void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("\n\nUNIFIED GREENHOUSE SENSOR MONITORING SYSTEM (MQTT Version)");

  // --- Sensor and WiFi setup (mostly unchanged) ---
  Wire.begin(SDA_PIN, SCL_PIN);
  if (!bmp.begin(0x76)) { if (!bmp.begin(0x77)) { Serial.println("❌ BMP280 not found!"); while (1) delay(10); } }
  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL, Adafruit_BMP280::SAMPLING_X2, Adafruit_BMP280::SAMPLING_X16, Adafruit_BMP280::FILTER_X16, Adafruit_BMP280::STANDBY_MS_500);
  Serial.println("✓ BMP280 initialized");
  dht.begin();
  Serial.println("✓ DHT22 initialized");
  pinMode(FLAME_SENSOR_PIN, INPUT); pinMode(LDR_PIN, INPUT); pinMode(MQ135_PIN, INPUT); pinMode(MQ2_PIN, INPUT); pinMode(MQ7_PIN, INPUT);
  Serial.println("✓ Analog pins configured");

  connectToWiFi();
  configTime(0, 0, "pool.ntp.org", "time.nist.gov");

  // --- NEW: Configure MQTT Client ---
  mqttClient.setServer(mqtt_server, mqtt_port);
  Serial.println("✓ MQTT client configured");

  // --- Gas Sensor Calibration (Unchanged) ---
  Serial.println("\n--- GAS SENSOR CALIBRATION PHASE ---");
  Serial.println("Gas sensors warming up (300 seconds)...");
  for (int i = 300; i > 0; i--) { if (i % 30 == 0) { Serial.printf("Warming up: %d seconds remaining...\n", i); } delay(1000); }
  calibrateGasSensors();
  Serial.println("\n--- INITIALIZATION COMPLETE - MONITORING STARTED ---\n");
}

// ============ MAIN LOOP ============
void loop() {
  // Maintain WiFi connection
  if (!WiFi.isConnected()) {
    connectToWiFi();
  }

  // NEW: Maintain MQTT connection
  if (!mqttClient.connected()) {
    reconnectMqtt();
  }
  mqttClient.loop(); // This is essential to keep the connection alive

  // Read and publish sensor data at the specified interval
  if (millis() - lastReadTime >= READ_INTERVAL) {
    lastReadTime = millis();
    readAllSensors();
    printSensorData();
    publishMqttMessage(); // Replaced sendToOracleAPEX()
  }
}

// ============ [NEW] MQTT RECONNECT LOGIC ============
void reconnectMqtt() {
  while (!mqttClient.connected()) {
    Serial.print("\nAttempting MQTT connection...");
    if (mqttClient.connect(mqtt_client_id)) {
      Serial.println("✓ connected");
    } else {
      Serial.print("❌ failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" | Retrying in 5 seconds...");
      delay(5000);
    }
  }
}

// ============ [NEW] PUBLISH MQTT MESSAGE ============
void publishMqttMessage() {
  if (!mqttClient.connected()) {
    Serial.println("❌ MQTT client not connected, skipping publish.");
    return;
  }

  String jsonPayload = createJsonPayload();
  const char* payload = jsonPayload.c_str();

  Serial.println("\n--- PUBLISHING TO MQTT BROKER ---");
  Serial.print("Topic: ");
  Serial.println(mqtt_topic);
  Serial.print("Payload: ");
  Serial.println(payload);

  if (mqttClient.publish(mqtt_topic, payload)) {
    Serial.println("✓ Message published successfully");
  } else {
    Serial.println("❌ Message publication failed");
  }
  Serial.println("----------------------------------\n");
}

// ============ WIFI CONNECTION (Unchanged) ============
void connectToWiFi() {
  Serial.print("Connecting to WiFi: ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) { delay(500); Serial.print("."); attempts++; }
  if (WiFi.isConnected()) { Serial.print("\n✓ WiFi connected\nIP address: "); Serial.println(WiFi.localIP());
  } else { Serial.println("\n❌ WiFi connection failed"); }
}

// ============ CREATE JSON PAYLOAD (Unchanged) ============
String createJsonPayload() {
  StaticJsonDocument<1024> doc;
  doc["temperature_bmp280"] = sensorData.temperature_bmp280;
  doc["temperature_dht22"]  = sensorData.temperature_dht22;
  doc["pressure"]           = sensorData.pressure;
  doc["altitude"]           = sensorData.altitude;
  doc["humidity"]           = sensorData.humidity;
  doc["flame_raw"]          = sensorData.flame_raw;
  doc["light_raw"]          = sensorData.light_raw;
  doc["mq135_raw"]          = sensorData.mq135_raw;
  doc["mq135_baseline"]     = sensorData.mq135_baseline;
  doc["mq135_drop"]         = sensorData.mq135_drop;
  doc["mq2_raw"]            = sensorData.mq2_raw;
  doc["mq2_baseline"]       = sensorData.mq2_baseline;
  doc["mq2_drop"]           = sensorData.mq2_drop;
  doc["mq7_raw"]            = sensorData.mq7_raw;
  doc["mq7_baseline"]       = sensorData.mq7_baseline;
  doc["mq7_drop"]           = sensorData.mq7_drop;
  doc["timestamp"]          = sensorData.timestamp;
  String jsonString;
  serializeJson(doc, jsonString);
  return jsonString;
}

// ============ All other functions (readAllSensors, printSensorData, calibrateGasSensors) remain exactly the same ============
void readAllSensors() {
  time_t now = time(nullptr);
  sensorData.timestamp = (long)now;
  sensorData.temperature_bmp280 = bmp.readTemperature();
  sensorData.pressure = bmp.readPressure() / 100.0F;
  if (!baselineSet) { baselinePressure = sensorData.pressure; baselineSet = true; }
  sensorData.altitude = bmp.readAltitude(PRESSURE_NORMAL);
  sensorData.temperature_dht22 = dht.readTemperature();
  sensorData.humidity = dht.readHumidity();
  sensorData.flame_raw = analogRead(FLAME_SENSOR_PIN);
  sensorData.light_raw = analogRead(LDR_PIN);
  sensorData.mq135_raw = analogRead(MQ135_PIN);
  sensorData.mq135_baseline = mq135_baseline;
  sensorData.mq135_drop = sensorData.mq135_raw - mq135_baseline;
  sensorData.mq2_raw = analogRead(MQ2_PIN);
  sensorData.mq2_baseline = mq2_baseline;
  sensorData.mq2_drop = sensorData.mq2_raw - mq2_baseline;
  sensorData.mq7_raw = analogRead(MQ7_PIN);
  sensorData.mq7_baseline = mq7_baseline;
  sensorData.mq7_drop = sensorData.mq7_raw - mq7_baseline;
}
void printSensorData() {
  Serial.printf("\n--- SENSOR READING CYCLE (%lus) ---\n", millis() / 1000);
  Serial.printf("🌍 BAROMETRIC PRESSURE (BMP280)\n   Temperature: %.1f °C\n   Pressure: %.2f hPa\n   Altitude (approx): %.1f m\n", sensorData.temperature_bmp280, sensorData.pressure, sensorData.altitude);
  Serial.println("\n💧 TEMPERATURE & HUMIDITY (DHT22)");
  if (isnan(sensorData.humidity) || isnan(sensorData.temperature_dht22)) { Serial.println("   ❌ Sensor read error!"); }
  else { Serial.printf("   Temperature: %.1f °C\n   Humidity: %.1f %%\n", sensorData.temperature_dht22, sensorData.humidity); }
  Serial.printf("\n🔥 FLAME DETECTION SENSOR\n   Raw Value: %d\n   Status: %s\n", sensorData.flame_raw, (sensorData.flame_raw < FLAME_THRESHOLD ? "⚠⚠ FLAME DETECTED ⚠⚠" : "✓ No flame detected"));
  Serial.printf("\n💡 AMBIENT LIGHT SENSOR (LDR)\n   Raw Value: %d\n", sensorData.light_raw);
  Serial.printf("\n🌫️  MQ135 - CO2/AIR QUALITY\n   Raw: %d | Baseline: %d | Drop: %d\n", sensorData.mq135_raw, sensorData.mq135_baseline, sensorData.mq135_drop);
  Serial.printf("\n💨 MQ2 - LPG/SMOKE DETECTION\n   Raw: %d | Baseline: %d | Drop: %d\n", sensorData.mq2_raw, sensorData.mq2_baseline, sensorData.mq2_drop);
  Serial.printf("\n☠️  MQ7 - CARBON MONOXIDE (CO)\n   Raw: %d | Baseline: %d | Drop: %d\n", sensorData.mq7_raw, sensorData.mq7_baseline, sensorData.mq7_drop);
  Serial.printf("\nTimestamp: %ld\n", sensorData.timestamp);
}
void calibrateGasSensors() {
  Serial.println("Taking baseline readings in clean air...\n");
  long sum135 = 0, sum2 = 0, sum7 = 0;
  for (int i = 0; i < 50; i++) { sum135 += analogRead(MQ135_PIN); sum2 += analogRead(MQ2_PIN); sum7 += analogRead(MQ7_PIN); delay(50); }
  mq135_baseline = sum135 / 50; mq2_baseline = sum2 / 50; mq7_baseline = sum7 / 50;
  gas_sensors_calibrated = true;
  Serial.printf("MQ135 Baseline: %d\n", mq135_baseline);
  Serial.printf("MQ2 Baseline: %d\n", mq2_baseline);
  Serial.printf("MQ7 Baseline: %d\n", mq7_baseline);
  Serial.println("\n✓ Gas sensor calibration complete!\n");
}