#include <Arduino.h>

const uint8_t STEP_PIN = 2;
const uint8_t DIRECTION_PIN = 3;
const uint8_t ENABLE_PIN = 4;
const uint8_t FORWARD_BUTTON_PIN = 5;
const uint8_t REVERSE_BUTTON_PIN = 6;

const unsigned long STEP_INTERVAL_US = 2500;
const unsigned long DEBOUNCE_MS = 30;

unsigned long lastStepMicros = 0;
unsigned long lastButtonChangeMs = 0;
bool lastForwardPressed = false;
bool lastReversePressed = false;

void readButtons()
{
  const unsigned long now = millis();

  if (now - lastButtonChangeMs < DEBOUNCE_MS)
    return;

  const bool forwardPressed = digitalRead(FORWARD_BUTTON_PIN) == LOW;
  const bool reversePressed = digitalRead(REVERSE_BUTTON_PIN) == LOW;

  if (forwardPressed != lastForwardPressed ||
      reversePressed != lastReversePressed)
  {
    lastButtonChangeMs = now;
    lastForwardPressed = forwardPressed;
    lastReversePressed = reversePressed;

    if (forwardPressed && !reversePressed)
      digitalWrite(DIRECTION_PIN, HIGH);
    else if (reversePressed && !forwardPressed)
      digitalWrite(DIRECTION_PIN, LOW);
  }
}

void stepMotor()
{
  const bool forwardPressed = digitalRead(FORWARD_BUTTON_PIN) == LOW;
  const bool reversePressed = digitalRead(REVERSE_BUTTON_PIN) == LOW;

  if (forwardPressed == reversePressed)
    return;

  const unsigned long now = micros();
  if (now - lastStepMicros < STEP_INTERVAL_US)
    return;

  lastStepMicros = now;
  digitalWrite(STEP_PIN, HIGH);
  delayMicroseconds(5);
  digitalWrite(STEP_PIN, LOW);
}

void setup()
{
  pinMode(STEP_PIN, OUTPUT);
  pinMode(DIRECTION_PIN, OUTPUT);
  pinMode(ENABLE_PIN, OUTPUT);
  pinMode(FORWARD_BUTTON_PIN, INPUT_PULLUP);
  pinMode(REVERSE_BUTTON_PIN, INPUT_PULLUP);

  digitalWrite(STEP_PIN, LOW);
  digitalWrite(DIRECTION_PIN, HIGH);
  digitalWrite(ENABLE_PIN, LOW);
}

void loop()
{
  readButtons();
  stepMotor();
}