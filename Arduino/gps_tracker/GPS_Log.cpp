#include "GPS_Log.h"

/******** PUBLIC ********/

void GPS_Log::begin(const char header[], SDCard &sdcard) {
  logHeader = &header[0];
  sdCard = &sdcard;
  if (Serial) {
    Serial.print("begin Header = ");
    Serial.println(header);
  }
}

void GPS_Log::logGPS(gps_fix &fix) {
  if (!sdCard->CardPresent()) {
    Serial.println("RETURN");
    return;
  }

  // Does the file already exist?
  bool fileExists = updateFileName(fix);

  // open the file.
  File myFile = SD.open(filename, FILE_WRITE);

  if (!fileExists) {
    if (Serial) {
      Serial.print(F("Created log file: "));
      Serial.println(filename);
    }
    myFile.println(F("Date/Time\tLat\tLong\tAlt\tHeading\tSpeed\tSatellites"));
  }

  // Date/Time
  if (fix.valid.time && fix.valid.date) {
    myFile.print(fix.dateTime.month);
    myFile.print('-');
    myFile.print(fix.dateTime.date);
    myFile.print('-');
    myFile.print(fix.dateTime.full_year());
    myFile.print(' ');
    if (fix.dateTime.hours < 10) myFile.print('0');
    myFile.print(fix.dateTime.hours);
    myFile.print(':');
    if (fix.dateTime.minutes < 10) myFile.print('0');
    myFile.print(fix.dateTime.minutes);
    myFile.print(':');
    if (fix.dateTime.seconds < 10) myFile.print('0');
    myFile.print(fix.dateTime.seconds);
  }
  myFile.print('\t');

  // Latitude
  if (fix.valid.location) {
    myFile.print(fix.latitude(), 6 );
  }
  myFile.print('\t');

  // Longitude
  if (fix.valid.location) {
    myFile.print(fix.longitude(), 6 );
  }
  myFile.print('\t');

  // Altitude
  if (fix.valid.altitude) {
    myFile.print(fix.altitude());
  }
  myFile.print('\t');

  // Heading
  if (fix.valid.heading) {
    myFile.print(fix.heading());
  }
  myFile.print('\t');

  // Speed
  if (fix.valid.speed) {
    myFile.print(fix.speed_kph());
  }
  myFile.print('\t');

  // Satellites
  myFile.println(fix.satellites);

  // close the file:
  myFile.close();
}

/******** PRIVATE ********/

bool GPS_Log::updateFileName(gps_fix &fix) {
  sprintf(filename, "%d%02d%02d.txt", year(), month(), day());

  return SD.exists(filename);
}
