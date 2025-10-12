# Autonomous-Greenhouse
 This project delivers a Resilient, Low-Cost Greenhouse Climate Control and Monitoring Solution explicitly engineered for the unique operational challenges within the South African agricultural sector, particularly mitigating risks associated with load shedding, water scarcity, and climate volatility. 

✨ Core Resilience Features
The system is optimized for operational continuity and resource efficiency, which are critical necessities for local producers:

Load Shedding Resilience (Power Autonomy): The NodeMCU utilizes Deep Sleep Mode (GPIO16→RST) to run on an extremely low duty cycle, minimizing power consumption to micro-amps. This strategy ensures the system maintains continuous monitoring and survives rotational power cuts, combating the 30%–50% losses reported by farmers due to energy disruption.   

Precision Water Management: It calculates the Vapor Pressure Deficit (VPD) based on DHT22 data. VPD is a superior agronomic metric that quantifies plant water stress, enabling precise, targeted irrigation and resource conservation, which is vital given projected increases in irrigation demand.   

Local Safety Fail-Safe: A hardwired Emergency Button is connected via an interrupt, providing a crucial manual override. This ensures immediate local control (e.g., forcing a fan ON via the Relay Module) even if the network fails, guaranteeing critical protective actions are executed.

Cost-Effective Gas Safety: The system employs firmware-assisted rough calibration for the MQ gas sensors (e.g., MQ7 for CO). This allows the system to accurately provide critical safety alerts without the prohibitive cost of professional calibration (150 or more per device).
