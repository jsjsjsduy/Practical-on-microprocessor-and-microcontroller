#include <avr/io.h>
#include <stdint.h>

constexpr uint8_t PIR_BIT = PD2;
constexpr uint8_t MOTION_OUTPUT_BIT = PB5;
constexpr unsigned long SERIAL_BAUD_RATE = 9600;

bool previous_motion_state = false;
bool motion_state_initialized = false;

void setup() {
  DDRD &= ~(1 << PIR_BIT);
  DDRB |= (1 << MOTION_OUTPUT_BIT);
  PORTB &= ~(1 << MOTION_OUTPUT_BIT);

  Serial.begin(SERIAL_BAUD_RATE);
}

void loop() {
  const bool motion_detected = (PIND & (1 << PIR_BIT)) != 0;

  if (!motion_state_initialized || motion_detected != previous_motion_state) {
    previous_motion_state = motion_detected;
    motion_state_initialized = true;

    if (motion_detected) PORTB |= (1 << MOTION_OUTPUT_BIT);
    else PORTB &= ~(1 << MOTION_OUTPUT_BIT);

    if (motion_detected) {
      Serial.println("Motion detected");
    } else {
      Serial.println("No motion detected");
    }
  }
}