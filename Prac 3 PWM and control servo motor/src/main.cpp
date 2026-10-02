#include <Arduino.h>
#include <Servo.h>
#include <avr/io.h>
#include <util/delay.h>

const int ledPin = 3; 
const int servoPin = 10;
const int minBrightness = 30;

Servo myServo;

void setup() {
  DDRD |= (1 << DDD3);
  TCCR2A = (1 << COM2B1) | (1 << WGM21) | (1 << WGM20);
  TCCR2B = (1 << CS22);
  OCR2B = 0;
  myServo.attach(servoPin);

  Serial.begin(9600);
  Serial.println("Servo + LED PWM Demo");
  Serial.println("LED stays on while brightness changes with servo angle");
}

void loop() {
  for (int angle = 0; angle <= 180; angle += 1) {
    myServo.write(angle);

    int brightness = minBrightness + (angle * (255 - minBrightness)) / 180;
    OCR2B = brightness;

    Serial.print("Angle: ");
    Serial.print(angle);
    Serial.print(" deg | LED PWM: ");
    Serial.println(brightness);

    _delay_ms(20);
  }

  for (int angle = 180; angle >= 0; angle -= 1) {
    myServo.write(angle);

    int brightness = minBrightness + (angle * (255 - minBrightness)) / 180;
    OCR2B = brightness;

    Serial.print("Angle: ");
    Serial.print(angle);
    Serial.print(" deg | LED PWM: ");
    Serial.println(brightness);

    _delay_ms(20);
  }
}