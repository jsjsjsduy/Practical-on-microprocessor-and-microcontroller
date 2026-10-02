#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>

const uint8_t STEP_BIT = PB2;
const uint8_t DIRECTION_BIT = PD3;
const uint8_t ENABLE_BIT = PD4;
const uint8_t FORWARD_BUTTON_BIT = PD5;
const uint8_t REVERSE_BUTTON_BIT = PD6;

const uint16_t STEP_INTERVAL_TICKS = 5000; // Timer1 at 2 MHz: 2500 us
const uint16_t DEBOUNCE_TICKS = 60000;    // 30 ms

uint16_t lastStepTicks = 0;
uint16_t lastButtonChangeTicks = 0;
bool lastForwardPressed = false;
bool lastReversePressed = false;

static uint16_t timer1_now()
{
  return TCNT1;
}

void readButtons()
{
  const uint16_t now = timer1_now();

  if ((uint16_t)(now - lastButtonChangeTicks) < DEBOUNCE_TICKS)
    return;

  const bool forwardPressed = (PIND & (1 << FORWARD_BUTTON_BIT)) == 0;
  const bool reversePressed = (PIND & (1 << REVERSE_BUTTON_BIT)) == 0;

  if (forwardPressed != lastForwardPressed ||
      reversePressed != lastReversePressed)
  {
    lastButtonChangeTicks = now;
    lastForwardPressed = forwardPressed;
    lastReversePressed = reversePressed;

    if (forwardPressed && !reversePressed)
      PORTD |= (1 << DIRECTION_BIT);
    else if (reversePressed && !forwardPressed)
      PORTD &= ~(1 << DIRECTION_BIT);
  }
}

void stepMotor()
{
  const bool forwardPressed = (PIND & (1 << FORWARD_BUTTON_BIT)) == 0;
  const bool reversePressed = (PIND & (1 << REVERSE_BUTTON_BIT)) == 0;

  if (forwardPressed == reversePressed)
    return;

  const uint16_t now = timer1_now();
  if ((uint16_t)(now - lastStepTicks) < STEP_INTERVAL_TICKS)
    return;

  lastStepTicks = now;
  PORTB |= (1 << STEP_BIT);
  _delay_us(5);
  PORTB &= ~(1 << STEP_BIT);
}

void setup()
{
  DDRB |= (1 << STEP_BIT);
  DDRD |= (1 << DIRECTION_BIT) | (1 << ENABLE_BIT);
  DDRD &= ~((1 << FORWARD_BUTTON_BIT) | (1 << REVERSE_BUTTON_BIT));
  PORTD |= (1 << FORWARD_BUTTON_BIT) | (1 << REVERSE_BUTTON_BIT);

  TCCR1A = 0;
  TCCR1B = (1 << CS11); // Timer1 free-running, 2 MHz tick

  PORTB &= ~(1 << STEP_BIT);
  PORTD |= (1 << DIRECTION_BIT);
  PORTD &= ~(1 << ENABLE_BIT);
}

void loop()
{
  readButtons();
  stepMotor();
}