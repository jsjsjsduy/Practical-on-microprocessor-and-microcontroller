#include <Arduino.h>
#include <Servo.h>

constexpr uint8_t SERVO_PIN = 9;
constexpr long MIN_SERVO_ANGLE = 0;
constexpr long MAX_SERVO_ANGLE = 180;
constexpr unsigned long SERIAL_BAUD_RATE = 9600;
constexpr size_t COMMAND_BUFFER_SIZE = 16;

Servo servo;
char command_buffer[COMMAND_BUFFER_SIZE];
size_t command_length = 0;

void process_command(const char *command);

void setup() {
  Serial.begin(SERIAL_BAUD_RATE);
  servo.attach(SERVO_PIN);
  servo.write(0);

  Serial.println("Enter a servo angle from 0 to 180 degrees:");
}

void loop() {
  while (Serial.available() > 0) {
    const char received = Serial.read();

    if (received == '\n' || received == '\r') {
      if (command_length > 0) {
        command_buffer[command_length] = '\0';
        process_command(command_buffer);
        command_length = 0;
      }
    } else if (command_length < COMMAND_BUFFER_SIZE - 1) {
      command_buffer[command_length++] = received;
    } else {
      command_length = 0;
      Serial.println("ERROR: command is too long. Command rejected.");
      Serial.println("Command processed.");
    }
  }
}

void process_command(const char *command) {
  char *end_pointer;
  const long requested_angle = strtol(command, &end_pointer, 10);

  if (command == end_pointer || *end_pointer != '\0') {
    Serial.println("ERROR: enter a whole-number angle from 0 to 180. Command rejected.");
    Serial.println("Command processed.");
    return;
  }

  if (requested_angle < MIN_SERVO_ANGLE || requested_angle > MAX_SERVO_ANGLE) {
    Serial.println("ERROR: angle must be between 0 and 180 degrees. Command rejected.");
    Serial.println("Command processed.");
    return;
  }

  servo.write(static_cast<int>(requested_angle));
  Serial.print("Servo moved to ");
  Serial.print(requested_angle);
  Serial.println(" degrees.");
  Serial.println("Command processed.");
}