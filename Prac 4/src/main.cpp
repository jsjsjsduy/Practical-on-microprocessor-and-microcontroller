//Timer and Scheduler
#include <Arduino.h>
#include <LiquidCrystal.h>
// RS, E, D4, D5, D6, D7
LiquidCrystal lcd(6, 7, A2, A3, A4, A5); 
const uint8_t LCD_VIRTUAL_GND = 12;
// ================== Timer1: CTC 1ms tick (nhịp gốc hệ thống) ==================
volatile uint16_t cnt_ADC    = 10;   // Task1 (10ms)
volatile uint16_t cnt_LED    = 20;   // Task2 (20ms)
volatile uint16_t cnt_Sensor = 100;  // Task3 (100ms)
volatile uint16_t cnt_UART   = 1000; // Task4 (1000ms)
volatile bool flag_ADC    = false;
volatile bool flag_LED    = false;
volatile bool flag_Sensor = false;
volatile bool flag_UART   = false;
ISR(TIMER1_COMPA_vect) {
  if (--cnt_ADC == 0)    { cnt_ADC = 10;     flag_ADC = true; }
  if (--cnt_LED == 0)    { cnt_LED = 20;     flag_LED = true; }
  if (--cnt_Sensor == 0) { cnt_Sensor = 100; flag_Sensor = true; }
  if (--cnt_UART == 0)   { cnt_UART = 1000;  flag_UART = true; }
}
void Scheduler_init() {
  TCCR1A = 0;
  TCCR1B = (1 << WGM12) | (1 << CS11) | (1 << CS10); // CTC, prescaler 64
  OCR1A = 249; // 16MHz/64 = 250kHz -> 250 tick = 1ms
  TIMSK1 |= (1 << OCIE1A);
  sei();
}
bool TakeFlag(volatile bool &flag) {
  noInterrupts();
  ;bool pending = flag;
  flag = false;
  interrupts();
  return pending;
}
// ================== Timer2: PWM cho LED ==================
void LED_PWM_init() {
  DDRB |= (1 << PB3); // D11
  TCCR2A = (1 << COM2A1) | (1 << WGM21) | (1 << WGM20); // Fast PWM
  TCCR2B = (1 << CS22); // prescaler 64 -> ~977Hz
  OCR2A = 0;
}
// ================== ADC dùng chung cho Task1 và Task3 ==================
void ADC_init() {
  ADMUX = (1 << REFS0); // AVCC làm điện áp tham chiếu
  ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0); // prescaler 128
}
uint16_t ADC_read(uint8_t channel) {
  ADMUX = (ADMUX & 0xF0) | (channel & 0x0F);
  ADCSRA |= (1 << ADSC);
  while (ADCSRA & (1 << ADSC)); // ~104µs, rất nhanh so với chu kỳ task
  return ADC;
}
// ================== Task 1: Read ADC (cảm biến ánh sáng, A0) mỗi 10ms ==================
volatile uint16_t rawLight = 0;
void Task_ReadADC() {
  rawLight = ADC_read(0); // kênh ADC0
  lcd.setCursor(0, 0);
  lcd.print("Light: ");
  lcd.print(rawLight);
  lcd.print("    "); // xóa ký tự thừa
}
// ================== Task 2: Update LED/PWM mỗi 20ms ==================
uint8_t ledDuty = 0;
int8_t  ledStep = 5;
void Task_UpdateLED() {
  ledDuty += ledStep;
  if (ledDuty == 0 || ledDuty >= 250) ledStep = -ledStep; // hiệu ứng breathing
  OCR2A = ledDuty;
}
// ================== Task 3: Read ADC (cảm biến thứ 2, A1) mỗi 100ms ==================
volatile uint16_t rawSensor2 = 0;
void Task_ReadSensor() {
  rawSensor2 = ADC_read(1); // kênh ADC1
  lcd.setCursor(0, 1);
  lcd.print("Gas: ");
  lcd.print(rawSensor2);
  lcd.print("   ");
}
// ================== Task 4: Send UART mỗi 1000ms ==================
void Task_SendUART() {
  Serial.print("Light(A0): ");
  Serial.print(rawLight);
  Serial.print(" | Gas(A1): ");
  Serial.print(rawSensor2);
  Serial.print(" | LED duty: ");
  Serial.println(ledDuty);
}
void setup() {
  pinMode(LCD_VIRTUAL_GND, OUTPUT);
  digitalWrite(LCD_VIRTUAL_GND, LOW);
  Serial.begin(9600);
  ADC_init();
  LED_PWM_init();
  lcd.begin(16, 2);
  lcd.setCursor(0, 0);
  lcd.print("Starting...");
  Scheduler_init();
}
void loop() {
  if (TakeFlag(flag_ADC))    Task_ReadADC();
  if (TakeFlag(flag_LED))    Task_UpdateLED();
  if (TakeFlag(flag_Sensor)) Task_ReadSensor();
  if (TakeFlag(flag_UART))   Task_SendUART();
}