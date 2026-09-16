//Input Capture
#include <Arduino.h>
// ================== Timer2: Tạo sóng vuông mẫu 1kHz tại D11 (PB3) ==================
void SignalGenerator_init() {
  DDRB |= (1 << PB3); // D11 làm output
  // CTC mode (WGM21=1, WGM20=0), toggle OC2A mỗi lần compare match (COM2A0=1)
  TCCR2A = (1 << COM2A0) | (1 << WGM21);
  // Prescaler 64
  TCCR2B = (1 << CS22);
  // f_out = F_CPU / (2 * prescaler * (1 + OCR2A))
  // Muốn f_out = 1000Hz -> OCR2A = 16MHz/(2*64*1000) - 1 = 124
  OCR2A = 124;
}
// ================== Timer1: Input Capture đo tần số tại ICP1 (D8/PB0) ==================
volatile uint16_t lastCapture = 0;
volatile uint16_t periodTicks = 0;
volatile bool newCaptureAvailable = false;
ISR(TIMER1_CAPT_vect) {
  uint16_t currentCapture = ICR1;
  // Phép trừ unsigned tự động xử lý đúng cả khi TCNT1 bị tràn (overflow) giữa 2 lần bắt
  periodTicks = currentCapture - lastCapture;
  lastCapture = currentCapture;
  newCaptureAvailable = true;
}
void InputCapture_init() {
  DDRB &= ~(1 << PB0); // D8 (ICP1) làm input
  TCCR1A = 0; // không dùng chân OC1A/OC1B, chỉ dùng bộ đếm free-running
  // ICNC1 = 1: bật noise canceler (lọc nhiễu, chờ 4 mẫu giống nhau mới chấp nhận cạnh)
  // ICES1 = 1: bắt cạnh LÊN (rising edge)
  // CS11 = 1: prescaler = 8 -> mỗi tick = 0.5µs (đủ phân giải cho tín hiệu ~1kHz)
  TCCR1B = (1 << ICNC1) | (1 << ICES1) | (1 << CS11);
  TIMSK1 |= (1 << ICIE1); // bật ngắt Input Capture
  sei();
}
void setup() {
  Serial.begin(9600);
  SignalGenerator_init();
  InputCapture_init();
}
void loop() {
  if (newCaptureAvailable) {
    noInterrupts();
    uint16_t ticks = periodTicks;
    newCaptureAvailable = false;
    interrupts();
    // f_timer = 16MHz / 8 = 2MHz (mỗi tick = 0.5µs)
    float periodSeconds = ticks / 2000000.0;
    float frequencyHz = 1.0 / periodSeconds;
    Serial.print("Period ticks: ");
    Serial.print(ticks);
    Serial.print(" | Period: ");
    Serial.print(periodSeconds * 1000000.0);
    Serial.print(" us | Frequency: ");
    Serial.print(frequencyHz);
    Serial.println(" Hz");
    delay(200); // giãn cách in cho dễ đọc, không ảnh hưởng độ chính xác đo (đo vẫn chạy nền qua ngắt)
  }
}
