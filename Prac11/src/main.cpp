#include <Arduino.h>

constexpr uint8_t LDR_PIN = A0;
constexpr uint8_t LED_PIN = LED_BUILTIN;
constexpr uint16_t DARK_THRESHOLD = 400;
constexpr unsigned long SERIAL_BAUD_RATE = 9600;
constexpr unsigned long REPORT_INTERVAL_MS = 500;
constexpr unsigned long DISPLAY_REFRESH_US = 2000;

constexpr uint8_t SEGMENT_PINS[] = {2, 3, 4, 5, 6, 7, 8};
constexpr uint8_t DIGIT_PINS[] = {9, 10, 11, 12};

const uint8_t DIGIT_SEGMENTS[] = {
    0b00111111,
    0b00000110,
    0b01011011,
    0b01001111,
    0b01100110,
    0b01101101,
    0b01111101,
    0b00000111,
    0b01111111,
    0b01101111,
};

uint16_t adc_value = 0;
uint8_t display_digit = 0;
unsigned long last_report_ms = 0;
unsigned long last_display_refresh_us = 0;

void disable_digits() {
  for (const uint8_t pin : DIGIT_PINS) {
    digitalWrite(pin, HIGH);
  }
}

void write_display_digit(uint8_t value) {
  const uint8_t segments = DIGIT_SEGMENTS[value];

  for (uint8_t segment = 0; segment < 7; ++segment) {
    digitalWrite(SEGMENT_PINS[segment], (segments >> segment) & 0x01);
  }
}

void refresh_display() {
  if (micros() - last_display_refresh_us < DISPLAY_REFRESH_US) {
    return;
  }

  last_display_refresh_us = micros();
  disable_digits();

  uint16_t divisor = 1000;
  for (uint8_t digit = 0; digit < display_digit; ++digit) {
    divisor /= 10;
  }

  write_display_digit((adc_value / divisor) % 10);
  digitalWrite(DIGIT_PINS[display_digit], LOW);
  display_digit = (display_digit + 1) % 4;
}

void setup() {
  pinMode(LDR_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);

  for (const uint8_t pin : SEGMENT_PINS) {
    pinMode(pin, OUTPUT);
  }
  for (const uint8_t pin : DIGIT_PINS) {
    pinMode(pin, OUTPUT);
  }

  disable_digits();
  Serial.begin(SERIAL_BAUD_RATE);
}

void loop() {
  refresh_display();

  adc_value = analogRead(LDR_PIN);
  const bool is_dark = adc_value < DARK_THRESHOLD;
  digitalWrite(LED_PIN, is_dark ? HIGH : LOW);

  if (millis() - last_report_ms >= REPORT_INTERVAL_MS) {
    last_report_ms = millis();

    Serial.print("ADC: ");
    Serial.print(adc_value);
    Serial.print(" | Light condition: ");
    Serial.println(is_dark ? "DARK" : "BRIGHT");
  }
}