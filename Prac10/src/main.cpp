#include <Arduino.h>

constexpr uint8_t PIR_PIN = 2;
constexpr uint8_t MOTION_OUTPUT_PIN = LED_BUILTIN;
constexpr unsigned long SERIAL_BAUD_RATE = 9600;

bool previous_motion_state = false;
bool motion_state_initialized = false;

void setup() {
  pinMode(PIR_PIN, INPUT);
  pinMode(MOTION_OUTPUT_PIN, OUTPUT);
  digitalWrite(MOTION_OUTPUT_PIN, LOW);

  Serial.begin(SERIAL_BAUD_RATE);
}

void loop() {
  const bool motion_detected = digitalRead(PIR_PIN) == HIGH;

  if (!motion_state_initialized || motion_detected != previous_motion_state) {
    previous_motion_state = motion_detected;
    motion_state_initialized = true;

    digitalWrite(MOTION_OUTPUT_PIN, motion_detected ? HIGH : LOW);

    if (motion_detected) {
      Serial.println("Motion detected");
    } else {
      Serial.println("No motion detected");
    }
  }
}