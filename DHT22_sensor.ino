// Include Libraries
#include "Arduino.h"
#include "DHT.h"

// Pin Definitions
#define DHT_PIN_DATA	16
#define DHTTYPE DHT22     // DHT22 (AM2302) sensor

// Global variables and defines

// object initialization
DHT dht(DHT_PIN_DATA, DHTTYPE);

// define vars for testing menu
const int timeout = 10000;       //define timeout of 10 sec
char menuOption = 0;
long time0;

// Greenhouse optimal ranges
const float TEMP_MIN_OPTIMAL = 18.0;
const float TEMP_MAX_OPTIMAL = 27.0;
const float TEMP_MIN_CRITICAL = 10.0;
const float TEMP_MAX_CRITICAL = 35.0;
const float HUMIDITY_MIN_OPTIMAL = 50.0;
const float HUMIDITY_MAX_OPTIMAL = 70.0;

// Setup the essentials for your circuit to work. It runs first every time your circuit is powered with electricity.
void setup() 
{
    // Setup Serial which is useful for debugging
    // Use the Serial Monitor to view printed messages
    Serial.begin(9600);
    while (!Serial) ; // wait for serial port to connect. Needed for native USB
    Serial.println("start");
    
    // Initialize DHT sensor
    dht.begin();
    Serial.println(F("\n╔════════════════════════════════════════╗"));
    Serial.println(F("║  GREENHOUSE CLIMATE MONITORING SYSTEM  ║"));
    Serial.println(F("╚════════════════════════════════════════╝"));
    Serial.println(F("\nSensor: DHT22 (Temperature & Humidity)"));
    Serial.println(F("Pin: GPIO 16"));
    Serial.println(F("\nOptimal Greenhouse Conditions:"));
    Serial.println(F("  Temperature: 18-27°C"));
    Serial.println(F("  Humidity: 50-70%"));
    Serial.println(F("\nInitializing sensor...\n"));
    
    delay(2000);  // Give sensor time to stabilize
    
    menuOption = menu();
}

// Main logic of your circuit. It defines the interaction between the components you selected. After setup, it runs over and over again, in an eternal loop.
void loop() 
{
    if(menuOption == '1') 
    {
        // DHT22 Greenhouse Climate Monitoring
        testGreenhouseClimate();
        delay(2000);  // DHT22 needs 2 second delay between readings
    }
    
    if (millis() - time0 > timeout)
    {
        menuOption = menu();
    }
}

// Function to assess greenhouse temperature status
String assessTemperature(float temp)
{
    if(temp < TEMP_MIN_CRITICAL)
        return "CRITICAL LOW - Risk of frost damage!";
    else if(temp < TEMP_MIN_OPTIMAL)
        return "Below Optimal - Consider heating";
    else if(temp <= TEMP_MAX_OPTIMAL)
        return "OPTIMAL - Good for crop growth";
    else if(temp <= TEMP_MAX_CRITICAL)
        return "Above Optimal - Consider ventilation";
    else
        return "CRITICAL HIGH - Heat stress risk!";
}

// Function to assess greenhouse humidity status
String assessHumidity(float humidity)
{
    if(humidity < 30.0)
        return "Very Low - Risk of plant stress";
    else if(humidity < HUMIDITY_MIN_OPTIMAL)
        return "Below Optimal - Consider misting";
    else if(humidity <= HUMIDITY_MAX_OPTIMAL)
        return "OPTIMAL - Good for crop health";
    else if(humidity <= 80.0)
        return "Above Optimal - Risk of fungal growth";
    else
        return "Very High - High disease risk!";
}

// Function to test DHT sensor for greenhouse monitoring
void testGreenhouseClimate()
{
    // Reading humidity in %
    float humidity = dht.readHumidity();
    // Read temperature in Celsius
    float tempC = dht.readTemperature();
    
    // Check if any reads failed
    if (isnan(humidity) || isnan(tempC)) 
    {
        Serial.println(F("╔════════════════════════════════════╗"));
        Serial.println(F("║        SENSOR ERROR                ║"));
        Serial.println(F("╚════════════════════════════════════╝"));
        Serial.println(F("❌ Failed to read from DHT22 sensor!"));
        Serial.println(F("\nTroubleshooting:"));
        Serial.println(F("  1. Check wiring connections"));
        Serial.println(F("     - VCC → 3.3V or 5V"));
        Serial.println(F("     - GND → GND"));
        Serial.println(F("     - DATA → GPIO Pin 16"));
        Serial.println(F("  2. Ensure sensor is powered"));
        Serial.println(F("  3. Wait 2 seconds between readings"));
        Serial.println(F("════════════════════════════════════════\n"));
        return;
    }
    
    // Calculate Vapor Pressure Deficit (VPD) - important for greenhouse management
    float es = 0.6108 * exp((17.27 * tempC) / (tempC + 237.3)); // Saturation vapor pressure
    float ea = (humidity / 100.0) * es; // Actual vapor pressure
    float vpd = es - ea; // VPD in kPa
    
    // Get status assessments
    String tempStatus = assessTemperature(tempC);
    String humidStatus = assessHumidity(humidity);
    
    // Display readings with greenhouse-focused formatting
    Serial.println(F("╔════════════════════════════════════════╗"));
    Serial.println(F("║     GREENHOUSE CLIMATE READING         ║"));
    Serial.println(F("╚════════════════════════════════════════╝"));
    
    // Temperature reading
    Serial.println(F("\n🌡️  TEMPERATURE"));
    Serial.print(F("   Current: "));
    Serial.print(tempC, 1);
    Serial.println(F(" °C"));
    Serial.print(F("   Status:  "));
    Serial.println(tempStatus);
    
    // Visual temperature indicator
    Serial.print(F("   Range:   ["));
    int tempBar = map((int)(tempC * 10), 100, 350, 0, 20);
    tempBar = constrain(tempBar, 0, 20);
    for(int i = 0; i < 20; i++)
    {
        if(i >= 8 && i <= 14) // Optimal range (18-27°C mapped)
            Serial.print(tempBar == i ? F("█") : F("░"));
        else
            Serial.print(tempBar == i ? F("█") : F("·"));
    }
    Serial.println(F("]"));
    Serial.println(F("            10°C      18-27°C      35°C"));
    
    // Humidity reading
    Serial.println(F("\n💧 HUMIDITY"));
    Serial.print(F("   Current: "));
    Serial.print(humidity, 1);
    Serial.println(F(" %"));
    Serial.print(F("   Status:  "));
    Serial.println(humidStatus);
    
    // Visual humidity indicator
    Serial.print(F("   Range:   ["));
    int humBar = map((int)humidity, 0, 100, 0, 20);
    humBar = constrain(humBar, 0, 20);
    for(int i = 0; i < 20; i++)
    {
        if(i >= 10 && i <= 14) // Optimal range (50-70% mapped)
            Serial.print(humBar == i ? F("█") : F("░"));
        else
            Serial.print(humBar == i ? F("█") : F("·"));
    }
    Serial.println(F("]"));
    Serial.println(F("            0%       50-70%      100%"));
    
    // Vapor Pressure Deficit (VPD)
    Serial.println(F("\n💨 VAPOR PRESSURE DEFICIT (VPD)"));
    Serial.print(F("   VPD: "));
    Serial.print(vpd, 2);
    Serial.println(F(" kPa"));
    Serial.print(F("   Status:  "));
    if(vpd < 0.4)
        Serial.println(F("Too Low - Poor transpiration"));
    else if(vpd < 0.8)
        Serial.println(F("Low - Good for propagation"));
    else if(vpd < 1.2)
        Serial.println(F("OPTIMAL - Ideal for growth"));
    else if(vpd < 1.6)
        Serial.println(F("High - Good for fruiting"));
    else
        Serial.println(F("Too High - Plant stress risk"));
    
    // Overall greenhouse status
    Serial.println(F("\n📊 OVERALL GREENHOUSE STATUS"));
    bool tempOK = (tempC >= TEMP_MIN_OPTIMAL && tempC <= TEMP_MAX_OPTIMAL);
    bool humOK = (humidity >= HUMIDITY_MIN_OPTIMAL && humidity <= HUMIDITY_MAX_OPTIMAL);
    
    if(tempOK && humOK)
    {
        Serial.println(F("   ✓ All parameters within optimal range"));
    }
    else
    {
        Serial.println(F("   ⚠ Action required:"));
        if(!tempOK)
        {
            if(tempC < TEMP_MIN_OPTIMAL)
                Serial.println(F("     - Increase heating"));
            else
                Serial.println(F("     - Increase ventilation/cooling"));
        }
        if(!humOK)
        {
            if(humidity < HUMIDITY_MIN_OPTIMAL)
                Serial.println(F("     - Activate misting system"));
            else
                Serial.println(F("     - Improve air circulation"));
        }
    }
    
    Serial.println(F("\n════════════════════════════════════════\n"));
}

// Menu function for selecting the components to be tested
// Follow serial monitor for instructions
char menu()
{
    Serial.println(F("\n╔════════════════════════════════════════╗"));
    Serial.println(F("║          COMPONENT TEST MENU           ║"));
    Serial.println(F("╚════════════════════════════════════════╝"));
    Serial.println(F("(1) DHT22 - Temperature & Humidity"));
    Serial.println(F("(X) Reset - Return to menu\n"));
    while (!Serial.available());

    // Read data from serial monitor if received
    while (Serial.available()) 
    {
        char c = Serial.read();
        if (isAlphaNumeric(c)) 
        {   
            if(c == '1') 
            {
                Serial.println(F("\n▶ Starting Greenhouse Climate Monitoring..."));
                Serial.println(F("  Reading interval: 2 seconds\n"));
            }
            else
            {
                Serial.println(F("⚠ Invalid input! Please enter '1'"));
                return 0;
            }
            time0 = millis();
            return c;
        }
    }
}

/*******************************************************
*    Greenhouse Climate Monitoring System
*    IFS325 Group Project - ARC Smart Agriculture
*    
*    Based on Circuito.io template
*    Modified for greenhouse environment monitoring
*******************************************************/