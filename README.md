🛩️ #Drone Impact & Drop Detection Tag (v1)#
A lightweight, self‑calibrating IMU‑based sensor tag for detecting impacts, drops, and abnormal vibration events on drones.
Built using an MPU6050, an ESP32, and a dynamic baseline envelope algorithm that adapts to each drone’s natural vibration profile.

This is v1, the first fully working prototype.

#Features#
✅ Dynamic Baseline Calibration
Collects min/max acceleration magnitude during startup

Learns the drone’s natural vibration envelope

Makes the system device‑agnostic and mounting‑agnostic

✅ Real‑Time Event Classification
Classifies events into four levels:

MINOR – small bumps

MODERATE – noticeable impacts

SEVERE – high‑energy shocks

DROP – free‑fall or sudden loss of support

✅ Risk Scoring System
Each event increases a cumulative risk score:

Minor: +1

Moderate: +5

Severe: +15

Drop: +10

Risk levels:

NORMAL

CAUTION

INSPECTION RECOMMENDED

✅ LED Feedback
1 flash → Minor

2 flashes → Moderate

3 flashes → Severe

4 flashes → Drop

✅ Telemetry Output
Continuous serial output for tuning:

Code
mag, baselineMin, baselineMax, dHigh, dLow, risk, state
📊 Bench Test Results (v1)
Validated behaviors:

✔️ Correctly identified impact severity levels

Minor impacts detected at ~0.4 g above baseline

Moderate impacts detected at ~1.1 g above baseline

✔️ Successfully detected drops from 1 ft

Drop events triggered at ~0.4 g below baseline

✔️ Stable baseline envelope

baselineMin ≈ 0.946 g

baselineMax ≈ 1.173 g

No false positives during idle testing

These results confirm the system’s ability to distinguish real events from noise.

#How It Works#
1. Calibration Phase
On startup, the device samples 200 readings and computes:

baselineMin

baselineMax

This forms the vibration envelope.

2. Live Monitoring
Each new accelerometer reading is converted to magnitude:

Code
mag = sqrt(ax² + ay² + az²)
Then compared to the envelope:

deltaHigh = mag - baselineMax

deltaLow = baselineMin - mag

Thresholds determine event severity.

3. Risk Accumulation
Events add to a cumulative risk score, which maps to a system state.

🛠️ #Hardware#
ESP32 

MPU6050 IMU

LED on GPIO 2

I2C pins:

SDA → 21

SCL → 22

#Installation#
Clone the repo

Install Arduino libraries:

Adafruit_MPU6050

Adafruit_Sensor

Flash to ESP32

Open Serial Monitor at 115200 baud

Keep the drone idle during calibration

Begin testing

🔮 #Roadmap (v2 and beyond)#
v2 — In‑Flight Rolling Baseline Calibration

Add EMA‑based rolling baseline updates
Reduce false positives during flight
Improve robustness across drones and mounting positions

#Future Additions#
BLE or WiFi event streaming

SD card logging

3D‑printed clamp‑on enclosure

Health score decay model

Auto‑tuned thresholds from real data
