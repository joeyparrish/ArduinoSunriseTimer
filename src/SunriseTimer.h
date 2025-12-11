// Arduino Sunrise Timer Library
// https://github.com/joeyparrish/ArduinoSunriseTimer
// Copyright (C) 2025 by Joey Parrish and licensed under
// GNU GPL v3.0, https://www.gnu.org/licenses/gpl.html
//
// Based on https://github.com/JChristensen/JC_Sunrise by Jack Christensen
//
// Arduino library to calculate whether or not the sun is up, and the time
// until the next transition (sunrise or sunset), with optional offsets.

#ifndef SUNRISE_TIMER_H_INCLUDED
#define SUNRISE_TIMER_H_INCLUDED

#include <cstdint>

#ifndef ARDUINO
# include <ctime>
#else
typedef uint32_t time_t;
typedef struct tm {
  uint8_t tm_sec;  // 0 to 59
  uint8_t tm_min;  // 0 to 59
  uint8_t tm_hour; // 0 to 23
  uint8_t tm_mday; // 1 to 31
  uint8_t tm_mon;  // 0 to 11
  uint8_t tm_year; // years since 1900
  uint8_t tm_yday; // 0 to 365
};

struct tm* gmtime_r(const time_t* timeInput, struct tm* tm);
time_t timegm(struct tm *tm);
#endif

class SunriseTimer {
public:
  SunriseTimer(float lat, float lon, float zenith)
      : m_lat{lat}, m_lon{lon}, m_zenith{zenith} {}

  void calculate(time_t t, bool& isUp, int32_t& secondsUntilTransition);

  static constexpr float
      officialZenith {90.83333},
      civilZenith {96.0},
      nauticalZenith {102.0},
      astronomicalZenith {108.0};

private:
  float m_lat;
  float m_lon;
  float m_zenith;

  void calcSunset(int doy, bool sunset, uint8_t& hourOut, uint8_t& minutesOut);

  float AdjustTo360(float i);
  float AdjustTo24(float i);
  float deg2rad(float degrees);
  float rad2deg(float radians);
};

#endif  // SUNRISE_TIMER_H_INCLUDED
