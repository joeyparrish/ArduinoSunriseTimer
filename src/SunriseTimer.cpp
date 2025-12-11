// Arduino Sunrise Timer Library
// https://github.com/joeyparrish/ArduinoSunriseTimer
// Copyright (C) 2025 by Joey Parrish and licensed under
// GNU GPL v3.0, https://www.gnu.org/licenses/gpl.html
//
// Based on https://github.com/JChristensen/JC_Sunrise by Jack Christensen
//
// Arduino library to calculate whether or not the sun is up, and the time
// until the next transition (sunrise or sunset), with optional offsets.

#include <SunriseTimer.h>

static constexpr float PI {3.141593};
static constexpr uint32_t SECONDS_PER_DAY {86400};
static constexpr uint32_t SECONDS_PER_HOUR {3600};
static constexpr uint32_t SECONDS_PER_MINUTE {60};

#ifndef ARDUINO
# include <cmath>
#else
// Input years are offset from 1970
#define FULL_YEAR(Y) (1970 + (Y))
#define IS_POSITIVE(Y) (FULL_YEAR(Y) > 0)
#define MULTIPLE_OF_4(Y) ((FULL_YEAR(Y) % 4) == 0)
#define MULTIPLE_OF_100(Y) ((FULL_YEAR(Y) % 100) == 0)
#define MULTIPLE_OF_400(Y) ((FULL_YEAR(Y) % 400) == 0)
#define LEAP_YEAR(Y) (IS_POSITIVE(Y) && MULTIPLE_OF_4(Y) && (!MULTIPLE_OF_100(Y) || MULTIPLE_OF_400(Y)))

static const uint8_t monthDays[] = {
  31, // Jan
  28, // Feb
  31, // Mar
  30, // Apr
  31, // May
  30, // Jun
  31, // Jul
  31, // Aug
  30, // Sep
  31, // Oct
  30, // Nov
  31, // Dec
};

struct tm* gmtime_r(const time_t* timeInput, struct tm* tm) {
  // Based on https://github.com/PaulStoffregen/Time/blob/master/Time.cpp
  // Originally Copyright (c) Michael Margolis 2009-2014
  // Licensed under GNU LGPL v2.1+
  // Modified to match gmtime_r for simplified testing and validation
  // between PC & Arduino.
  uint8_t year;
  uint8_t month, monthLength;
  uint32_t time;
  uint32_t days;

  time = (uint32_t)*timeInput;
  tm->tm_sec = time % 60;
  time /= 60; // now it is minutes
  tm->tm_min = time % 60;
  time /= 60; // now it is hours
  tm->tm_hour = time % 24;
  time /= 24; // now it is days

  year = 0;
  days = 0;
  while((unsigned)(days += (LEAP_YEAR(year) ? 366 : 365)) <= time) {
    year++;
  }
  tm->tm_year = year + 70; // year is offset from 1970, tm_year from 1900

  days -= LEAP_YEAR(year) ? 366 : 365;
  time -= days; // now it is days in this year, starting at 0
  tm->tm_yday = time;  // 0-based

  days = 0;
  month = 0;
  monthLength = 0;

  for (month = 0; month < 12; month++) {
    if (month == 1) { // february
      if (LEAP_YEAR(year)) {
        monthLength = 29;
      } else {
        monthLength = 28;
      }
    } else {
      monthLength = monthDays[month];
    }

    if (time >= monthLength) {
      time -= monthLength;
    } else {
        break;
    }
  }

  tm->tm_mon = month;  // month is 0-based
  tm->tm_mday = time + 1;  // day of month is 1-based

  return tm;
}

// Expects tm->tm_yday to be correct, and ignores tm->tm_mon and tm->mday!
time_t timegm(struct tm *tm) {
  time_t time = 0;
  uint8_t targetYear = tm->tm_year - 70;  // 1900-base to 1970-base
  uint8_t year = 0;
  while (year < targetYear) {
    time += LEAP_YEAR(year) ? 366 : 365;
  }
  time += tm->tm_yday;
  // time is now in days elapsed since 1970

  time *= SECONDS_PER_DAY;
  // time is now in seconds elapsed since 1970

  time += tm->tm_hour * SECONDS_PER_HOUR;
  time += tm->tm_min * SECONDS_PER_MINUTE;
  time += tm->tm_sec;

  return time;
}
#endif

// Calculate whether or not the sun is up, and how long until the next
// transition (sunrise or sunset).
void SunriseTimer::calculate(
    time_t t, bool& isUp, int32_t& secondsUntilTransition) {
  uint8_t transitionHour, transitionMinute;
  uint16_t transitionTimeOfDay;
  uint16_t inputTimeOfDay;

  struct tm tm;
  gmtime_r(&t, &tm);

  inputTimeOfDay = tm.tm_hour * 60 + tm.tm_min;

  // Check sunrise for this day of the year.
  calcSunset(tm.tm_yday, false, transitionHour, transitionMinute);
  transitionTimeOfDay = transitionHour * 60 + transitionMinute;

  if (transitionTimeOfDay > inputTimeOfDay) {
    // Waiting on sunrise.
    isUp = false;

    tm.tm_hour = transitionHour;
    tm.tm_min = transitionMinute;
    tm.tm_sec = 0;

    time_t transition = timegm(&tm);
    secondsUntilTransition = transition - t;
    return;
  }

  // Waiting on sunset.
  isUp = true;

  // Check sunset for this day of the year.
  calcSunset(tm.tm_yday, true, transitionHour, transitionMinute);
  transitionTimeOfDay = transitionHour * 60 + transitionMinute;

  tm.tm_hour = transitionHour;
  tm.tm_min = transitionMinute;
  tm.tm_sec = 0;

  if (transitionTimeOfDay < inputTimeOfDay) {
    // This time of day takes place in the next UTC day.
    // tm_yday is used in the Arduino impl above, while mday is used in the c
    // standard library.  Adjust both.
    tm.tm_yday++;
    tm.tm_mday++;
  }

  time_t transition = timegm(&tm);
  secondsUntilTransition = transition - t;
}

/*----------------------------------------------------------------------*
 * This is an implementation of the Sunrise/Sunset Algorithm found at   *
 * http://williams.best.vwh.net/sunrise_sunset_algorithm.htm            *
 * from the Almanac for Computers, 1990                                 *
 * Published by Nautical Almanac Office                                 *
 * Washington, DC 20392                                                 *
 * Implemented by Chris Snyder                                          *
 *                                                                      *
 * Modified 09Dec2011 by Jack Christensen                               *
 *  - Improved rounding of the returned hour and minute values          *
 *    (e.g. would sometimes return 16h60m rather than 17h0m)            *
 *  - Replaced dayNumber() function with ordinalDate() and isLeap().    *
 *    (dayNumber returned zero as the first day for non-leap years.)    *
 *                                                                      *
 *    These changes resulted in better agreement with the US Naval      *
 *    Observatory calculations,                                         *
 *    http://aa.usno.navy.mil/data/docs/RS_OneYear.php                  *
 *                                                                      *
 *    For 2011, for my locale, W083°37', N42°56', all sunrise and       *
 *    sunset times agreed within one minute. 12 sunrise times were one  *
 *    minute later than the USNO time, and 18 earlier. 19 sunset times  *
 *    were one minute later and 2 were earlier.                         *
 *                                                                      *
 * Modified 11Dec2025 by Joey Parrish                                   *
 *  - Combined with fork of TimeLib, adjusted to use struct tm for      *
 *    compatibility with the C standard library time.h routines,        *
 *    removed UTC offset                                                *
 *----------------------------------------------------------------------*/

// Function name: calcSunset
// Parameters:
// doy: The day of the year to calculate sunset/rise for
// lat: The latitude of the location to calculate sunset/rise for
// lon: The longitude of the location to calculate sunset/rise for
// sunset: true to calculate sunset, false to calculate sunrise
// zenith: Sun's zenith for sunrise/sunset
//         offical      = 90 degrees 50'  (90.8333)
//         civil        = 96 degrees
//         nautical     = 102 degrees
//         astronomical = 108 degrees
// hourOut, minutesOut: Return values, time of sunrise or sunset
//
// Note: longitude is positive for East and negative for West
//       latitude is positive for North and negative for south

void SunriseTimer::calcSunset(int doy, bool sunset,
                              uint8_t& hourOut, uint8_t& minutesOut)
{
    hourOut = minutesOut = 0;

    // Convert the longitude to hour value and calculate an approximate time.
    float lonhour = (m_lon / 15);

    float t;
    if (sunset)
        t = doy + ((18 - lonhour) / 24);
    else
        t = doy + ((6 - lonhour) / 24);

    // Calculate the Sun's mean anomaly
    float m = (0.9856 * t) - 3.289;

    // Calculate the Sun's true longitude
    float sinm = sin(deg2rad(m));
    float sin2m = sin(2 * deg2rad(m));
    float l= AdjustTo360 (m + (1.916 * sinm) + (0.02 * sin2m) + 282.634);

    // Calculate the Sun's right ascension(RA)
    float tanl = 0.91764 * tan(deg2rad(l));
    float ra = AdjustTo360 (rad2deg(atan(tanl)));

    // Putting the RA value into the same quadrant as L
    float lq = (floor(l / 90)) * 90;
    float raq = (floor(ra / 90)) * 90;
    ra = ra + (lq - raq);

    // Convert RA values to hours
    ra /= 15;

    // Calculate the Sun's declination
    float sindec = 0.39782 * sin(deg2rad(l));
    float cosdec = cos(asin(sindec));

    // Calculate the Sun's local hour angle
    float cosh = (cos(deg2rad(m_zenith)) - (sindec * sin(deg2rad(m_lat))))
        / (cosdec * cos(deg2rad(m_lat)));

    // if cosH > 1 the sun never rises on this date at this location
    // if cosH < -1 the sun never sets on this date at this location
    if (cosh >  1)
        return;
    else if (cosh < -1)
        return;

    // Finish calculating H and convert into hours
    float h;
    if (sunset)
        h = rad2deg(acos(cosh));
    else
        h = 360 - rad2deg(acos(cosh));

    h /= 15;

    // Calculate local mean time of rising/setting
    t = h + ra - (0.06571 * t) - 6.622;

    // Adjust back to UTC
    float ut = AdjustTo24(t - lonhour);

    hourOut = floor(ut);
    // rounded above, so letting the float-to-int assignment truncate is OK -- jc
    // was: minutesOut = round(60 * (ut - hour));
    minutesOut = 60.0 * (ut - hourOut);
}

float SunriseTimer::AdjustTo360(float i)
{
    if (i > 360.0)
        i -= 360.0;
    else if (i < 0.0)
        i += 360.0;
    return i;
}

float SunriseTimer::AdjustTo24(float i)
{
    if (i > 24.0)
        i -= 24.0;
    else if (i < 0.0)
        i += 24.0;
    return i;
}

float SunriseTimer::deg2rad(float degrees)
{
    return degrees * PI / 180.0;
}

float SunriseTimer::rad2deg(float radians)
{
    return radians / (PI / 180.0);
}
