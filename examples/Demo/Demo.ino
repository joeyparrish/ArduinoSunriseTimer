// Arduino Sunrise Timer Library
// https://github.com/joeyparrish/ArduinoSunriseTimer
// Copyright (C) 2025 by Joey Parrish and licensed under
// GNU GPL v3.0, https://www.gnu.org/licenses/gpl.html
//
// Based on https://github.com/JChristensen/JC_Sunrise by Jack Christensen
//
// Arduino library to calculate whether or not the sun is up, and the time
// until the next transition (sunrise or sunset), with optional offsets.

// This example runs on Linux and Arduino.  To compile on Linux, use:
//   g++ -o demo -x c++ -I src examples/Demo/Demo.ino src/JC_Sunrise.cpp
//   ./demo

#include <JC_Sunrise.h>

#ifndef ARDUINO
# include <stdio.h>
#endif

int zenith = SunriseTimer::civilZenith;
SunriseTimer location(34.0522, -118.2437, zenith);

void testTime(time_t now) {
  bool isUp;
  int32_t secondsUntilTransition;
  location.calculate(now, isUp, secondsUntilTransition);

  // Now you know the state (isUp) and how many seconds until the next
  // transition (secondsUntilTransition).

  // Everything below here is printing and formatting for inspection.
  // You should get the same results on your PC as on the Arduino.

  time_t nextTransition = now + secondsUntilTransition;

  struct tm tm_now;
  gmtime_r(&now, &tm_now);

  struct tm tm_transition;
  gmtime_r(&nextTransition, &tm_transition);

#ifdef ARDUINO
  Serial.print("Input time is ");
  Serial.print(tm_now.tm_hour);
  Serial.print(":");
  Serial.print(tm_now.tm_min);
  Serial.print(" (@");
  Serial.print(now);
  Serial.println(")");
#else
  printf("Input time is %02d:%02d (@%ld)\n",
      tm_now.tm_hour, tm_now.tm_min, (long)now);
#endif

#ifdef ARDUINO
  Serial.print("The sun is ");
  Serial.println(isUp ? "up!", "down!");
#else
  printf("The sun is %s!\n", isUp ? "up" : "down");
#endif

#ifdef ARDUINO
  Serial.print("Transition time is ");
  Serial.print(tm_transition.tm_hour);
  Serial.print(":");
  Serial.print(tm_transition.tm_min);
  Serial.print(" (@");
  Serial.print(nextTransition);
  Serial.print("), in ");
  Serial.print(secondsUntilTransition);
  Serial.println("seconds.");
  Serial.println("");
#else
  printf("Transition time is %02d:%02d (@%ld), in %ld seconds.\n\n",
      tm_transition.tm_hour, tm_transition.tm_min, (long)nextTransition,
      (long)secondsUntilTransition);
#endif
}

void setup() {
#ifdef ARDUINO
  Serial.begin(9600);
#endif

  testTime(1765432728);  // 5:58
  testTime(1765462861);  // 14:21, right after next sunrise
  testTime(1765501921);  // 1:12, right after next sunset
}

#ifdef ARDUINO
void loop() {}
#else
int main() {
  setup();
  return 0;
}
#endif
