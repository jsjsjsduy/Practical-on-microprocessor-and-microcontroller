#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>

// One shared output pin for both LED and buzzer
constexpr uint8_t BUTTON_BIT = PD2;
constexpr uint8_t SHARED_OUTPUT_BIT = PB1;
constexpr uint32_t DEBOUNCE_DELAY = 30;

bool outputState = false;
bool stableState = true;
bool lastReading = true;
volatile uint32_t milliseconds = 0;

ISR(TIMER1_COMPA_vect)
{
  ++milliseconds;
}

uint32_t millis_now()
{
  uint32_t value;
  uint8_t savedStatus = SREG;
  cli();
  value = milliseconds;
  SREG = savedStatus;
  return value;
}

void setup()
{
  DDRD &= ~(1 << BUTTON_BIT);
  PORTD |= (1 << BUTTON_BIT);
  DDRB |= (1 << SHARED_OUTPUT_BIT);
  PORTB &= ~(1 << SHARED_OUTPUT_BIT);

  TCCR1A = 0;
  TCCR1B = (1 << WGM12) | (1 << CS11) | (1 << CS10);
  OCR1A = 249;
  TIMSK1 = (1 << OCIE1A);
  sei();
}

void loop()
{
  static uint32_t lastDebounceTime = 0;
  bool reading = (PIND & (1 << BUTTON_BIT)) != 0;

  if (reading != lastReading) {
    lastDebounceTime = millis_now();
  }

  if ((millis_now() - lastDebounceTime) > DEBOUNCE_DELAY) {
    if (reading != stableState) {
      stableState = reading;

      if (!stableState) {
        outputState = !outputState;
        if (outputState) PORTB |= (1 << SHARED_OUTPUT_BIT);
        else PORTB &= ~(1 << SHARED_OUTPUT_BIT);
      }
    }
  }

  lastReading = reading;
}