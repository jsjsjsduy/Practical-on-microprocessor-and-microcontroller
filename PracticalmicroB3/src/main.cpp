#include <Arduino.h>

// One shared output pin for both LED and buzzer
constexpr uint8_t BUTTON_PIN = 2;
constexpr uint8_t SHARED_OUTPUT_PIN = 9;
constexpr unsigned long DEBOUNCE_DELAY = 30; // ms

bool outputState = false;
uint8_t stableState = HIGH;
uint8_t lastReading = HIGH;
unsigned long lastDebounceTime = 0;

void setup() {
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(SHARED_OUTPUT_PIN, OUTPUT);

  // Force output OFF on boot
  digitalWrite(SHARED_OUTPUT_PIN, LOW);
}

void loop() {
  uint8_t reading = digitalRead(BUTTON_PIN);

  if (reading != lastReading) {
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > DEBOUNCE_DELAY) {
    if (reading != stableState) {
      stableState = reading;

      // Active-LOW button because INPUT_PULLUP is used
      if (stableState == LOW) {
        outputState = !outputState;
        digitalWrite(SHARED_OUTPUT_PIN, outputState ? HIGH : LOW);
      }
    }
  }

  lastReading = reading;
}