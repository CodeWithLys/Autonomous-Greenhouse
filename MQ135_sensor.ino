// Include Libraries
#include "Arduino.h"

// Pin Definitions
#define MQ135_3V3_PIN_AOUT	A0  // Changed to analog pin

// Global variables and defines
const int timeout = 10000;       //define timeout of 10 sec
char menuOption = 0;
long time0;

// MQ135 sensor variables
int sensorValue = 0;
int baselineValue = 0;           // Baseline reading for clean air
float voltage = 0;
const int numReadings = 10;      // Number of readings to average
bool baselineSet = false;

// Setup the essentials for your circuit to work. It runs first every time your circuit is powered with electricity.
void setup() 
{
    // Setup Serial which is useful for debugging
    // Use the Serial Monitor to view printed messages
    Serial.begin(9600);
    while (!Serial) ; // wait for serial port to connect. Needed for native USB
    Serial.println("start");
    
    // Configure analog pin
    pinMode(MQ135_3V3_PIN_AOUT, INPUT);
    
    Serial.println(F("\n=== MQ135 Sensor Warmup ==="));
    Serial.println(F("Please wait 2-3 minutes for sensor to warm up..."));
    Serial.println(F("The sensor needs to heat up for accurate readings."));
    Serial.println(F("After warmup, the sensor will calibrate to clean air.\n"));
    
    menuOption = menu();
}

// Main logic of your circuit. It defines the interaction between the components you selected. After setup, it runs over and over again, in an eternal loop.
void loop() 
{
    if(menuOption == '1')
    {
        // Read MQ135 Sensor
        testMQ135Sensor();
        delay(1000);  // Wait 1 second between readings
    }
    
    if (millis() - time0 > timeout)
    {
        menuOption = menu();
    }
}

// Function to test MQ135 Gas Sensor
void testMQ135Sensor()
{
    // Take multiple readings and average them
    long sum = 0;
    for(int i = 0; i < numReadings; i++)
    {
        sum += analogRead(MQ135_3V3_PIN_AOUT);
        delay(10);
    }
    sensorValue = sum / numReadings;
    
    // Set baseline on first reading (after warmup)
    if(!baselineSet)
    {
        baselineValue = sensorValue;
        baselineSet = true;
        Serial.println(F("=== Baseline Calibrated ==="));
        Serial.print(F("Clean Air Baseline: "));
        Serial.println(baselineValue);
        Serial.println(F("Now monitoring for changes...\n"));
    }
    
    // Convert to voltage (assuming 3.3V reference)
    voltage = sensorValue * (3.3 / 1023.0);
    
    // Calculate change from baseline
    int change = sensorValue - baselineValue;
    float changePercent = (change / (float)baselineValue) * 100.0;
    
    // Display readings
    Serial.println(F("=== MQ135 Gas Sensor Reading ==="));
    Serial.print(F("Analog Value: "));
    Serial.print(sensorValue);
    Serial.print(F("  (Baseline: "));
    Serial.print(baselineValue);
    Serial.println(F(")"));
    
    Serial.print(F("Change: "));
    if(change >= 0) Serial.print(F("+"));
    Serial.print(change);
    Serial.print(F(" ("));
    if(changePercent >= 0) Serial.print(F("+"));
    Serial.print(changePercent, 1);
    Serial.println(F("%)"));
    
    Serial.print(F("Voltage: "));
    Serial.print(voltage, 3);
    Serial.println(F(" V"));
    
    // Provide sensitive interpretation based on change from baseline
    Serial.print(F("Air Quality: "));
    if(change < 5)
    {
        Serial.println(F("Clean Air ✓"));
    }
    else if(change < 15)
    {
        Serial.println(F("⚠ SLIGHT CHANGE DETECTED - Minor pollutants"));
    }
    else if(change < 30)
    {
        Serial.println(F("⚠⚠ MODERATE CHANGE - Noticeable pollutants"));
    }
    else if(change < 50)
    {
        Serial.println(F("⚠⚠⚠ SIGNIFICANT CHANGE - High pollutant levels"));
    }
    else
    {
        Serial.println(F("⚠⚠⚠⚠ SEVERE CHANGE - Very high pollutant levels!"));
    }
    
    // Visual bar graph
    Serial.print(F("Level: ["));
    int barLength = map(change, 0, 100, 0, 20);
    barLength = constrain(barLength, 0, 20);
    for(int i = 0; i < 20; i++)
    {
        if(i < barLength) Serial.print(F("█"));
        else Serial.print(F("·"));
    }
    Serial.println(F("]"));
    
    Serial.println(F("================================\n"));
}

// Menu function for selecting the components to be tested
// Follow serial monitor for instructions
char menu()
{
    Serial.println(F("\nWhich component would you like to test?"));
    Serial.println(F("(1) Hazardous Gas Sensor - MQ-135"));
    Serial.println(F("(menu) send anything else or press on board reset button\n"));
    while (!Serial.available());

    // Read data from serial monitor if received
    while (Serial.available()) 
    {
        char c = Serial.read();
        if (isAlphaNumeric(c)) 
        {   
            if(c == '1') 
            {
                Serial.println(F("Now Testing Hazardous Gas Sensor - MQ-135"));
                Serial.println(F("Readings will update every second...\n"));
            }
            else
            {
                Serial.println(F("illegal input!"));
                return 0;
            }
            time0 = millis();
            return c;
        }
    }
}

/*******************************************************
*    Circuito.io is an automatic generator of schematics and code for off
*    the shelf hardware combinations.
*    Copyright (C) 2016 Roboplan Technologies Ltd.
*    Licensed under GNU General Public License v3.0
*******************************************************/