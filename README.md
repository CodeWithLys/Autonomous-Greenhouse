# EcoView 🌿  
**Transforming Agriculture Through Technology and Sustainability**  

This project — **EcoView**, a **resilient, low-cost greenhouse climate control and monitoring solution** designed specifically for the **South African agricultural context**, addressing challenges such as **seasonal wildfires, load shedding, water scarcity, and climate volatility**.  

---

## 🌍 Core System Overview  

EcoView integrates **two ESP32 microcontrollers** operating in tandem:  
- **Node 1 (Environmental Monitoring):** Gathers temperature, humidity, air quality, and light data using DHT22, BMP280, LDR, and MQ gas sensors.  
- **Node 2 (Fire Safety & Actuation):** Controls a servo motor for vents, fan, and relay system to respond to flame detection events and environmental thresholds.  

Data from both nodes is transmitted via **MQTT** as JSON payloads to an **Oracle APEX cloud database**, where it is visualized and managed in real time through a **Flutter mobile application**.  

---

## ⚙️ Key Resilience & Innovation Features  

### 💧 Soil Management
- Using a collaboration with a **Soil Management Team** for **soil sensors and data**.  
- Enables data-driven AI recommendations.  
- Supports sustainable farming practices.

### 🔥 Fire Detection and Ventilation Safety System
- Developed in response to **Cape Town’s wildfire risks**.  
- Integrates a **Flame IR Sensor** for early flame detection.  
- On detection, the system:  
  - Shuts down the **12V ventilation fan** via relay.  
  - Activates the **SG90 servo motor** to close vent flaps, restricting oxygen flow.  
  - Issues a local alert and cloud notification through Oracle APEX.  
- Designed for both **climate control** and **fire containment**, ensuring dual functionality.

### 🛑 Manual Safety Override
- A **hardware emergency button** allows manual activation or deactivation of fans.  
- Provides direct, local safety control even if Wi-Fi or cloud services are unavailable.

### 🔔 Real-Time Data & Alerts
- MQTT publishes JSON payloads directly to the **Oracle APEX** dashboard.  
- The **Flutter app** displays live sensor data, AI recommendations, and system alerts for farm users.  
- Enables farmers to view conditions and act immediately on abnormalities.

### 🧪 Cost-Effective Gas Monitoring
- Utilizes **MQ2, MQ7, and MQ135** sensors for gas detection (CO, CO₂, LPG, NH₃).  
- Provides reliable safety monitoring through firmware calibration without costly hardware upgrades.  

---

## 🧠 System Architecture  

1. **Node 1 (Sensor Node)**  
   - DHT22, BMP280, LDR, MQ-series sensors.  
   - Powered via 12V → Buck Converter (5V/3.3V).  
   - Publishes environmental data via MQTT to Oracle APEX.  

2. **Node 2 (Actuator Node)**  
   - Flame IR Sensor, Servo Motor, Fan (via Relay), Manual Button.  
   - Powered via 12V → Breadboard Power Module (5V).  
   - Publishes flame alerts via MQTT and executes automated responses.  

3. **Cloud Integration**  
   - Oracle APEX serves as the **real-time database** hosted by the **Data Management Team**.  
   - The **Flutter App** retrieves data from Oracle APEX for real-time visualization and alerts.  

4. **Common Ground**  
   - Both 12V power supplies share a unified ground bus for stability and communication reliability.  

---

## 🧩 System Flow Summary  

1. **Sensors** capture real-time data (temperature, humidity, gases, flame).  
2. **ESP32 controllers** process readings, apply thresholds, and publish JSON payloads via MQTT.  
3. **Oracle APEX** receives and stores data in a cloud-hosted database.  
4. **Flutter mobile app** displays live dashboards and notifications.  
5. **Servo & Fan** automatically respond to fire or environmental triggers.  
6. **Manual override button** provides emergency on-site control.  

---

## 🏢 Stakeholder Engagement  

On **4 November 2025**, the **Agricultural Research Council (ARC)** joined our final presentation, reviewing EcoView’s real-world potential to enhance agricultural safety and efficiency in South Africa.  

---

## 👥 Development Team  

**EcoView Project Team (Group 9):**  
- Ismail Abrahams  
- Alyssa Jordan Krishna  
- Stacey Rosenburg  
- Xavier Jeniker  
- Griffiths Moshoeshoe  
- Enrique Thomas  

**Supervised by:**  
- *Lecturer Ruchen Wyngaard*  

---

> 🌱 *EcoView – Empowering sustainable agriculture through smart, fire-resilient IoT innovation.*  
> *Built by students, inspired by South African resilience.*
