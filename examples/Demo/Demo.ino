// Arduino Sun Timer Library
// https://github.com/joeyparrish/ArduinoSunTimer
// Copyright (C) 2025 by Joey Parrish and licensed under
// GNU GPL v3.0, https://www.gnu.org/licenses/gpl.html
//
// Based on https://github.com/JChristensen/JC_Sunrise by Jack Christensen
//
// A library to calculate the phase of the day, and the time until the next
// phase.

// This example runs on Linux and Arduino.  To compile on Linux, use:
//   g++ -o demo -x c++ -I src examples/Demo/Demo.ino src/SunTimer.cpp
//   ./demo

#include <SunTimer.h>

#ifdef ARDUINO
void printPadded(int num) {
  if (num < 10) {
    Serial.print("0");
  }
  Serial.print(num);
}
#else
# include <stdio.h>
#endif

SunTimer location(47.68, -122.21);

void testTime(time_t now) {
  SunTimer::phase_t phase;
  int32_t secondsUntilNextPhase;
  location.calculate(now, &phase, &secondsUntilNextPhase);

  // Now you know the phase and how many seconds until the next phase.

  // Everything below here is printing and formatting for inspection.
  // You should get the same results on your PC as on the Arduino.

  time_t nextPhase = now + secondsUntilNextPhase;

  struct tm tmNow;
  gmtime_r(&now, &tmNow);

  struct tm tmNext;
  gmtime_r(&nextPhase, &tmNext);

#ifdef ARDUINO
  Serial.print("Input time is ");
  printPadded(tmNow.tm_hour);
  Serial.print(":");
  printPadded(tmNow.tm_min);
  Serial.print(" (@");
  Serial.print(now);
  Serial.println(")");
#else
  printf("Input time is %02d:%02d (@%ld)\n",
      tmNow.tm_hour, tmNow.tm_min, (long)now);
#endif

#ifdef ARDUINO
  Serial.print("The phase is ");
  Serial.println(SunTimer::phase_name(phase));
#else
  printf("The phase is %s\n", SunTimer::phase_name(phase));
#endif

#ifdef ARDUINO
  Serial.print("Next transition time is ");
  printPadded(tmNext.tm_hour);
  Serial.print(":");
  printPadded(tmNext.tm_min);
  Serial.print(" (@");
  Serial.print(nextPhase);
  Serial.print("), in ");
  Serial.print(secondsUntilNextPhase);
  Serial.println(" seconds.");
  Serial.println("");
#else
  printf("Next transition time is %02d:%02d (@%ld), in %ld seconds.\n\n",
      tmNext.tm_hour, tmNext.tm_min, (long)nextPhase,
      (long)secondsUntilNextPhase);
#endif
}

void setup() {
#ifdef ARDUINO
  Serial.begin(9600);
#endif

  // Edge cases around New Year's
  testTime(1735610400);  // 2024-12-31 02:00:00 UTC
  testTime(1735686000);  // 2024-12-31 23:00:00 UTC
  testTime(1735696800);  // 2025-01-01 02:00:00 UTC
  testTime(1735783200);  // 2025-01-02 02:00:00 UTC

  // A sequence of sunrise/sunset events
  testTime(1765454400);  // 2025-12-11 12:00:00 UTC, 4AM US/PST, NIGHT
  testTime(1765490400);  // 2025-12-11 22:00:00 UTC, 2PM US/PST, DAY
  testTime(1765512000);  // 2025-12-12 04:00:00 UTC, 8PM US/PST, NIGHT
  testTime(1765548000);  // 2025-12-12 14:00:00 UTC, 6AM US/PST, ASTRONOMICAL_TWILIGHT_MORNING
  testTime(1765555200);  // 2025-12-12 16:00:00 UTC, 8AM US/PST, DAY

  // Regression tests... these didn't work at some point.
  testTime(1767396030);  // 2026-01-02 23:20:00 UTC, 15:20 US/PST, DAY
}

#ifdef ARDUINO
void loop() {}
#else
int main() {
  setup();
  return 0;
}
#endif
