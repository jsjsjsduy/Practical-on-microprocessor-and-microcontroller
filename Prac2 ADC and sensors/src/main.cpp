#include <Arduino.h>
#include <LiquidCrystal.h>

#define ADC_CHANNEL 1          // ADC1 = PC1 = Arduino A1
#define ADC_SAMPLES 16
#define ADC_VREF_MV 5000UL

// RS, E, D4, D5, D6, D7
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

volatile uint32_t adcSum = 0;
volatile uint8_t adcCount = 0;
volatile bool adcReady = false;

ISR(ADC_vect)
{
  adcSum += ADC;
  adcCount++;

  if (adcCount >= ADC_SAMPLES)
  {
    ADCSRA &= ~((1 << ADIE) | (1 << ADATE) | (1 << ADEN));
    adcReady = true;
  }
}

void startAdcSampling()
{
  noInterrupts();

  adcSum = 0;
  adcCount = 0;
  adcReady = false;

  // Internal 1.1 V reference, ADC1 selected
  ADMUX = (1 << REFS1) | (1 << REFS0) | ADC_CHANNEL;

  // Free-running mode
  ADCSRB = 0;

  // Clear interrupt flag
  ADCSRA |= (1 << ADIF);

  // Enable ADC, auto trigger, interrupt, prescaler = 128
  ADCSRA =
      (1 << ADEN)  |
      (1 << ADATE) |
      (1 << ADIE)  |
      (1 << ADPS2) |
      (1 << ADPS1) |
      (1 << ADPS0);

  ADCSRA |= (1 << ADSC);

  interrupts();
}

void setup()
{
  // PC0 / A0 permanently connected to GND
  DDRC |= (1 << PC0);
  PORTC &= ~(1 << PC0);

  Serial.begin(9600);

  lcd.begin(16, 2);
  lcd.clear();
  lcd.print("ADC Sensor");

  // Disable digital input on ADC1 / A1
  DIDR0 |= (1 << ADC1D);

  startAdcSampling();
}

void loop()
{
  if (adcReady)
  {
    uint32_t sum;
    uint16_t averageRaw;

    noInterrupts();
    sum = adcSum;
    averageRaw = sum / ADC_SAMPLES;
    interrupts();

    uint32_t voltageMv =
        ((uint32_t)averageRaw * ADC_VREF_MV) / 1023UL;

    lcd.clear();

    lcd.setCursor(0, 0);
    lcd.print("Raw: ");
    lcd.print(averageRaw);

    lcd.setCursor(0, 1);
    lcd.print("V: ");
    lcd.print(voltageMv / 1000.0, 3);
    lcd.print(" V");

    Serial.print("Raw: ");
    Serial.print(averageRaw);
    Serial.print(" Voltage: ");
    Serial.print(voltageMv / 1000.0, 3);
    Serial.println(" V");

    startAdcSampling();
  }
}
