
/*
 * MQ-7 Carbon Monoxide Sensor Test Code
 * For Greenhouse Climate Monitoring - Group 4
 * IFS325 IoT Project
 * 
 * This code tests the MQ-7 sensor and reads CO levels
 * Connect MQ-7 analog output to A0
 * VCC to 5V, GND to GND
 */

// Pin definitions
const int MQ7_PIN = A0;           // Analog pin for MQ-7 sensor
const int LED_PIN = 13;           // Built-in LED for alerts

// Calibration and threshold values
const int RL_VALUE = 10;          // Load resistance in kilo ohms
const float RO_CLEAN_AIR = 27.5;  // Sensor resistance in clean air
float Ro = 10.0;                  // Will be calibrated

// Threshold values for CO (in ppm)
const int CO_THRESHOLD_LOW = 50;   // Warning level
const int CO_THRESHOLD_HIGH = 200; // Dangerous level

// Timing variables
unsigned long previousMillis = 0;
const long interval = 2000;        // Read every 2 seconds

void setup() {
  Serial.begin(9600);
  pinMode(LED_PIN, OUTPUT);
  pinMode(MQ7_PIN, INPUT);
  
  Serial.println("=================================");
  Serial.println("MQ-7 CO Sensor Test - Greenhouse Monitoring");
  Serial.println("=================================");
  Serial.println();
  
  // Warm-up period
  Serial.println("Warming up sensor (60 seconds)...");
  Serial.println("Please ensure sensor is in clean air for calibration");
  
  for(int i = 60; i > 0; i--) {
    Serial.print("Time remaining: ");
    Serial.print(i);
    Serial.println(" seconds");
    delay(1000);
  }
  
  // Calibrate sensor
  Serial.println("\nCalibrating sensor...");
  Ro = calibrateSensor();
  Serial.print("Calibration complete. Ro = ");
  Serial.println(Ro);
  Serial.println("\nStarting measurements...\n");
}

void loop() {
  unsigned long currentMillis = millis();
  
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    
    // Read sensor
    int rawValue = analogRead(MQ7_PIN);
    float voltage = rawValue * (5.0 / 1023.0);
    float Rs = calculateRs(voltage);
    float ratio = Rs / Ro;
    float coPPM = calculateCO_PPM(ratio);
    
    // Display readings
    Serial.println("--- Sensor Reading ---");
    Serial.print("Raw ADC Value: ");
    Serial.println(rawValue);
    Serial.print("Voltage: ");
    Serial.print(voltage, 3);
    Serial.println(" V");
    Serial.print("Rs: ");
    Serial.print(Rs, 2);
    Serial.println(" kΩ");
    Serial.print("Rs/Ro Ratio: ");
    Serial.println(ratio, 3);
    Serial.print("CO Level: ");
    Serial.print(coPPM, 1);
    Serial.println(" ppm");
    
    // Check thresholds and alert
    checkCOLevels(coPPM);
    
    Serial.println();
  }
}

// Calibrate sensor in clean air
float calibrateSensor() {
  float sum = 0;
  int samples = 50;
  
  for(int i = 0; i < samples; i++) {
    int rawValue = analogRead(MQ7_PIN);
    float voltage = rawValue * (5.0 / 1023.0);
    float Rs = calculateRs(voltage);
    sum += Rs;
    delay(100);
  }
  
  float avgRs = sum / samples;
  return avgRs / RO_CLEAN_AIR;
}

// Calculate sensor resistance from voltage
float calculateRs(float voltage) {
  if (voltage == 0) return 0;
  // Rs = (Vc/VRL - 1) * RL
  return ((5.0 * RL_VALUE) / voltage) - RL_VALUE;
}

// Convert Rs/Ro ratio to CO concentration in ppm
float calculateCO_PPM(float ratio) {
  // MQ-7 characteristic curve approximation
  // PPM = a * (Rs/Ro)^b
  // For CO: approximate values
  float a = 100.0;
  float b = -1.53;
  
  if (ratio <= 0) return 0;
  return a * pow(ratio, b);
}

// Check CO levels and provide alerts
void checkCOLevels(float ppm) {
  if (ppm >= CO_THRESHOLD_HIGH) {
    Serial.println("⚠️  ALERT: DANGEROUS CO LEVEL!");
    Serial.println("Action: Increase ventilation immediately!");
    blinkLED(5, 100);  // Fast blinking
  } 
  else if (ppm >= CO_THRESHOLD_LOW) {
    Serial.println("⚠️  WARNING: Elevated CO detected");
    Serial.println("Action: Monitor and increase ventilation");
    blinkLED(3, 300);  // Moderate blinking
  } 
  else {
    Serial.println("✓ CO levels normal");
    digitalWrite(LED_PIN, LOW);
  }
}

// Blink LED for alerts
void blinkLED(int times, int delayMs) {
  for(int i = 0; i < times; i++) {
    digitalWrite(LED_PIN, HIGH);
    delay(delayMs);
    digitalWrite(LED_PIN, LOW);
    delay(delayMs);
  }
}

// Additional function: Get sensor status
String getSensorStatus(float ppm) {
  if (ppm >= CO_THRESHOLD_HIGH) return "DANGER";
  else if (ppm >= CO_THRESHOLD_LOW) return "WARNING";
  else return "NORMAL";
}