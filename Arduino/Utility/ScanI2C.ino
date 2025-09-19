/*
  I2C Bus Scanner Snippet
  Continuously scans the I2C bus, reports detected device addresses, and flags bus errors.
  Wiring (Arduino Uno defaults):
    - SDA -> A4
    - SCL -> A5
    - Pull-up resistors (4.7 kΩ typical) to 3.3V or 5V as required by your bus
  Usage:
    1. Power your I2C peripherals and upload this sketch.
    2. Open the Serial Monitor at 115200 baud to observe scan results.
*/

#include <Wire.h>

constexpr unsigned long SCAN_INTERVAL_MS = 3000;
constexpr uint32_t I2C_BUS_FREQUENCY = 400000;
constexpr uint8_t I2C_ADDRESS_MIN = 0x01;
constexpr uint8_t I2C_ADDRESS_MAX = 0x7F; // 7-bit addressing upper bound (exclusive)

uint8_t scanI2CBus();
void reportWireError(uint8_t address, uint8_t errorCode);
const __FlashStringHelper *wireErrorToString(uint8_t code);

void setup() {
  Serial.begin(115200);
  const unsigned long serialStart = millis();
  while (!Serial && (millis() - serialStart) < 2000) {
    // Wait for Serial monitor (useful on boards with native USB)
  }

  Wire.begin();
  Wire.setClock(I2C_BUS_FREQUENCY);

  Serial.println(F("\nI2C bus scanner ready."));
  Serial.print(F("Bus speed: "));
  Serial.print(I2C_BUS_FREQUENCY / 1000);
  Serial.println(F(" kHz"));
}

void loop() {
  static unsigned long lastScanMs = 0;
  const unsigned long now = millis();
  if (lastScanMs != 0 && (now - lastScanMs) < SCAN_INTERVAL_MS) {
    return;
  }
  lastScanMs = now;

  Serial.println();
  Serial.println(F("Scanning I2C bus..."));
  const unsigned long started = millis();
  const uint8_t devicesFound = scanI2CBus();
  const unsigned long duration = millis() - started;

  if (devicesFound == 0) {
    Serial.println(F("No I2C devices responded."));
  }

  Serial.print(F("Scan complete: "));
  Serial.print(devicesFound);
  Serial.print(devicesFound == 1 ? F(" device") : F(" devices"));
  Serial.print(F(" | Duration: "));
  Serial.print(duration);
  Serial.println(F(" ms"));
}

uint8_t scanI2CBus() {
  uint8_t devicesFound = 0;

  for (uint8_t address = I2C_ADDRESS_MIN; address < I2C_ADDRESS_MAX; ++address) {
    Wire.beginTransmission(address);
    const uint8_t error = Wire.endTransmission();

    if (error == 0) {
      Serial.print(F("  0x"));
      if (address < 16) {
        Serial.print('0');
      }
      Serial.print(address, HEX);
      Serial.println(F(" (ACK)"));
      ++devicesFound;
    } else if (error != 2) {
      reportWireError(address, error);
    }
  }

  return devicesFound;
}

void reportWireError(uint8_t address, uint8_t errorCode) {
  Serial.print(F("  0x"));
  if (address < 16) {
    Serial.print('0');
  }
  Serial.print(address, HEX);
  Serial.print(F(" (error "));
  Serial.print(errorCode);
  Serial.print(F(": "));
  Serial.print(wireErrorToString(errorCode));
  Serial.println(')');
}

const __FlashStringHelper *wireErrorToString(uint8_t code) {
  switch (code) {
    case 1:
      return F("data too long");
    case 2:
      return F("address NACK");
    case 3:
      return F("data NACK");
    case 4:
      return F("other error");
#if defined(ARDUINO_ARCH_SAMD) || defined(ARDUINO_ARCH_NRF5)
    case 5:
      return F("timeout");
#endif
    default:
      return F("unknown");
  }
}
