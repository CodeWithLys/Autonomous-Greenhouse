/*
 * MQ-5 LPG/Natural Gas/Coal Gas Sensor Test Code with Heating Cycle
 * For Greenhouse Climate Monitoring - Group 4
 * IFS325 IoT Project
 * ESP32 Version with 1.4V Heating Cycle
 */

// Pin definitions for ESP32
const int MQ5_AOUT_PIN = 34;      // ESP32 ADC1_CH6 (GPIO 34)
const int MQ5_DOUT_PIN = 35;      // ESP32 ADC1_CH7 (GPIO 35)
const int LED_PIN = 2;            // Built-in LED for alerts

// MQ-5 Heating Cycle Parameters
const float HEATING_VOLTAGE = 1.4; // Heating coil voltage (volts)
const int HEATING_TIME = 90;       // Heating duration (seconds)
const int MEASUREMENT_TIME = 45;   // Measurement duration (seconds)

// Calibration and threshold values
const int RL_VALUE = 10;          // Load resistance in kilo ohms
const float RO_CLEAN_AIR = 6.5;   // MQ-5 sensor resistance in clean air
float Ro = 10.0;                  // Will be calibrated

// Threshold values (in ppm)
const int LPG_THRESHOLD = 400;    // LPG danger level
const int CH4_THRESHOLD = 2000;   // Natural gas threshold
const int CO_THRESHOLD = 800;     // Coal gas threshold

// Timing variables
unsigned long previousMillis = 0;
const long interval = 2000;       // Read every 2 seconds

// Heating cycle state variables
boolean isHeatingPhase = true;
unsigned long phaseStartTime = 0;

void setup() {
  Serial.begin(115200);
  delay(100);
  
  pinMode(MQ5_AOUT_PIN, INPUT);
  pinMode(MQ5_DOUT_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);
  
  Serial.println("\n\n=================================");
  Serial.println("MQ-5 Gas Sensor Test");
  Serial.println("LPG/Natural Gas/Coal Gas Detection");
  Serial.println("Greenhouse Climate Monitoring");
  Serial.println("=================================\n");
  
  Serial.println("MQ-5 Heating Cycle Starting...");
  Serial.println("Phase 1: Heating (1.4V for 90 seconds)");
  Serial.println("Phase 2: Measurement (45 seconds)\n");
  
  phaseStartTime = millis();
  isHeatingPhase = true;
}

void loop() {
  unsigned long currentTime = millis();
  unsigned long elapsedTime = (currentTime - phaseStartTime) / 1000; // in seconds
  
  // Heating cycle phase management
  if (isHeatingPhase) {
    if (elapsedTime < HEATING_TIME) {
      // Still in heating phase
      if (elapsedTime % 10 == 0 && elapsedTime > 0) {
        Serial.print("HEATING PHASE: ");
        Serial.print(HEATING_TIME - elapsedTime);
        Serial.println(" seconds remaining...");
      }
      return; // Skip measurements during heating
    } else {
      // Switch to measurement phase
      Serial.println("\n--- Heating Complete! Starting Measurement Phase ---\n");
      isHeatingPhase = false;
      phaseStartTime = millis(); // Reset timer for measurement phase
      return;
    }
  }
  
  // Measurement phase
  if (elapsedTime >= MEASUREMENT_TIME) {
    // Measurement phase complete, restart cycle
    Serial.println("\n--- Measurement Phase Complete! Restarting Cycle ---\n");
    isHeatingPhase = true;
    phaseStartTime = millis();
    return;
  }
  
  if (currentTime - previousMillis >= interval) {
    previousMillis = currentTime;
    
    // Read analog values
    int rawValue = analogRead(MQ5_AOUT_PIN);
    int digitalValue = digitalRead(MQ5_DOUT_PIN);
    
    // Convert to voltage (ESP32: 3.3V, 12-bit ADC = 4095)
    float voltage = rawValue * (3.3 / 4095.0);
    
    float Rs = calculateRs(voltage);
    float ratio = Rs / Ro;
    
    // Calculate gas concentrations (ppm)
    float lpgPPM = calculateLPG_PPM(ratio);
    float ch4PPM = calculateCH4_PPM(ratio);
    float coPPM = calculateCO_PPM(ratio);
    
    // Display readings
    Serial.println("--- Sensor Reading ---");
    Serial.print("Raw ADC Value: ");
    Serial.println(rawValue);
    Serial.print("Voltage: ");
    Serial.print(voltage, 3);
    Serial.println(" V");
    Serial.print("Digital Output: ");
    Serial.println(digitalValue);
    Serial.print("Rs: ");
    Serial.print(Rs, 2);
    Serial.println(" kΩ");
    Serial.print("Rs/Ro Ratio: ");
    Serial.println(ratio, 3);
    Serial.println();
    
    // Gas concentration readings
    Serial.println("--- Gas Concentrations ---");
    Serial.print("LPG Level: ");
    Serial.print(lpgPPM, 1);
    Serial.println(" ppm");
    
    Serial.print("Natural Gas (CH4) Level: ");
    Serial.print(ch4PPM, 1);
    Serial.println(" ppm");
    
    Serial.print("Coal Gas (CO) Level: ");
    Serial.print(coPPM, 1);
    Serial.println(" ppm");
    
    // Check thresholds and alert
    checkGasLevels(lpgPPM, ch4PPM, coPPM);
    
    Serial.println();
  }
}

// Calculate sensor resistance from voltage
float calculateRs(float voltage) {
  if (voltage == 0) return 0;
  float Vc = 3.3 - voltage;
  return (Vc / voltage) * RL_VALUE;
}

// Convert Rs/Ro ratio to LPG concentration in ppm
float calculateLPG_PPM(float ratio) {
  float a = 1200.0;
  float b = -2.75;
  
  if (ratio <= 0) return 0;
  return a * pow(ratio, b);
}

// Convert Rs/Ro ratio to Natural Gas (CH4) concentration in ppm
float calculateCH4_PPM(float ratio) {
  float a = 2100.0;
  float b = -2.95;
  
  if (ratio <= 0) return 0;
  return a * pow(ratio, b);
}

// Convert Rs/Ro ratio to Coal Gas (CO) concentration in ppm
float calculateCO_PPM(float ratio) {
  float a = 3000.0;
  float b = -3.0;
  
  if (ratio <= 0) return 0;
  return a * pow(ratio, b);
}

// Check gas levels and provide alerts
void checkGasLevels(float lpg, float ch4, float co) {
  boolean gasDetected = false;
  
  Serial.println("--- Safety Status ---");
  
  if (lpg >= LPG_THRESHOLD) {
    Serial.println("DANGER: LPG Detected!");
    Serial.println("Action: Evacuate and ventilate greenhouse immediately!");
    gasDetected = true;
    blinkLED(10, 100);
  }
  else if (lpg >= LPG_THRESHOLD * 0.7) {
    Serial.println("WARNING: High LPG levels detected");
    gasDetected = true;
  }
  
  if (ch4 >= CH4_THRESHOLD) {
    Serial.println("DANGER: High Natural Gas (CH4) Detected!");
    Serial.println("Action: Check for gas leaks - ventilate greenhouse!");
    gasDetected = true;
    blinkLED(8, 150);
  }
  else if (ch4 >= CH4_THRESHOLD * 0.7) {
    Serial.println("WARNING: Elevated Natural Gas levels");
    gasDetected = true;
  }
  
  if (co >= CO_THRESHOLD) {
    Serial.println("DANGER: High Coal Gas (CO) Detected!");
    Serial.println("Action: Increase ventilation immediately!");
    gasDetected = true;
    blinkLED(8, 150);
  }
  else if (co >= CO_THRESHOLD * 0.7) {
    Serial.println("WARNING: Elevated Coal Gas levels");
    gasDetected = true;
  }
  
  if (!gasDetected) {
    Serial.println("All gas levels normal - Safe");
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