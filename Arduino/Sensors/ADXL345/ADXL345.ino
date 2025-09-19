/*********************************************************************************************
PROJECT:          Code Snippets
COMPONENT:        Arduino/Sensors/ADXL345
FILE:             ADXL345.ino
AUTHOR:           Ing. Mirko Mirabella
                  m.mirabella@neptunengineering.com
                  www.neptunengineering.com
REVISION:         v. 1.0
DATE:             19/09/2025
**********************************************************************************************/

/*
  ADXL345 Quickstart Snippet
  Demonstrates initialization, calibration, and periodic acceleration output (in g) for the ADXL345 accelerometer over I2C.
  Wiring (default Arduino Uno pins):
    - VCC -> 3.3V
    - GND -> GND
    - SDA -> A4 (SDA)
    - SCL -> A5 (SCL)
*/

#include <Wire.h>

constexpr uint8_t ADXL345_ADDRESS = 0x53;
constexpr uint8_t REG_POWER_CTL = 0x2D;
constexpr uint8_t REG_DATA_FORMAT = 0x31;
constexpr uint8_t REG_BW_RATE = 0x2C;
constexpr uint8_t REG_DATAX0 = 0x32;

constexpr uint8_t MEASURE_BIT = 0x08;
constexpr uint8_t FULL_RES_BIT = 0x08;
constexpr uint8_t RANGE_8G = 0x02;
constexpr uint8_t RATE_100HZ = 0x0A;

constexpr float G_PER_LSB = 0.0039f; // 3.9 mg/LSB in full-resolution mode
constexpr uint16_t CALIBRATION_SAMPLES = 200;
constexpr uint16_t CALIBRATION_SAMPLE_DELAY_MS = 5;
constexpr unsigned long SAMPLE_PERIOD_MS = 200;

struct Vector3 {
  float x;
  float y;
  float z;
};

Vector3 bias = {0.0f, 0.0f, 0.0f};

bool writeRegister(uint8_t reg, uint8_t value);
bool readRegisters(uint8_t startReg, uint8_t *buffer, size_t length);
bool initializeADXL345();
bool readRawAcceleration(Vector3 &raw);
bool readAcceleration(Vector3 &measurement);
void calibrateADXL345();
void printAcceleration(const Vector3 &g);
float vectorMagnitude(const Vector3 &v);

void setup() {
  Serial.begin(115200);
  delay(50);

  Wire.begin();

  if (!initializeADXL345()) {
    Serial.println(F("ADXL345 initialization failed. Check wiring and power."));
    while (true) {
      delay(1000);
    }
  }

  Serial.println(F("Hold the board still for calibration..."));
  calibrateADXL345();
  Serial.println(F("Calibration done. Streaming acceleration in g."));
}

void loop() {
  static unsigned long lastSampleMs = 0;
  const unsigned long now = millis();
  if (now - lastSampleMs < SAMPLE_PERIOD_MS) {
    return;
  }
  lastSampleMs = now;

  Vector3 measurement;
  if (!readAcceleration(measurement)) {
    Serial.println(F("Failed to read acceleration data."));
    return;
  }

  printAcceleration(measurement);
}

bool writeRegister(uint8_t reg, uint8_t value) {
  Wire.beginTransmission(ADXL345_ADDRESS);
  Wire.write(reg);
  Wire.write(value);
  return Wire.endTransmission() == 0;
}

bool readRegisters(uint8_t startReg, uint8_t *buffer, size_t length) {
  Wire.beginTransmission(ADXL345_ADDRESS);
  Wire.write(startReg);
  if (Wire.endTransmission(false) != 0) {
    return false;
  }

  const uint8_t bytesRequested = static_cast<uint8_t>(length);
  const uint8_t bytesRead = Wire.requestFrom(ADXL345_ADDRESS, bytesRequested, true);
  if (bytesRead != bytesRequested) {
    return false;
  }

  for (size_t i = 0; i < length; ++i) {
    buffer[i] = static_cast<uint8_t>(Wire.read());
  }
  return true;
}

bool initializeADXL345() {
  if (!writeRegister(REG_POWER_CTL, 0x00)) { // ensure standby
    return false;
  }
  delay(5);

  if (!writeRegister(REG_DATA_FORMAT, FULL_RES_BIT | RANGE_8G)) {
    return false;
  }

  if (!writeRegister(REG_BW_RATE, RATE_100HZ)) {
    return false;
  }

  if (!writeRegister(REG_POWER_CTL, MEASURE_BIT)) {
    return false;
  }

  delay(10);
  return true;
}

bool readRawAcceleration(Vector3 &raw) {
  uint8_t buffer[6];
  if (!readRegisters(REG_DATAX0, buffer, sizeof(buffer))) {
    return false;
  }

  const int16_t x = static_cast<int16_t>((static_cast<int16_t>(buffer[1]) << 8) | buffer[0]);
  const int16_t y = static_cast<int16_t>((static_cast<int16_t>(buffer[3]) << 8) | buffer[2]);
  const int16_t z = static_cast<int16_t>((static_cast<int16_t>(buffer[5]) << 8) | buffer[4]);

  raw.x = static_cast<float>(x) * G_PER_LSB;
  raw.y = static_cast<float>(y) * G_PER_LSB;
  raw.z = static_cast<float>(z) * G_PER_LSB;

  return true;
}

bool readAcceleration(Vector3 &measurement) {
  Vector3 raw;
  if (!readRawAcceleration(raw)) {
    return false;
  }

  measurement.x = raw.x - bias.x;
  measurement.y = raw.y - bias.y;
  measurement.z = raw.z - bias.z;

  return true;
}

void calibrateADXL345() {
  Vector3 sum = {0.0f, 0.0f, 0.0f};
  uint16_t collected = 0;
  uint16_t attempts = 0;
  const uint16_t maxAttempts = CALIBRATION_SAMPLES * 3;

  while (collected < CALIBRATION_SAMPLES && attempts < maxAttempts) {
    Vector3 raw;
    if (readRawAcceleration(raw)) {
      sum.x += raw.x;
      sum.y += raw.y;
      sum.z += raw.z;
      ++collected;
    }
    ++attempts;
    delay(CALIBRATION_SAMPLE_DELAY_MS);
  }

  if (collected == 0) {
    Serial.println(F("Calibration skipped. Using default offsets."));
    return;
  }

  const float denom = static_cast<float>(collected);
  const Vector3 average = {sum.x / denom, sum.y / denom, sum.z / denom};
  const Vector3 expected = {0.0f, 0.0f, 1.0f};

  bias.x = average.x - expected.x;
  bias.y = average.y - expected.y;
  bias.z = average.z - expected.z;

  Serial.print(F("Bias (g): "));
  Serial.print(bias.x, 4);
  Serial.print(F(", "));
  Serial.print(bias.y, 4);
  Serial.print(F(", "));
  Serial.println(bias.z, 4);
}

void printAcceleration(const Vector3 &g) {
  Serial.print(F("X: "));
  Serial.print(g.x, 3);
  Serial.print(F(" g  Y: "));
  Serial.print(g.y, 3);
  Serial.print(F(" g  Z: "));
  Serial.print(g.z, 3);
  Serial.print(F(" g  |g|: "));
  Serial.println(vectorMagnitude(g), 3);
}

float vectorMagnitude(const Vector3 &v) {
  return sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
}
