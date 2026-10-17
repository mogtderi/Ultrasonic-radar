/*
  Ultrasonic Radar - ESP32 firmware (v1.1)

  Sweeps a servo from 0 to 180 degrees and back, measures the distance at each
  angle with an HC-SR04 and prints "angle,distance" lines over serial
  (115200 baud). The Processing sketch in ../../processing draws the data.

  Wiring (see README):
    Servo signal -> GPIO 13
    HC-SR04 TRIG -> GPIO 5
    HC-SR04 ECHO -> GPIO 18   (HC-SR04 Echo is 5 V: use a voltage divider!)
    Servo and sensor are powered from a separate 5 V supply (LM2596);
    ESP32 GND must be connected to that supply's GND (common ground).

  Library: ESP32Servo (install from the Arduino Library Manager).
*/

#include <ESP32Servo.h>
#include "radar_filter.h"

// ---- Pins -----------------------------------------------------------------
const int TRIG_PIN  = 5;
const int ECHO_PIN  = 18;
const int SERVO_PIN = 13;

// ---- Radar settings ---------------------------------------------------------
const int   MAX_RANGE_CM      = 15;      // anything farther is reported as MAX_RANGE_CM ("no object")
const int   STEP_DEG          = 1;       // smooth 1-degree steps
const int   SERVO_SETTLE_MS   = 35;      // time for the servo to settle before measuring

// ---- Measurement / filtering ----------------------------------------------------
const int   SAMPLES_PER_ANGLE = 3;       // readings per angle (median-based filter)
const int   SAMPLE_GAP_MS     = 8;       // pause between readings so old echoes fade
const int   OUTLIER_TOL_CM    = 3;       // readings further than this from the median are dropped
const unsigned long ECHO_TIMEOUT_US = 3000;   // ~50 cm round trip; short so empty space does not slow the sweep
const float SOUND_SPEED_CM_PER_US = 0.0343f;  // speed of sound at ~20 degrees C

Servo radarServo;

void setup() {
  Serial.begin(115200);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  radarServo.setPeriodHertz(50);
  radarServo.attach(SERVO_PIN, 500, 2400);
}

void loop() {
  // forward sweep 0 -> 180
  for (int angle = 0; angle <= 180; angle += STEP_DEG) {
    scanAngle(angle);
  }
  // return sweep 180 -> 0
  for (int angle = 180; angle >= 0; angle -= STEP_DEG) {
    scanAngle(angle);
  }
}

void scanAngle(int angle) {
  radarServo.write(angle);
  delay(SERVO_SETTLE_MS);

  int distance = getStableDistance();

  Serial.print(angle);
  Serial.print(',');
  Serial.println(distance);
}

// One raw reading in cm. Returns MAX_RANGE_CM when nothing is detected.
int readDistanceOnce() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  unsigned long duration = pulseIn(ECHO_PIN, HIGH, ECHO_TIMEOUT_US);
  if (duration == 0) return MAX_RANGE_CM;  // no echo

  int dist = (int)(duration * SOUND_SPEED_CM_PER_US / 2.0f);
  if (dist <= 0 || dist > MAX_RANGE_CM) return MAX_RANGE_CM;
  return dist;
}

// Several readings per angle, spikes rejected, valid ones averaged.
int getStableDistance() {
  int samples[SAMPLES_PER_ANGLE];
  for (int i = 0; i < SAMPLES_PER_ANGLE; i++) {
    samples[i] = readDistanceOnce();
    if (i < SAMPLES_PER_ANGLE - 1) delay(SAMPLE_GAP_MS);
  }
  return filterDistanceSamples(samples, SAMPLES_PER_ANGLE, MAX_RANGE_CM, OUTLIER_TOL_CM);
}
