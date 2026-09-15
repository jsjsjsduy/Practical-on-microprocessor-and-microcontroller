#include <Arduino.h>
#include <Servo.h>

const int ledPin = 3; 
const int servoPin = 10;
const int minBrightness = 30;

Servo myServo;

void setup() {
  pinMode(ledPin, OUTPUT);
  myServo.attach(servoPin);

  Serial.begin(9600);
  Serial.println("Servo + LED PWM Demo");
  Serial.println("LED stays on while brightness changes with servo angle");
}

void loop() {
  for (int angle = 0; angle <= 180; angle += 1) {
    myServo.write(angle);

    int brightness = map(angle, 0, 180, minBrightness, 255);
    analogWrite(ledPin, brightness);

    Serial.print("Angle: ");
    Serial.print(angle);
    Serial.print(" deg | LED PWM: ");
    Serial.println(brightness);

    delay(20);
  }

  for (int angle = 180; angle >= 0; angle -= 1) {
    myServo.write(angle);

    int brightness = map(angle, 0, 180, minBrightness, 255);
    analogWrite(ledPin, brightness);

    Serial.print("Angle: ");
    Serial.print(angle);
    Serial.print(" deg | LED PWM: ");
    Serial.println(brightness);

    delay(20);
  }
}