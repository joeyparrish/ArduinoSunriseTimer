// Arduino Sun Timer Library
// https://github.com/joeyparrish/ArduinoSunTimer
// Copyright (C) 2025 by Joey Parrish and licensed under
// GNU GPL v3.0, https://www.gnu.org/licenses/gpl.html
//
// Based on https://github.com/JChristensen/JC_Sunrise by Jack Christensen
//
// A library to calculate the phase of the day, and the time until the next
// phase.

#ifndef SUN_TIMER_H_INCLUDED
#define SUN_TIMER_H_INCLUDED

#include <stdint.h>

#ifndef ARDUINO
# include <time.h>
#else
typedef uint32_t time_t;
typedef struct tm {
  int8_t tm_sec;  // 0 to 59
  int8_t tm_min;  // 0 to 59
  int8_t tm_hour; // 0 to 23
  int8_t tm_mday; // 1 to 31
  int8_t tm_mon;  // 0 to 11
  int16_t tm_year; // years since 1900
  int16_t tm_yday; // 0 to 365
};

struct tm* gmtime_r(const time_t* timeInput, struct tm* tm);
time_t timegm(struct tm *tm);
#endif

class SunTimer {
public:
  SunTimer(float lat, float lon)
      : m_lat{lat}, m_lon{lon} {}

  typedef enum {
    ASTRONOMICAL_TWILIGHT_MORNING,
    NAUTICAL_TWILIGHT_MORNING,
    CIVIL_TWILIGHT_MORNING,
    DAY,
    CIVIL_TWILIGHT_EVENING,
    NAUTICAL_TWILIGHT_EVENING,
    ASTRONOMICAL_TWILIGHT_EVENING,
    NIGHT,

    MIN = ASTRONOMICAL_TWILIGHT_MORNING,
    MAX = NIGHT,
  } phase_t;

  void calculate(
      time_t time, phase_t* currentPhase, int32_t* secondsUntilNextPhase);

  static const char* phase_name(phase_t phase);

private:
  static constexpr float
      officialZenith {90.83333},
      civilZenith {96.0},
      nauticalZenith {102.0},
      astronomicalZenith {108.0};

  float m_lat;
  float m_lon;
  float m_zenith;

  void phaseParameters(phase_t phase, float* zenith, bool* sunset);
  bool phaseBegins(
      const struct tm* tmInput, phase_t phase, uint16_t* timeOfDay);
  bool calcSunset(
      const struct tm* tmIn, bool sunset, float zenith, struct tm* tmOut);
  bool calcSunsetPrimitive(
      int doy, bool sunset, float zenith, int8_t& hourOut, int8_t& minutesOut);

  float AdjustTo360(float i);
  float deg2rad(float degrees);
  float rad2deg(float radians);
};

#endif  // SUN_TIMER_H_INCLUDED
