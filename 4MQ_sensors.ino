/*
 * MULTI-SENSOR GREENHOUSE CLIMATE MONITORING SYSTEM
 * For Greenhouse Climate Monitoring - Group 4
 * IFS325 IoT Project - ARC Greenhouse
 * 
 * Sensors: MQ-2 (Smoke/LPG), MQ-5 (Natural Gas), MQ-7 (CO), MQ-135 (Air Quality)
 * 
 * SYNCHRONIZED HEATING CYCLES FOR MQ2 & MQ5 (Both heat at same time)
 * Readings every 10 seconds
 * LED Alert on GPIO 2
 */

// ===== PIN DEFINITIONS =====
const int LED_PIN = 2;            // Built-in LED for alerts (HIGH = danger detected)

// MQ-2 Sensor (Smoke/LPG Detector)
const int MQ2_AOUT_PIN = 32;      // Analog input for MQ-2

// MQ-5 Sensor (Natural Gas/LPG)
const int MQ5_AOUT_PIN = 34;      // Analog input for MQ-5

// MQ-7 Sensor (Carbon Monoxide)
const int MQ7_AOUT_PIN = 35;      // Analog input for MQ-7

// MQ-135 Sensor (Air Quality/Hazardous Gases)
const int MQ135_AOUT_PIN = 33;    // Analog input for MQ-135

// ===== HEATING CYCLE PARAMETERS (MQ2 & MQ5) =====
const int HEATING_TIME = 90;      // Heating duration (seconds) - synchronized
const int MEASUREMENT_TIME = 45;  // Measurement duration (seconds) - synchronized
boolean isHeatingPhase = true;
unsigned long heatingCycleStart = 0;
unsigned long lastHeatingMessage = 0;

// ===== SENSOR BASELINES (Dynamically calibrated) =====
int MQ2_BASELINE = 0;
boolean MQ2_CALIBRATED = false;

int MQ5_BASELINE = 0;
boolean MQ5_CALIBRATED = false;

int MQ7_BASELINE = 0;
boolean MQ7_CALIBRATED = false;

int MQ135_BASELINE = 0;
boolean MQ135_CALIBRATED = false;

// ===== SENSOR THRESHOLDS (% change from baseline) =====
const float MQ2_WARNING_PERCENT = 20.0;   // 20% change = warning
const float MQ2_DANGER_PERCENT = 50.0;    // 50% change = danger

const float MQ5_WARNING_PERCENT = 20.0;   
const float MQ5_DANGER_PERCENT = 50.0;    

const float MQ7_WARNING_PERCENT = 20.0;   
const float MQ7_DANGER_PERCENT = 50.0;    

const float MQ135_WARNING_PERCENT = 15.0; 
const float MQ135_DANGER_PERCENT = 30.0;  

// ===== TIMING =====
unsigned long lastReadingTime = 0;
const long READING_INTERVAL = 10000;  // 10 seconds between readings

// ===== ALERT STATE =====
boolean dangerDetected = false;

void setup() {
  Serial.begin(115200);
  delay(100);
  
  // Configure all pins
  pinMode(LED_PIN, OUTPUT);
  pinMode(MQ2_AOUT_PIN, INPUT);
  pinMode(MQ5_AOUT_PIN, INPUT);
  pinMode(MQ7_AOUT_PIN, INPUT);
  pinMode(MQ135_AOUT_PIN, INPUT);
  
  digitalWrite(LED_PIN, LOW);
  
  // Welcome message
  Serial.println("\n\n");
  Serial.println("╔════════════════════════════════════════════════╗");
  Serial.println("║ GREENHOUSE CLIMATE MONITORING SYSTEM           ║");
  Serial.println("║ Multi-Sensor IoT Solution - Group 4 IFS325     ║");
  Serial.println("║ 4 Gas Sensors with Dynamic Calibration         ║");
  Serial.println("╚════════════════════════════════════════════════╝");
  Serial.println();
  
  Serial.println("SENSOR INFORMATION:");
  Serial.println("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━");
  Serial.println("✓ MQ-2: Smoke/LPG Detection (GPIO32)");
  Serial.println("✓ MQ-5: Natural Gas/LPG Detection (GPIO34)");
  Serial.println("✓ MQ-7: Carbon Monoxide Detection (GPIO35)");
  Serial.println("✓ MQ-135: Air Quality/Hazardous Gases (GPIO33)");
  Serial.println();
  
  Serial.println("HEATING CYCLE SCHEDULE:");
  Serial.println("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━");
  Serial.println("Phase 1: MQ-2 & MQ-5 HEATING (90 seconds)");
  Serial.println("Phase 2: ALL SENSORS MEASURING (45 seconds)");
  Serial.println("Phase 3: CYCLE REPEATS");
  Serial.println();
  
  Serial.println("CALIBRATION WILL START AFTER HEATING...");
  Serial.println("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━");
  
  // Begin heating cycle
  heatingCycleStart = millis();
  isHeatingPhase = true;
  lastHeatingMessage = 0;
  
  Serial.println("MQ-2 & MQ-5: HEATING PHASE (90 seconds)");
  Serial.println("Do NOT introduce any gases during heating!");
  Serial.println();
}

void loop() {
  unsigned long currentTime = millis();
  unsigned long elapsedTime = (currentTime - heatingCycleStart) / 1000; // seconds
  
  // ===== SYNCHRONIZED HEATING CYCLE MANAGEMENT =====
  if (isHeatingPhase) {
    if (elapsedTime < HEATING_TIME) {
      // Still heating - show countdown every 10 seconds
      unsigned long timeSinceLastMessage = (currentTime - lastHeatingMessage) / 1000;
      if (timeSinceLastMessage >= 10 && elapsedTime > 0) {
        Serial.print("[HEATING] ");
        Serial.print(HEATING_TIME - elapsedTime);
        Serial.println("s remaining...");
        lastHeatingMessage = currentTime;
      }
      return; // Exit loop, don't take readings
    } else {
      // Heating complete, switch to measurement
      Serial.println("\n✓ HEATING COMPLETE - SWITCHING TO MEASUREMENT MODE");
      Serial.println("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
      isHeatingPhase = false;
      heatingCycleStart = millis(); // Reset timer for measurement phase
      lastReadingTime = 0; // Force immediate first reading
      return;
    }
  }
  
  // ===== MEASUREMENT PHASE =====
  if (elapsedTime >= MEASUREMENT_TIME) {
    // Measurement phase complete, restart cycle
    Serial.println("\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━");
    Serial.println("✓ MEASUREMENT PHASE COMPLETE - RESTARTING CYCLE");
    Serial.println("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    isHeatingPhase = true;
    heatingCycleStart = millis();
    lastHeatingMessage = 0;
    return;
  }
  
  // ===== READ SENSORS EVERY 10 SECONDS =====
  if (currentTime - lastReadingTime >= READING_INTERVAL) {
    lastReadingTime = currentTime;
    
    dangerDetected = false;
    
    Serial.println("\n┌─ SENSOR READING ─────────────────────────────┐");
    
    // Read MQ-2 (Smoke/LPG)
    readMQ2Sensor();
    
    // Read MQ-5 (Natural Gas)
    readMQ5Sensor();
    
    // Read MQ-7 (Carbon Monoxide)
    readMQ7Sensor();
    
    // Read MQ-135 (Air Quality)
    readMQ135Sensor();
    
    Serial.println("└───────────────────────────────────────────────┘");
    
    // ===== ALERT SYSTEM =====
    if (dangerDetected) {
      Serial.println("\n⚠️  DANGER ALERT DETECTED - LED ACTIVATED");
      digitalWrite(LED_PIN, HIGH);
      delay(500);
      digitalWrite(LED_PIN, LOW);
      delay(500);
    } else {
      Serial.println("\n✓ All sensors within safe parameters");
      digitalWrite(LED_PIN, LOW);
    }
  }
}

// ===== MQ-2 SENSOR READING FUNCTION =====
void readMQ2Sensor() {
  int rawValue = analogRead(MQ2_AOUT_PIN);
  float voltage = rawValue * (3.3 / 4095.0);
  
  if (!MQ2_CALIBRATED) {
    MQ2_BASELINE = rawValue;
    MQ2_CALIBRATED = true;
    Serial.print("MQ-2 (Smoke/LPG): CALIBRATING... Baseline=");
    Serial.print(MQ2_BASELINE);
    Serial.print(" (");
    Serial.print(voltage, 2);
    Serial.println("V) [Calibration done]");
    return;
  }
  
  float percentChange = abs(((float)(rawValue - MQ2_BASELINE) / MQ2_BASELINE) * 100.0);
  
  Serial.print("MQ-2 (Smoke/LPG): Raw=");
  Serial.print(rawValue);
  Serial.print(" V=");
  Serial.print(voltage, 2);
  Serial.print("V Change=");
  Serial.print(percentChange, 1);
  Serial.print("%");
  
  if (percentChange > MQ2_DANGER_PERCENT) {
    Serial.println(" [🚨 CRITICAL DANGER]");
    dangerDetected = true;
  } else if (percentChange > MQ2_WARNING_PERCENT) {
    Serial.println(" [⚠️  WARNING]");
  } else {
    Serial.println(" [✓ SAFE]");
  }
}

// ===== MQ-5 SENSOR READING FUNCTION =====
void readMQ5Sensor() {
  int rawValue = analogRead(MQ5_AOUT_PIN);
  float voltage = rawValue * (3.3 / 4095.0);
  
  if (!MQ5_CALIBRATED) {
    MQ5_BASELINE = rawValue;
    MQ5_CALIBRATED = true;
    Serial.print("MQ-5 (Natural Gas): CALIBRATING... Baseline=");
    Serial.print(MQ5_BASELINE);
    Serial.print(" (");
    Serial.print(voltage, 2);
    Serial.println("V) [Calibration done]");
    return;
  }
  
  float percentChange = abs(((float)(rawValue - MQ5_BASELINE) / MQ5_BASELINE) * 100.0);
  
  Serial.print("MQ-5 (Natural Gas): Raw=");
  Serial.print(rawValue);
  Serial.print(" V=");
  Serial.print(voltage, 2);
  Serial.print("V Change=");
  Serial.print(percentChange, 1);
  Serial.print("%");
  
  if (percentChange > MQ5_DANGER_PERCENT) {
    Serial.println(" [🚨 CRITICAL DANGER]");
    dangerDetected = true;
  } else if (percentChange > MQ5_WARNING_PERCENT) {
    Serial.println(" [⚠️  WARNING]");
  } else {
    Serial.println(" [✓ SAFE]");
  }
}

// ===== MQ-7 SENSOR READING FUNCTION =====
void readMQ7Sensor() {
  int rawValue = analogRead(MQ7_AOUT_PIN);
  float voltage = rawValue * (3.3 / 4095.0);
  
  if (!MQ7_CALIBRATED) {
    MQ7_BASELINE = rawValue;
    MQ7_CALIBRATED = true;
    Serial.print("MQ-7 (Carbon Monoxide): CALIBRATING... Baseline=");
    Serial.print(MQ7_BASELINE);
    Serial.print(" (");
    Serial.print(voltage, 2);
    Serial.println("V) [Calibration done]");
    return;
  }
  
  float percentChange = abs(((float)(rawValue - MQ7_BASELINE) / MQ7_BASELINE) * 100.0);
  
  Serial.print("MQ-7 (CO): Raw=");
  Serial.print(rawValue);
  Serial.print(" V=");
  Serial.print(voltage, 2);
  Serial.print("V Change=");
  Serial.print(percentChange, 1);
  Serial.print("%");
  
  if (percentChange > MQ7_DANGER_PERCENT) {
    Serial.println(" [🚨 CRITICAL DANGER]");
    dangerDetected = true;
  } else if (percentChange > MQ7_WARNING_PERCENT) {
    Serial.println(" [⚠️  WARNING]");
  } else {
    Serial.println(" [✓ SAFE]");
  }
}

// ===== MQ-135 SENSOR READING FUNCTION =====
void readMQ135Sensor() {
  int rawValue = analogRead(MQ135_AOUT_PIN);
  float voltage = rawValue * (3.3 / 4095.0);
  
  // Check if sensor is saturated
  if (rawValue >= 4090) {
    Serial.print("MQ-135 (Air Quality): SATURATED (");
    Serial.print(rawValue);
    Serial.println(") - Check wiring/power!");
    return;
  }
  
  if (!MQ135_CALIBRATED) {
    MQ135_BASELINE = rawValue;
    MQ135_CALIBRATED = true;
    Serial.print("MQ-135 (Air Quality): CALIBRATING... Baseline=");
    Serial.print(MQ135_BASELINE);
    Serial.print(" (");
    Serial.print(voltage, 2);
    Serial.println("V) [Calibration done]");
    return;
  }
  
  float percentChange = abs(((float)(rawValue - MQ135_BASELINE) / MQ135_BASELINE) * 100.0);
  
  Serial.print("MQ-135 (Air Quality): Raw=");
  Serial.print(rawValue);
  Serial.print(" V=");
  Serial.print(voltage, 2);
  Serial.print("V Change=");
  Serial.print(percentChange, 1);
  Serial.print("%");
  
  if (percentChange > MQ135_DANGER_PERCENT) {
    Serial.println(" [🚨 CRITICAL DANGER]");
    dangerDetected = true;
  } else if (percentChange > MQ135_WARNING_PERCENT) {
    Serial.println(" [⚠️  WARNING]");
  } else {
    Serial.println(" [✓ SAFE]");
  }
}