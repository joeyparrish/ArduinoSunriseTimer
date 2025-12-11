// Arduino Sunrise Timer Library
// https://github.com/joeyparrish/ArduinoSunriseTimer
// Copyright (C) 2025 by Joey Parrish and licensed under
// GNU GPL v3.0, https://www.gnu.org/licenses/gpl.html
//
// Based on https://github.com/JChristensen/JC_Sunrise by Jack Christensen
//
// Arduino library to calculate whether or not the sun is up, and the time
// until the next transition (sunrise or sunset).

// This example runs on Linux and Arduino.  To compile on Linux, use:
//   g++ -o demo -x c++ -I src examples/Demo/Demo.ino src/SunriseTimer.cpp
//   ./demo

#include <SunriseTimer.h>

#ifndef ARDUINO
# include <stdio.h>
#endif

constexpr float zenith = SunriseTimer::civilZenith;
SunriseTimer location(34.0522, -118.2437, zenith);

void testTime(time_t now) {
  bool isUp;
  int32_t secondsSinceLastTransition, secondsUntilNextTransition;
  location.calculate(now, &isUp,
      &secondsSinceLastTransition, &secondsUntilNextTransition);

  // Now you know the state (isUp) and how many seconds until the next
  // transition (secondsUntilNextTransition).

  // Everything below here is printing and formatting for inspection.
  // You should get the same results on your PC as on the Arduino.

  time_t lastTransition = now - secondsSinceLastTransition;
  time_t nextTransition = now + secondsUntilNextTransition;

  struct tm tmNow;
  gmtime_r(&now, &tmNow);

  struct tm tmLast;
  gmtime_r(&lastTransition, &tmLast);

  struct tm tmNext;
  gmtime_r(&nextTransition, &tmNext);

#ifdef ARDUINO
  Serial.print("Input time is ");
  Serial.print(tmNow.tm_hour);
  Serial.print(":");
  Serial.print(tmNow.tm_min);
  Serial.print(" (@");
  Serial.print(now);
  Serial.println(")");
#else
  printf("Input time is %02d:%02d (@%ld)\n",
      tmNow.tm_hour, tmNow.tm_min, (long)now);
#endif

#ifdef ARDUINO
  Serial.print("The sun is ");
  Serial.println(isUp ? "up!" : "down!");
#else
  printf("The sun is %s!\n", isUp ? "up" : "down");
#endif

#ifdef ARDUINO
  Serial.print("Last transition time was ");
  Serial.print(tmLast.tm_hour);
  Serial.print(":");
  Serial.print(tmLast.tm_min);
  Serial.print(" (@");
  Serial.print(lastTransition);
  Serial.print("), ");
  Serial.print(secondsSinceLastTransition);
  Serial.println("seconds ago.");
#else
  printf("Last transition time was %02d:%02d (@%ld), %ld seconds ago.\n",
      tmLast.tm_hour, tmLast.tm_min, (long)lastTransition,
      (long)secondsSinceLastTransition);
#endif

#ifdef ARDUINO
  Serial.print("Next transition time is ");
  Serial.print(tmNext.tm_hour);
  Serial.print(":");
  Serial.print(tmNext.tm_min);
  Serial.print(" (@");
  Serial.print(nextTransition);
  Serial.print("), in ");
  Serial.print(secondsUntilNextTransition);
  Serial.println("seconds.");
  Serial.println("");
#else
  printf("Transition time is %02d:%02d (@%ld), in %ld seconds.\n\n",
      tmNext.tm_hour, tmNext.tm_min, (long)nextTransition,
      (long)secondsUntilNextTransition);
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
  testTime(1765454400);  // 2025-12-11 12:00:00 UTC, 4AM US/PST, before sunrise
  testTime(1765490400);  // 2025-12-11 22:00:00 UTC, 2PM US/PST, daytime
  testTime(1765512000);  // 2025-12-12 04:00:00 UTC, 8PM US/PST, after sunset
  testTime(1765548000);  // 2025-12-12 14:00:00 UTC, 6AM US/PST, before next sunrise
  testTime(1765555200);  // 2025-12-12 16:00:00 UTC, 8AM US/PST, after next sunrise
}

#ifdef ARDUINO
void loop() {}
#else
int main() {
  setup();
  return 0;
}
#endif
