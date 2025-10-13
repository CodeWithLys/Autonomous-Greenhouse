// Include Libraries
#include "Arduino.h"
#include <Wire.h>
#include <Adafruit_BMP280.h>

// Pin Definitions - Using I2C
#define SDA_PIN 21  // Standard I2C SDA pin for ESP32
#define SCL_PIN 22  // Standard I2C SCL pin for ESP32

// Global variables and defines
Adafruit_BMP280 bmp; // I2C interface

// define vars for testing menu
const int timeout = 10000;       //define timeout of 10 sec
char menuOption = 0;
long time0;

// Greenhouse optimal ranges for pressure
const float PRESSURE_MIN = 980.0;   // hPa - Low pressure (storm warning)
const float PRESSURE_NORMAL = 1013.25; // hPa - Standard atmospheric pressure
const float PRESSURE_MAX = 1040.0;  // hPa - High pressure

// Baseline for altitude compensation
float baselinePressure = PRESSURE_NORMAL;
bool baselineSet = false;

// Setup the essentials for your circuit to work
void setup() 
{
    // Setup Serial which is useful for debugging
    Serial.begin(9600);
    delay(1000); // Give serial time to initialize
    Serial.println("start");
    
    // Initialize I2C communication with custom pins
    Wire.begin(SDA_PIN, SCL_PIN);
    
    Serial.println(F("\n╔════════════════════════════════════════╗"));
    Serial.println(F("║  GREENHOUSE BAROMETRIC PRESSURE SYSTEM ║"));
    Serial.println(F("╚════════════════════════════════════════╝"));
    Serial.println(F("\nSensor: BMP280 (Pressure & Temperature)"));
    Serial.println(F("Interface: I2C"));
    Serial.println(F("  SDA: GPIO 21"));
    Serial.println(F("  SCL: GPIO 22"));
    Serial.println(F("\nNote: GPIO 34 & 35 are INPUT ONLY on ESP32"));
    Serial.println(F("      Using standard I2C pins instead\n"));
    
    // Initialize BMP280 sensor
    if (!bmp.begin(0x76)) // Try address 0x76 first
    { 
        if (!bmp.begin(0x77)) // Try address 0x77 if 0x76 fails
        {
            Serial.println(F("\n❌ Could not find BMP280 sensor!"));
            Serial.println(F("\nTroubleshooting:"));
            Serial.println(F("  1. Check I2C wiring:"));
            Serial.println(F("     - VCC → 3.3V"));
            Serial.println(F("     - GND → GND"));
            Serial.println(F("     - SDA → GPIO 21"));
            Serial.println(F("     - SCL → GPIO 22"));
            Serial.println(F("  2. Check I2C address (0x76 or 0x77)"));
            Serial.println(F("  3. Ensure sensor is powered"));
            Serial.println(F("  4. IMPORTANT: GPIO 34/35 are INPUT ONLY!"));
            Serial.println(F("     Use GPIO 21/22 for I2C instead"));
            while (1) delay(10);
        }
    }
    
    // Configure BMP280 settings for greenhouse monitoring
    bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,     // Operating Mode
                    Adafruit_BMP280::SAMPLING_X2,     // Temp. oversampling
                    Adafruit_BMP280::SAMPLING_X16,    // Pressure oversampling
                    Adafruit_BMP280::FILTER_X16,      // Filtering
                    Adafruit_BMP280::STANDBY_MS_500); // Standby time
    
    Serial.println(F("\n✓ BMP280 sensor initialized successfully!"));
    Serial.println(F("\nCalibrating baseline pressure..."));
    Serial.println(F("Starting continuous monitoring...\n"));
    
    delay(1000);
    
    menuOption = '1'; // Auto-start monitoring
}

// Main logic of your circuit
void loop() 
{
    if(menuOption == '1') 
    {
        // BMP280 Barometric Pressure Monitoring
        testBarometricSensor();
        delay(1000);  // Update every second
    }
    
    // Check if user wants to see menu
    if (Serial.available())
    {
        char c = Serial.read();
        if(c == 'm' || c == 'M')
        {
            menuOption = menu();
        }
    }
}

// Function to assess pressure trend impact on greenhouse
String assessPressure(float pressure)
{
    if(pressure < 980.0)
        return "Very Low - Storm approaching";
    else if(pressure < 1000.0)
        return "Low - Unsettled weather likely";
    else if(pressure < 1020.0)
        return "Normal - Stable conditions";
    else if(pressure < 1030.0)
        return "High - Fair weather expected";
    else
        return "Very High - Settled conditions";
}

// Function to calculate weather trend
String getWeatherTrend(float currentPressure)
{
    static float lastPressure = 0;
    static unsigned long lastTime = 0;
    
    if(lastPressure == 0)
    {
        lastPressure = currentPressure;
        lastTime = millis();
        return "Stabilizing...";
    }
    
    // Calculate pressure change over time
    float pressureChange = currentPressure - lastPressure;
    unsigned long timeChange = millis() - lastTime;
    
    // Update for next reading
    if(timeChange > 60000) // Update baseline every minute
    {
        lastPressure = currentPressure;
        lastTime = millis();
    }
    
    if(abs(pressureChange) < 0.5)
        return "Steady ➡";
    else if(pressureChange > 0.5)
        return "Rising ↗ (Improving weather)";
    else
        return "Falling ↘ (Worsening weather)";
}

// Function to test BMP280 sensor for greenhouse monitoring
void testBarometricSensor()
{
    // Read sensor values
    float temperature = bmp.readTemperature();
    float pressure = bmp.readPressure() / 100.0F; // Convert Pa to hPa
    
    // Set baseline on first reading
    if(!baselineSet)
    {
        baselinePressure = pressure;
        baselineSet = true;
        Serial.println(F("═══════════════════════════════════════"));
        Serial.print(F("✓ Baseline pressure set: "));
        Serial.print(baselinePressure, 2);
        Serial.println(F(" hPa"));
        Serial.println(F("═══════════════════════════════════════\n"));
    }
    
    // Calculate approximate altitude (if needed)
    float altitude = bmp.readAltitude(baselinePressure);
    
    // Get assessments
    String pressureStatus = assessPressure(pressure);
    String weatherTrend = getWeatherTrend(pressure);
    
    // Display readings
    Serial.println(F("╔════════════════════════════════════════╗"));
    Serial.println(F("║   GREENHOUSE BAROMETRIC MONITORING     ║"));
    Serial.println(F("╚════════════════════════════════════════╝"));
    
    // Atmospheric Pressure
    Serial.println(F("\n🌍 ATMOSPHERIC PRESSURE"));
    Serial.print(F("   Current:  "));
    Serial.print(pressure, 2);
    Serial.println(F(" hPa"));
    Serial.print(F("   Baseline: "));
    Serial.print(baselinePressure, 2);
    Serial.println(F(" hPa"));
    Serial.print(F("   Change:   "));
    float pressureChange = pressure - baselinePressure;
    if(pressureChange >= 0) Serial.print(F("+"));
    Serial.print(pressureChange, 2);
    Serial.println(F(" hPa"));
    Serial.print(F("   Status:   "));
    Serial.println(pressureStatus);
    Serial.print(F("   Trend:    "));
    Serial.println(weatherTrend);
    
    // Visual pressure indicator
    Serial.print(F("   Range:    ["));
    int pressBar = map((int)(pressure * 10), 9800, 10400, 0, 20);
    pressBar = constrain(pressBar, 0, 20);
    for(int i = 0; i < 20; i++)
    {
        if(i >= 8 && i <= 14) // Normal range
            Serial.print(pressBar == i ? F("█") : F("░"));
        else
            Serial.print(pressBar == i ? F("█") : F("·"));
    }
    Serial.println(F("]"));
    Serial.println(F("             980      1000-1020     1040 hPa"));
    
    // Temperature (from BMP280)
    Serial.println(F("\n🌡️  TEMPERATURE (BMP280)"));
    Serial.print(F("   Current: "));
    Serial.print(temperature, 1);
    Serial.println(F(" °C"));
    
    // Altitude estimation
    Serial.println(F("\n📏 ALTITUDE ESTIMATE"));
    Serial.print(F("   Height: "));
    Serial.print(altitude, 1);
    Serial.println(F(" meters"));
    
    // Greenhouse implications
    Serial.println(F("\n📊 GREENHOUSE IMPLICATIONS"));
    
    if(pressure < 1000.0)
    {
        Serial.println(F("   ⚠ Low pressure detected:"));
        Serial.println(F("     - Storm/rain likely approaching"));
        Serial.println(F("     - Check greenhouse seals"));
        Serial.println(F("     - Prepare drainage systems"));
        Serial.println(F("     - Monitor humidity levels"));
    }
    else if(pressure > 1025.0)
    {
        Serial.println(F("   ✓ High pressure:"));
        Serial.println(F("     - Stable, dry conditions expected"));
        Serial.println(F("     - Good for pest control treatments"));
        Serial.println(F("     - May need increased irrigation"));
    }
    else
    {
        Serial.println(F("   ✓ Normal pressure conditions"));
        Serial.println(F("     - Continue standard operations"));
    }
    
    // Pressure change warnings
    if(abs(pressureChange) > 5.0)
    {
        Serial.println(F("\n   ⚠⚠ RAPID PRESSURE CHANGE:"));
        if(pressureChange < 0)
            Serial.println(F("     - Weather deteriorating quickly"));
        else
            Serial.println(F("     - Rapid weather improvement"));
    }
    
    Serial.println(F("\n════════════════════════════════════════\n"));
}

// Menu function for selecting the components to be tested
char menu()
{
    Serial.println(F("\n╔════════════════════════════════════════╗"));
    Serial.println(F("║          COMPONENT TEST MENU           ║"));
    Serial.println(F("╚════════════════════════════════════════╝"));
    Serial.println(F("(1) BMP280 - Barometric Pressure"));
    Serial.println(F("(M) Menu - Show this menu anytime\n"));
    Serial.println(F("Note: Monitoring starts automatically"));
    while (!Serial.available());

    // Read data from serial monitor
    while (Serial.available()) 
    {
        char c = Serial.read();
        if (isAlphaNumeric(c)) 
        {   
            if(c == '1') 
            {
                Serial.println(F("\n▶ Restarting Barometric Monitoring..."));
                Serial.println(F("  Reading interval: 1 second\n"));
                time0 = millis();
                return c;
            }
            else
            {
                Serial.println(F("⚠ Invalid input! Please enter '1'"));
                Serial.println(F("  Returning to continuous monitoring...\n"));
                return '1'; // Return to monitoring
            }
        }
    }
    return '1'; // Default to monitoring
}

/*******************************************************
*    Greenhouse Barometric Pressure Monitoring System
*    IFS325 Group Project - ARC Smart Agriculture
*    
*    Sensor: BMP280
*    Communication: I2C (No analog pins required)
*    Purpose: Weather prediction & greenhouse management
*******************************************************/