#ifndef TIMEADJUST_H
#define TIMEADJUST_H

#include <NMEAGPS.h>

//======================================================================
//  Program: NMEAtimezone.ino
//
//  Description:  This program shows how to offset the GPS dateTime member
//          into your specific timezone.  GPS devices do not know which
//          timezone they are in, so they always report a UTC time.  This
//          is the same as GMT.
//
//  Prerequisites:
//     1) NMEA.ino works with your device
//     2) GPS_FIX_TIME is enabled in GPSfix_cfg.h
//     3) NMEAGPS_PARSE_RMC is enabled in NMEAGPS_cfg.h.  You could use
//        any sentence that contains a time field.  Be sure to change the
//        "if" statement in GPSloop from RMC to your selected sentence.
//
//  'Serial' is for debug output to the Serial Monitor window.
//
//  License:
//    Copyright (C) 2014-2017, SlashDevin
//
//    This file is part of NeoGPS
//
//    NeoGPS is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    NeoGPS is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with NeoGPS.  If not, see <http://www.gnu.org/licenses/>.
//
//======================================================================

//--------------------------
// CHECK CONFIGURATION

#if !defined(GPS_FIX_TIME) | !defined(GPS_FIX_DATE)
#error You must define GPS_FIX_TIME and DATE in GPSfix_cfg.h!
#endif

#if !defined(NMEAGPS_PARSE_RMC) & !defined(NMEAGPS_PARSE_ZDA)
#error You must define NMEAGPS_PARSE_RMC or ZDA in NMEAGPS_cfg.h!
#endif

#include <GPSport.h>

//--------------------------
// Set these values to the offset of your timezone from GMT

static const int32_t          zone_hours   = -5L; // EST
static const int32_t          zone_minutes =  0L; // usually zero
static const NeoGPS::clock_t  zone_offset  =
  zone_hours   * NeoGPS::SECONDS_PER_HOUR +
  zone_minutes * NeoGPS::SECONDS_PER_MINUTE;

// Uncomment one DST changeover rule, or define your own:
#define USA_DST
//#define EU_DST

#if defined(USA_DST)
static const uint8_t springMonth =  3;
static const uint8_t springDate  = 14; // latest 2nd Sunday
static const uint8_t springHour  =  2;
static const uint8_t fallMonth   = 11;
static const uint8_t fallDate    =  7; // latest 1st Sunday
static const uint8_t fallHour    =  2;
#define CALCULATE_DST

#elif defined(EU_DST)
static const uint8_t springMonth =  3;
static const uint8_t springDate  = 31; // latest last Sunday
static const uint8_t springHour  =  1;
static const uint8_t fallMonth   = 10;
static const uint8_t fallDate    = 31; // latest last Sunday
static const uint8_t fallHour    =  1;
#define CALCULATE_DST
#endif

//--------------------------

void adjustTime( NeoGPS::time_t & dt )
{
  NeoGPS::clock_t seconds = dt; // convert date/time structure to seconds

#ifdef CALCULATE_DST
  //  Calculate DST changeover times once per reset and year!
  static NeoGPS::time_t  changeover;
  static NeoGPS::clock_t springForward, fallBack;

  if ((springForward == 0) || (changeover.year != dt.year)) {

    //  Calculate the spring changeover time (seconds)
    changeover.year    = dt.year;
    changeover.month   = springMonth;
    changeover.date    = springDate;
    changeover.hours   = springHour;
    changeover.minutes = 0;
    changeover.seconds = 0;
    changeover.set_day();
    // Step back to a Sunday, if day != SUNDAY
    changeover.date -= (changeover.day - NeoGPS::time_t::SUNDAY);
    springForward = (NeoGPS::clock_t) changeover;

    //  Calculate the fall changeover time (seconds)
    changeover.month   = fallMonth;
    changeover.date    = fallDate;
    changeover.hours   = fallHour - 1; // to account for the "apparent" DST +1
    changeover.set_day();
    // Step back to a Sunday, if day != SUNDAY
    changeover.date -= (changeover.day - NeoGPS::time_t::SUNDAY);
    fallBack = (NeoGPS::clock_t) changeover;
  }
#endif

  //  First, offset from UTC to the local timezone
  seconds += zone_offset;

#ifdef CALCULATE_DST
  //  Then add an hour if DST is in effect
  if ((springForward <= seconds) && (seconds < fallBack))
    seconds += NeoGPS::SECONDS_PER_HOUR;
#endif

  dt = seconds; // convert seconds back to a date/time structure

} // adjustTime

//--------------------------

#ifdef NMEAGPS_INTERRUPT_PROCESSING
static void GPSisr( uint8_t c )
{
  gps.handle( c );
}
#endif

#endif
