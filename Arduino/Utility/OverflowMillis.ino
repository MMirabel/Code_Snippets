/*********************************************************************************************
**********************************************************************************************
PROJECT:          Arduino Snippets
COMPONENT:        Utility - OverflowMillis
FILE:             OverflowMillis.ino
AUTHOR:           Ing. Mirko Mirabella
                  m.mirabella@neptunengineering.com
                  www.neptunengineering.com
REVISION:         v1.0
DATE:             17/02/2025
**********************************************************************************************/
/*
  Millis Overflow Demonstration Snippet
  Shows how to safeguard periodic scheduling against the 49.7 day wrap-around of millis().
  The sketch forces millis() close to its 32-bit limit on AVR boards, prints timestamps every
  second, and notifies when the overflow occurs so you can observe correct delta handling.
*/

#include <Arduino.h>

#if defined(ARDUINO_ARCH_AVR)
#include <avr/interrupt.h>
extern volatile unsigned long timer0_millis; // defined inside the core timing code
#endif

constexpr unsigned long PRINT_INTERVAL_MS = 1000;
constexpr unsigned long FORCED_WRAP_START = 4294960000UL; // ~80 seconds before overflow

unsigned long nextPrint = 0;
unsigned long lastMillis = 0;
bool overflowReported = false;

void forceMillisOverflow();
void logTimestamp(unsigned long now);
void checkOverflow(unsigned long now);

void setup() {
  Serial.begin(115200);
  const unsigned long serialStart = millis();
  while (!Serial && (millis() - serialStart) < 2000) {
    // Wait for the Serial Monitor (native USB boards)
  }

  Serial.println(F("\nMillis overflow demo ready."));
  Serial.println(F("Observing wrap-around handling for millis()."));

  forceMillisOverflow();

  lastMillis = millis();
  nextPrint = lastMillis;
}

void loop() {
  const unsigned long now = millis();
  checkOverflow(now);

  if ((long)(now - nextPrint) >= 0) {
    logTimestamp(now);
    nextPrint += PRINT_INTERVAL_MS;
  }
}

void forceMillisOverflow() {
#if defined(ARDUINO_ARCH_AVR)
  cli();
  timer0_millis = FORCED_WRAP_START;
  sei();

  Serial.print(F("Timer preset near overflow: "));
  Serial.println(FORCED_WRAP_START);
  Serial.println(F("Wrap expected within ~80 s."));
#else
  Serial.println(F("Running without forced overflow (non-AVR target)."));
  Serial.println(F("Wrap will occur naturally after ~49.7 days."));
#endif
}

void logTimestamp(unsigned long now) {
  Serial.print(F("millis(): "));
  Serial.println(now);
}

void checkOverflow(unsigned long now) {
  if (now < lastMillis && !overflowReported) {
    overflowReported = true;
    Serial.println(F("*** millis() overflow detected ***"));
  }
  lastMillis = now;
}
