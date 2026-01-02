# Arduino Sun Timer Library

<https://github.com/joeyparrish/ArduinoSunTimer>


A library to calculate the phase of the day, and the time until the next
phase in seconds.  Calculated based on the current time (epoch time, seconds
since 1970 UTC) for a given latitude and longitude.

All times are given in UTC, so there is no need to configure a timezone offset.
This can be connected directly to a GPS, which knows the time in UTC and your
latitude and longitude.

Also runs on Linux for testing and development, and for comparison to the
Arduino output.


## License

Arduino Sun Timer Library Copyright (C) 2025 Joey Parrish under GNU GPL v3.0

This program is free software: you can redistribute it and/or modify it under
the terms of the GNU General Public License v3.0 as published by the Free
Software Foundation.

This program is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
this program. If not, see <https://www.gnu.org/licenses/gpl.html>


## Dependencies

None!


## Motivation

The original version of this library from
<https://github.com/JChristensen/JC_Sunrise> was lacking in a couple key ways:

 - Oriented around local times, making it confusing to use in UTC
 - Oriented around time of day (hour & minute), rather than `time_t`
 - Methods that output `time_t` were broken when using anything other than UTC
 - Methods that output `time_t` were broken also when using UTC, since they
   didn't handle edge cases and were often off by a day
 - Couldn't be used directly on Linux for comparison or verification

This fork addresses all of these issues.  Finally, I changed the API more
radically to make it more useful for my own plans, which involve putting a
device to sleep until the next transition point.


## Example Sketch

For a fully-feature example sketch, see
[examples/Demo/Demo.ino](examples/Demo/Demo.ino).

You can also run `make` on Linux to build locally with GCC.


## Usage


### Constructor

`SunTimer(lat, lon)`

Define a sun timer for a particular location and definition of sunrise.

 - **lat:** Latitude *(float)*
 - **lon:** Longitude *(float)*

#### Example

```c++
#include <SunTimer.h>

// ...

  constexpr float myLat {45.8171}, myLon {-84.7278};
  SunTimer myLocation {myLat, myLon};
```


### calculate

`void calculate(time_t time, phase_t* currentPhase, int32_t* secondsUntilNextPhase)`

Calculate what phase we're currently in, and how long until the next transition.

 - **time:** Current Unix epoch time (seconds since 00:00, January 1, 1970, UTC).
 - **currentPhase:** Output variable, the current phase of the day (e.g.
     `ASTRONOMICAL_TWILIGHT_MORNING`,
     `NAUTICAL_TWILIGHT_MORNING`,
     `CIVIL_TWILIGHT_MORNING`,
     `DAY`,
     `CIVIL_TWILIGHT_EVENING`,
     `NAUTICAL_TWILIGHT_EVENING`,
     `ASTRONOMICAL_TWILIGHT_EVENING`,
     or `NIGHT`)
 - **secondsUntilNextPhase:** Output variable, seconds until the next
     phase begins.

#### Example

```c++
#include <SunTimer.h>

// ...

  constexpr float myLat {45.8171}, myLon {-84.7278};
  SunTimer myLocation {myLat, myLon};

  // Construct a fake time: midnight, January 1, 2022.
  struct tm tm;
  tm.tm_year = 2022 - 1900;  // 1900-based
  tm.tm_mon = 0;  // 0-based
  tm.tm_mday = 1; // 1-based
  tm.tm_yday = 0; // 0-based
  tm.tm_hour = 0; // 0-based
  tm.tm_min = 0;  // 0-based
  tm.tm_sec = 0;  // 0-based

  // Convert to Unix epoch time.
  time_t now = timegm(&tm);

  // Compute the state of the sun and time until transition.
  SunTimer::phase_t phase;
  int32_t secondsUntilNextPhase;
  myLocation.calculate(now, &phase, &secondsUntilNextPhase);

#ifdef ARDUINO
  Serial.print("The phase is ");
  Serial.println(SunTimer::phase_name(phase));
#else
  printf("The phase is %s\n", SunTimer::phase_name(phase));
#endif

#ifdef ARDUINO
  Serial.print("Transition time is in ");
  Serial.print(secondsUntilNextPhase);
  Serial.println("seconds.");
#else
  printf("Transition time is in %ld seconds.\n", (long)secondsUntilNextPhase);
#endif
```
