/*
 * GREENHOUSE CLIMATE MONITORING SYSTEM - PROFESSIONAL OUTPUT
 * ARC Greenhouse Project - Group 4 IFS325
 * 
 * Sensors: MQ-2 (Smoke/Flammable Gas), MQ-5 (LPG/Natural Gas), 
 *          MQ-7 (Carbon Monoxide), MQ-135 (Air Quality)
 */

// ===== PIN DEFINITIONS =====
const int LED_PIN = 2;
const int MQ2_AOUT_PIN = 32;      // Smoke/Flammable Gas (with voltage divider)
const int MQ5_AOUT_PIN = 34;      // LPG/Natural Gas
const int MQ7_AOUT_PIN = 35;      // Carbon Monoxide (CO)
const int MQ135_AOUT_PIN = 33;    // Air Quality (CO2 equivalent)

// ===== HEATING CYCLE =====
const int HEATING_TIME = 90;
const int MEASUREMENT_TIME = 45;
boolean isHeatingPhase = true;
unsigned long heatingCycleStart = 0;
unsigned long lastHeatingMessage = 0;

// ===== SENSOR CALIBRATION =====
float MQ2_R0 = 0;
float MQ5_R0 = 0;
float MQ7_R0 = 0;
float MQ135_R0 = 0;

boolean MQ2_CALIBRATED = false;
boolean MQ5_CALIBRATED = false;
boolean MQ7_CALIBRATED = false;
boolean MQ135_CALIBRATED = false;

// ===== ALERT THRESHOLDS (PPM or equivalent) =====
// MQ-2: Smoke/Flammable Gas
const float MQ2_WARNING_PPM = 300;
const float MQ2_DANGER_PPM = 500;

// MQ-5: LPG/Natural Gas
const float MQ5_WARNING_PPM = 600;
const float MQ5_DANGER_PPM = 1000;

// MQ-7: Carbon Monoxide
const float MQ7_WARNING_PPM = 35;
const float MQ7_DANGER_PPM = 100;

// MQ-135: Air Quality (CO2 equivalent ppm)
const float MQ135_WARNING_PPM = 1000;
const float MQ135_DANGER_PPM = 1500;

// ===== TIMING =====
unsigned long lastReadingTime = 0;
const long READING_INTERVAL = 10000;

// ===== ALERT TRACKING =====
boolean warningDetected = false;
boolean dangerDetected = false;
String alertSensors = "";

void setup() {
  Serial.begin(115200);
  delay(100);
  
  pinMode(LED_PIN, OUTPUT);
  pinMode(MQ2_AOUT_PIN, INPUT);
  pinMode(MQ5_AOUT_PIN, INPUT);
  pinMode(MQ7_AOUT_PIN, INPUT);
  pinMode(MQ135_AOUT_PIN, INPUT);
  
  digitalWrite(LED_PIN, LOW);
  
  // Header
  Serial.println("\n\n");
  Serial.println("╔═══════════════════════════════════════════════════════════╗");
  Serial.println("║         ARC GREENHOUSE CLIMATE MONITORING SYSTEM           ║");
  Serial.println("║              IoT Environmental Control System              ║");
  Serial.println("╚═══════════════════════════════════════════════════════════╝\n");
  
  Serial.println("SYSTEM CONFIGURATION:");
  Serial.println("───────────────────────────────────────────────────────────");
  Serial.println("Sensor 1: MQ-2   → Smoke/Flammable Gas Detection");
  Serial.println("Sensor 2: MQ-5   → LPG/Natural Gas Detection");
  Serial.println("Sensor 3: MQ-7   → Carbon Monoxide (CO) Detection");
  Serial.println("Sensor 4: MQ-135 → Air Quality/CO2 Equivalent Monitoring");
  Serial.println("Alert LED: GPIO 2 (Built-in)\n");
  
  Serial.println("OPERATIONAL CYCLE:");
  Serial.println("───────────────────────────────────────────────────────────");
  Serial.println("Phase 1: Sensor Heating        | Duration: 90 seconds");
  Serial.println("Phase 2: Sensor Calibration    | Automatic (first reading)");
  Serial.println("Phase 3: Active Monitoring     | Readings every 10 seconds");
  Serial.println("Phase 4: Cycle Restart         | After 45 seconds of Phase 3\n");
  
  Serial.println("INITIATING STARTUP SEQUENCE...");
  Serial.println("═══════════════════════════════════════════════════════════\n");
  
  heatingCycleStart = millis();
  isHeatingPhase = true;
}

void loop() {
  unsigned long currentTime = millis();
  unsigned long elapsedTime = (currentTime - heatingCycleStart) / 1000;
  
  // ===== HEATING PHASE =====
  if (isHeatingPhase) {
    if (elapsedTime < HEATING_TIME) {
      if ((currentTime - lastHeatingMessage) >= 10000) {
        Serial.print("[HEATING] MQ-2 & MQ-5 warming up... ");
        Serial.print(HEATING_TIME - elapsedTime);
        Serial.println("s remaining");
        lastHeatingMessage = currentTime;
      }
      return;
    } else {
      Serial.println("\n✓ Sensors warmed up - Beginning calibration & measurement\n");
      isHeatingPhase = false;
      heatingCycleStart = millis();
      lastReadingTime = 0;
      return;
    }
  }
  
  // ===== MEASUREMENT PHASE =====
  if (elapsedTime >= MEASUREMENT_TIME) {
    Serial.println("\n───────────────────────────────────────────────────────────");
    Serial.println("Measurement cycle complete. Restarting heating phase...\n");
    isHeatingPhase = true;
    heatingCycleStart = millis();
    return;
  }
  
  // ===== READ SENSORS EVERY 10 SECONDS =====
  if (currentTime - lastReadingTime >= READING_INTERVAL) {
    lastReadingTime = currentTime;
    
    warningDetected = false;
    dangerDetected = false;
    alertSensors = "";
    
    // Print timestamp
    Serial.print("[");
    Serial.print(elapsedTime);
    Serial.println("s] ENVIRONMENTAL READING");
    Serial.println("───────────────────────────────────────────────────────────");
    
    // Read all sensors
    readMQ2();
    readMQ5();
    readMQ7();
    readMQ135();
    
    Serial.println("───────────────────────────────────────────────────────────");
    
    // Print system summary
    printSystemSummary();
    
    // Alert system
    if (dangerDetected) {
      digitalWrite(LED_PIN, HIGH);
      delay(300);
      digitalWrite(LED_PIN, LOW);
      delay(300);
    } else if (warningDetected) {
      digitalWrite(LED_PIN, HIGH);
      delay(100);
      digitalWrite(LED_PIN, LOW);
    } else {
      digitalWrite(LED_PIN, LOW);
    }
    
    Serial.println();
  }
}

// ===== MQ-2 SENSOR (Smoke/Flammable Gas) =====
void readMQ2() {
  int rawValue = analogRead(MQ2_AOUT_PIN);
  float voltage = rawValue * (3.3 / 4095.0);
  float Rs = calculateRs(voltage, 3.3);
  
  if (!MQ2_CALIBRATED) {
    MQ2_R0 = Rs / 9.83;  // MQ-2 calibration factor for clean air
    MQ2_CALIBRATED = true;
    Serial.println("MQ-2 (Smoke/Flammable Gas): CALIBRATING...");
    Serial.print("  Baseline: ");
    Serial.print(MQ2_R0, 2);
    Serial.println(" kΩ [READY]");
    return;
  }
  
  float ratio = Rs / MQ2_R0;
  float ppm = 116.6 * pow(ratio, -2.769);  // MQ-2 characteristic curve
  ppm = max(0.0f, ppm);  // Prevent negative values
  
  Serial.print("MQ-2 (Smoke/Flammable Gas): ");
  Serial.print(ppm, 1);
  Serial.print(" ppm");
  
  if (ppm > MQ2_DANGER_PPM) {
    Serial.println(" [DANGER - TOXIC LEVELS]");
    dangerDetected = true;
    alertSensors += "Smoke/Gas ";
  } else if (ppm > MQ2_WARNING_PPM) {
    Serial.println(" [WARNING - Elevated]");
    warningDetected = true;
    alertSensors += "Smoke/Gas ";
  } else {
    Serial.println(" [SAFE]");
  }
}

// ===== MQ-5 SENSOR (LPG/Natural Gas) =====
void readMQ5() {
  int rawValue = analogRead(MQ5_AOUT_PIN);
  float voltage = rawValue * (3.3 / 4095.0);
  float Rs = calculateRs(voltage, 3.3);
  
  if (!MQ5_CALIBRATED) {
    MQ5_R0 = Rs / 6.5;  // MQ-5 calibration factor for clean air
    MQ5_CALIBRATED = true;
    Serial.println("MQ-5 (LPG/Natural Gas): CALIBRATING...");
    Serial.print("  Baseline: ");
    Serial.print(MQ5_R0, 2);
    Serial.println(" kΩ [READY]");
    return;
  }
  
  float ratio = Rs / MQ5_R0;
  float ppm = 116.6 * pow(ratio, -2.789);  // MQ-5 characteristic curve
  ppm = max(0.0f, ppm);
  
  Serial.print("MQ-5 (LPG/Natural Gas): ");
  Serial.print(ppm, 1);
  Serial.print(" ppm");
  
  if (ppm > MQ5_DANGER_PPM) {
    Serial.println(" [DANGER - GAS LEAK]");
    dangerDetected = true;
    alertSensors += "Gas-Leak ";
  } else if (ppm > MQ5_WARNING_PPM) {
    Serial.println(" [WARNING - Elevated]");
    warningDetected = true;
    alertSensors += "Gas-Level ";
  } else {
    Serial.println(" [SAFE]");
  }
}

// ===== MQ-7 SENSOR (Carbon Monoxide) =====
void readMQ7() {
  int rawValue = analogRead(MQ7_AOUT_PIN);
  float voltage = rawValue * (3.3 / 4095.0);
  float Rs = calculateRs(voltage, 3.3);
  
  if (!MQ7_CALIBRATED) {
    MQ7_R0 = Rs / 9.83;  // MQ-7 calibration factor for clean air
    MQ7_CALIBRATED = true;
    Serial.println("MQ-7 (Carbon Monoxide): CALIBRATING...");
    Serial.print("  Baseline: ");
    Serial.print(MQ7_R0, 2);
    Serial.println(" kΩ [READY]");
    return;
  }
  
  float ratio = Rs / MQ7_R0;
  float ppm = 100.0 * pow(ratio, -1.53);  // MQ-7 characteristic curve
  ppm = max(0.0f, ppm);
  
  Serial.print("MQ-7 (Carbon Monoxide): ");
  Serial.print(ppm, 1);
  Serial.print(" ppm");
  
  if (ppm > MQ7_DANGER_PPM) {
    Serial.println(" [DANGER - TOXIC CO LEVELS]");
    dangerDetected = true;
    alertSensors += "CO-Toxic ";
  } else if (ppm > MQ7_WARNING_PPM) {
    Serial.println(" [WARNING - Elevated CO]");
    warningDetected = true;
    alertSensors += "CO-Level ";
  } else {
    Serial.println(" [SAFE]");
  }
}

// ===== MQ-135 SENSOR (Air Quality) =====
void readMQ135() {
  int rawValue = analogRead(MQ135_AOUT_PIN);
  float voltage = rawValue * (3.3 / 4095.0);
  float Rs = calculateRs(voltage, 3.3);
  
  if (!MQ135_CALIBRATED) {
    MQ135_R0 = Rs / 10.0;  // MQ-135 calibration factor
    MQ135_CALIBRATED = true;
    Serial.println("MQ-135 (Air Quality): CALIBRATING...");
    Serial.print("  Baseline: ");
    Serial.print(MQ135_R0, 2);
    Serial.println(" kΩ [READY]");
    return;
  }
  
  float ratio = Rs / MQ135_R0;
  float ppm = 116.6 * pow(ratio, -2.56);  // MQ-135 characteristic curve
  ppm = max(0.0f, ppm);
  
  Serial.print("MQ-135 (Air Quality/CO2 eq.): ");
  Serial.print(ppm, 0);
  Serial.print(" ppm");
  
  if (ppm > MQ135_DANGER_PPM) {
    Serial.println(" [DANGER - Poor Air Quality]");
    dangerDetected = true;
    alertSensors += "AirQuality ";
  } else if (ppm > MQ135_WARNING_PPM) {
    Serial.println(" [WARNING - Monitor Air]");
    warningDetected = true;
    alertSensors += "AirQuality ";
  } else {
    Serial.println(" [SAFE]");
  }
}

// ===== SYSTEM SUMMARY =====
void printSystemSummary() {
  Serial.print("SYSTEM STATUS: ");
  
  if (dangerDetected) {
    Serial.print("[CRITICAL ALERT] ");
    Serial.println(alertSensors);
    Serial.println("ACTION: Activate emergency ventilation. Check equipment.");
  } else if (warningDetected) {
    Serial.print("[WARNING] ");
    Serial.println(alertSensors);
    Serial.println("ACTION: Monitor levels. Increase ventilation if needed.");
  } else {
    Serial.println("[NORMAL OPERATION] All parameters within safe limits");
  }
}

// ===== HELPER FUNCTION: Calculate Sensor Resistance =====
float calculateRs(float voltage, float refVoltage) {
  if (voltage == 0) return 0;
  float Vc = refVoltage - voltage;
  const int RL_VALUE = 10;  // Load resistance (kΩ)
  return (Vc / voltage) * RL_VALUE;
}
