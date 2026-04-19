#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <math.h>

Adafruit_MPU6050 mpu;

const int SDA_PIN = 21;
const int SCL_PIN = 22;

// Calibration, need to figure out a timeframe tht will give adequate results
//probably start with 30s-1min calibration period and take avg of magnitudes : max and min

const int CALIBRATION_SAMPLES = 250;
float baselineMin = 10.0;
float baselineMax = 0.0; // in g, will be updated during calibration

// event thresholds above baselineMax (in g)
float minorThreshold = 0.30;
float moderateThreshold = 0.80;
float severeThreshold = 1.50;

//event threshold below baselineMin
float dropThreshold = 0.30; //detects any drops

//risk score
int riskScore = 0;

// prevent duplicate triggers from one hit
unsigned long lastEventTime = 0;
const unsigned long EVENT_COOLDOWN_MS = 500;

// visual sign of every hit
const int LED_PIN = 2;

String getStateFromRisk(int score) {
  if (score >= 25) return "INSPECTION RECOMMENDED";
  if (score >= 10) return "CAUTION";
  return "NORMAL";
}

//using arbitrary numbers to start a point system, each hit adds up
void addRisk(String eventClass) {
  if (eventClass == "MINOR") {
    riskScore += 1;
  } else if (eventClass == "MODERATE") {
    riskScore += 5;
  } else if (eventClass == "SEVERE") {
    riskScore += 15;
  }
}

void flashLED(int times, int delayMs) {
  for (int i = 0; i < times; i++) {
    digitalWrite(LED_PIN, HIGH);
    delay(delayMs);
    digitalWrite(LED_PIN, LOW);
    delay(delayMs);
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  Wire.begin(SDA_PIN, SCL_PIN);

  if (!mpu.begin()) {
    Serial.println("MPU6050 not found. Check wiring.");
    while (1) {
      delay(10);
    }
  }

  // Sensor settings
  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);

  Serial.println("MPU6050 initialized.");
  Serial.println("Starting calibration... proceed to keep drone idle.");

  //resetting baseline envelope
  baselineMin = 10.0;
  baselineMax = 0.0;

  //calibration loop
  for (int i = 0; i < CALIBRATION_SAMPLES; i++) {
    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);

    // Convert m/s^2 to g
    float ax = a.acceleration.x / 9.81;
    float ay = a.acceleration.y / 9.81;
    float az = a.acceleration.z / 9.81;

    float mag = sqrt(ax * ax + ay * ay + az * az);
    if (mag < baselineMin)
    baselineMin = mag;

    if (mag > baselineMax)
    baselineMax = mag;
    

    delay(10);
  }

  //baselineMag = sumMag / CALIBRATION_SAMPLES;

  Serial.print("Calibration complete. Baseline magnitude: ");
  Serial.print(baselineMin, 3);
  Serial.print("Max:  ");
  Serial.print(baselineMax, 3);

  Serial.println(" g");
  Serial.println("Starting live monitoring...");
}

void loop() {
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  // Convert to g
  float ax = a.acceleration.x / 9.81;
  float ay = a.acceleration.y / 9.81;
  float az = a.acceleration.z / 9.81;

  float mag = sqrt(ax * ax + ay * ay + az * az);
  float deltaHigh = (mag - baselineMax);// positive = hit
  float deltaLow = (baselineMin - mag); // positive = drop

  String eventClass = "NONE";

// this is for high-side impacts
  if (deltaHigh >= severeThreshold) {
    eventClass = "SEVERE";
  } else if (deltaHigh >= moderateThreshold) {
    eventClass = "MODERATE";
  } else if (deltaHigh >= minorThreshold) {
    eventClass = "MINOR";
  }
// low-side impacts like free-fall/sudden drop
  else if (deltaLow >= dropThreshold) {
    eventClass = "DROP";
  }

unsigned long now = millis();

  // Trigger only if enough time passed since the last event
  if (eventClass != "NONE" && (now - lastEventTime > EVENT_COOLDOWN_MS)) {
    addRisk(eventClass);
    lastEventTime = now;

    String state = getStateFromRisk(riskScore);

    Serial.println("====================================");
    Serial.print("Event: ");
    Serial.println(eventClass);
    Serial.print("Accel magnitude: ");
    Serial.print(mag, 3);
    Serial.println(" g");
    Serial.print("DeltaHigh: ");
    Serial.println(deltaHigh, 3);
    Serial.print("  DeltaLow: ");
    Serial.println(deltaLow);
    Serial.print("Risk score: ");
    Serial.println(riskScore);
    Serial.print("State: ");
    Serial.println(state);

    if (eventClass == "MINOR") {
      flashLED(1, 100);
    } else if (eventClass == "MODERATE") {
      flashLED(2, 100);
    } else if (eventClass == "SEVERE") {
      flashLED(3, 100);
    }
    else if (eventClass == "DROP"){
      flashLED(4, 80);
    }
  }

  // Continuous telemetry for tuning thresholds
 Serial.print("mag: ");
  Serial.print(mag, 3);
  Serial.print("  min: ");
  Serial.print(baselineMin, 3);
  Serial.print("  max: ");
  Serial.print(baselineMax, 3);
  Serial.print("  dHigh: ");
  Serial.print(deltaHigh, 3);
  Serial.print("  dLow: ");
  Serial.print(deltaLow, 3);
  Serial.print("  risk: ");
  Serial.print(riskScore);
  Serial.print("  state: ");
  Serial.println(getStateFromRisk(riskScore));

  delay(2000);
}
