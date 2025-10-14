# Autonomous-Greenhouse 🌿
 This project delivers a Resilient, Low-Cost Greenhouse Climate Control and Monitoring Solution explicitly engineered for the unique operational challenges within the South African agricultural sector, particularly mitigating risks associated with **seasonal fires, load shedding, water scarcity, and climate volatility**. 



##  Core Resilience Features

### ⚡ Load Shedding Resilience (Power Autonomy)
- The **NodeMCU** utilizes **Deep Sleep Mode (GPIO16→RST)**, reducing power draw to micro-amps.
- Enables **continuous monitoring** during power outages.
- Helps mitigate **30%–50% agricultural losses** linked to energy disruptions.

### 💧 Precision Water Management
- Calculates **Vapor Pressure Deficit (VPD)** using **DHT22 sensor data**.
- Enables **targeted irrigation** by quantifying plant water stress.
- Promotes **sustainable water usage** in response to rising irrigation demands.

### 🛑 Local Safety Fail-Safe
- Includes a **hardwired Emergency Button** (via interrupt) allowing **manual override** of automated systems.
- Ensures **local control** (e.g., forcing fan ON via Relay Module) even when network connectivity fails.

### 🔥 Fire and Hazard Detection
- Integrates a **Flame Sensor** for early fire detection, crucial for **Cape Town’s dry-season risk zones**.
- Automatically triggers **buzzer alerts** and **relay-based responses** to safeguard greenhouse environments.

### 🧪 Cost-Effective Gas Safety
- Employs **firmware-assisted rough calibration** for MQ-series gas sensors (e.g., **MQ7 for CO**).
- Provides accurate **gas safety alerts** without the need for costly professional calibration (~R150 per sensor).

### 🔔 Real-Time Audible Alerts
- A **Buzzer Module** emits alerts when temperature, humidity, CO levels, or fire readings exceed **critical thresholds**.
- Ensures **farmers are promptly notified** even during power or network interruptions.


##  System Overview

1. **Sensors** continuously collect temperature, humidity, gas, and fire data.  
2. The **NodeMCU** processes readings, calculates VPD, and checks threshold conditions.  
3. **Alerts** are issued via buzzer, and **actuators** (fans, relays) are triggered as needed.  
4. Data can be transmitted to a **cloud or local dashboard** for visualization and control.  
5. **Deep Sleep Mode** conserves energy between monitoring cycles.


## 👥 Dev Team

Developed by:  
**Ismail Abrahams**, **Alyssa Jordan Krishna**, **Stacey Rosenburg**, **Xavier Jeniker**, **Griffiths Moshoeshoe**, and **Enrique Thomas**


##  Acknowledgment

Special thanks to **Lecturer Ruchen Wyngaard** for continuous guidance, mentorship, and for providing the project brief.

---

> 🌱 *Empowering sustainable agriculture through resilient, low-cost IoT innovation.*

