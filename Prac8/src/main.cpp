#include <Arduino.h>
#include <avr/io.h>

const uint8_t I2C_SDA_PIN = A4;
const uint8_t I2C_SCL_PIN = A5;
const uint8_t MPU6050_ADDRESS = 0x68;
const uint8_t MPU6050_WHO_AM_I = 0x75;
const uint8_t MPU6050_PWR_MGMT_1 = 0x6B;
const uint8_t MPU6050_TEMP_OUT_H = 0x41;

void i2c_init() {
  TWSR = 0;
  TWBR = 72; // 100 kHz SCL with a 16 MHz CPU clock
  TWCR = (1 << TWEN);
}

bool i2c_start() {
  TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);
  while (!(TWCR & (1 << TWINT))) {
  }

  return TWSR == 0x08 || TWSR == 0x10;
}

bool i2c_write(uint8_t data) {
  TWDR = data;
  TWCR = (1 << TWINT) | (1 << TWEN);
  while (!(TWCR & (1 << TWINT))) {
  }

  return TWSR == 0x18 || TWSR == 0x28 || TWSR == 0x40;
}

uint8_t i2c_read_ack() {
  TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWEA);
  while (!(TWCR & (1 << TWINT))) {
  }

  return TWDR;
}

uint8_t i2c_read_nack() {
  TWCR = (1 << TWINT) | (1 << TWEN);
  while (!(TWCR & (1 << TWINT))) {
  }

  return TWDR;
}

void i2c_stop() {
  TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWSTO);
}

bool mpu6050_write_register(uint8_t reg, uint8_t value) {
  if (!i2c_start()) return false;
  if (!i2c_write(MPU6050_ADDRESS << 1)) {
    i2c_stop();
    return false;
  }
  if (!i2c_write(reg) || !i2c_write(value)) {
    i2c_stop();
    return false;
  }
  i2c_stop();
  return true;
}

bool mpu6050_read_registers(uint8_t reg, uint8_t *data, uint8_t length) {
  if (!i2c_start()) return false;
  if (!i2c_write(MPU6050_ADDRESS << 1) || !i2c_write(reg)) {
    i2c_stop();
    return false;
  }

  if (!i2c_start() || !i2c_write((MPU6050_ADDRESS << 1) | 1)) {
    i2c_stop();
    return false;
  }

  for (uint8_t index = 0; index < length; ++index) {
    data[index] = index + 1 < length ? i2c_read_ack() : i2c_read_nack();
  }
  i2c_stop();
  return true;
}

void setup() {
  Serial.begin(9600);

  pinMode(I2C_SDA_PIN, INPUT_PULLUP);
  pinMode(I2C_SCL_PIN, INPUT_PULLUP);
  i2c_init();

  Serial.println(F("Prac 8: low-level I2C MPU6050"));
  Serial.println(F("SDA: A4 | SCL: A5 | GND: GND"));
  Serial.println(F("START -> address + R/W -> ACK -> DATA -> STOP"));

  if (!mpu6050_write_register(MPU6050_PWR_MGMT_1, 0x00)) {
    Serial.println(F("MPU6050 wake-up failed"));
    return;
  }

  uint8_t deviceId = 0;
  if (mpu6050_read_registers(MPU6050_WHO_AM_I, &deviceId, 1)) {
    Serial.print(F("WHO_AM_I: 0x"));
    Serial.println(deviceId, HEX);
  } else {
    Serial.println(F("MPU6050 read failed"));
  }
}

void loop() {
  uint8_t temperatureBytes[2];
  if (mpu6050_read_registers(MPU6050_TEMP_OUT_H, temperatureBytes, 2)) {
    int16_t rawTemperature = (static_cast<int16_t>(temperatureBytes[0]) << 8) |
                             temperatureBytes[1];
    float temperatureCelsius = rawTemperature / 340.0f + 36.53f;

    Serial.print(F("Temperature: "));
    Serial.print(temperatureCelsius, 2);
    Serial.println(F(" C"));
  } else {
    Serial.println(F("Sensor read failed"));
  }

  delay(1000);
}