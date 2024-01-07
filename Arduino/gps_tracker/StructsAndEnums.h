#ifndef STRUCTSANDENUMS_H
#define STRUCTSANDENUMS_H

/*****************************************************************************
 *                               Include Files                               *
 *****************************************************************************/
#include <Adafruit_GPS.h>
#include <map>
#include <Time.h>

enum class DebugLevels {
  Verbose = 0,
  Info = 1,
  Warning = 2,
  Error = 3
};

enum class BatteryChargeStates {
  Discharging,
  Charging,
  Charged
};

enum class SwitchStates {
  Up = 0,
  Pressed = 1,
  Down = 2,
  Released = 3
};

typedef enum : uint8_t {
  Invalid_FIX = 0,
  GPS_FIX = 1,
  DGPS_FIX = 2
} GPS_FIX_QUALITY;

typedef enum : uint8_t {
  Nofix = 1,
  FIX2D = 2,
  FIX3D = 3
} GPS_FIX_QUALITY_3D;

/*****************************************************************************
 *                                  MAPPINGS                                 *
 *****************************************************************************/
static std::map<DebugLevels, const char*> debugLevelName{
  { DebugLevels::Verbose, "Verbose" },
  { DebugLevels::Info, "Info" },
  { DebugLevels::Warning, "Warning" },
  { DebugLevels::Error, "Error" }
};

// Mapping for string lookup
static std::map<BatteryChargeStates, const char*> batteryChargeStateName{
  { BatteryChargeStates::Discharging, "Discharging" },
  { BatteryChargeStates::Charging, "Charging" },
  { BatteryChargeStates::Charged, "Charged" }
};

static std::map<SwitchStates, const char*> switchStateName{
  { SwitchStates::Up, "Up" },
  { SwitchStates::Pressed, "Pressed" },
  { SwitchStates::Down, "Down" },
  { SwitchStates::Released, "Released" }
};

static std::map<GPS_FIX_QUALITY, const char*> gpsFixQualityName{
  { GPS_FIX_QUALITY::Invalid_FIX, "Invalid" },
  { GPS_FIX_QUALITY::GPS_FIX, "GPS" },
  { GPS_FIX_QUALITY::DGPS_FIX, "DGPS" }
};

static std::map<GPS_FIX_QUALITY_3D, const char*> gpsFixQuality3dName{
  { GPS_FIX_QUALITY_3D::Nofix, "No fix" },
  { GPS_FIX_QUALITY_3D::FIX2D, "2D" },
  { GPS_FIX_QUALITY_3D::FIX3D, "3D" }
};

static std::map<int, const char*> dayAbbr{
  { 1, "SUN" },
  { 2, "MON" },
  { 3, "TUE" },
  { 4, "WED" },
  { 5, "THU" },
  { 6, "FRI" },
  { 7, "SAT" }
};

static std::map<int, const char*> monthAbbr{
  { 1, "JAN" },
  { 2, "FEB" },
  { 3, "MAR" },
  { 4, "APR" },
  { 5, "MAY" },
  { 6, "JUN" },
  { 7, "JUL" },
  { 8, "AUG" },
  { 9, "SEP" },
  { 10, "OCT" },
  { 11, "NOV" },
  { 12, "DEC" }
};

struct CardInfo {
  bool cardInserted;
  // unsigned long totalKBytes;
  // unsigned long usedKBytes;
  size_t totalKBytes;
  size_t usedKBytes;
  int fileCount;
  char cardType[16];
  char totalSize[16];
  char usedSize[16];
  char freeSize[16];
};  // CardInfo;

struct GpsInfo {
  uint8_t hour;                      ///< GMT hours
  uint8_t minute;                    ///< GMT minutes
  uint8_t seconds;                   ///< GMT seconds
  uint8_t year;                      ///< GMT year
  uint8_t month;                     ///< GMT month
  uint8_t day;                       ///< GMT day
  nmea_float_t latitudeDegrees;      ///< Latitude in decimal degrees
  nmea_float_t longitudeDegrees;     ///< Longitude in decimal degrees
  nmea_float_t altitude;             ///< Altitude in meters above MSL
  nmea_float_t speed;                ///< Current speed over ground in knots
  nmea_float_t angle;                ///< Course in degrees from true north
  GPS_FIX_QUALITY fixquality;        ///< Fix quality (0, 1, 2 = Invalid, GPS, DGPS)
  GPS_FIX_QUALITY_3D fixquality_3d;  ///< 3D fix quality (1, 3, 3 = Nofix, 2D fix, 3D fix)
  uint8_t satellites;                ///< Number of satellites in use
  bool fix;                          ///< Have a fix?
  time_t epoch;                      // Unix Time
  char isoTime[22];                  // ISO 8601 formatted timestamp
  char utc_date[11];                // UTC Date
  char utc_time[11];                // UTC Time
};                                   // GpsInfo

struct Log_Entry {
  char message[512];
  char logfile[128];
};  // logEntry;

struct Switch {
  int index;
  int pin;
  SwitchStates state;
  int last;
  int current;
};  // Switch;


#endif  // STRUCTSANDENUMS_H