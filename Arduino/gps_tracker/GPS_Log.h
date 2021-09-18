#ifndef GPS_LOG_H
#define GPS_LOG_H

#include <NMEAGPS.h>
#include <SD.h>
#include <SPI.h>
#include <TimeLib.h>
#include "SD_Card.h"

class GPS_Log {
  public:
    void begin(const char header[], SDCard &sdcard);

    void logGPS(gps_fix &fix);

  private:
    const char *logHeader;
    SDCard *sdCard;
    unsigned long lastDate = 0;
    char filename[13] = {0x00};

    void checkHeader();
    bool updateFileName(gps_fix &fix);

};



#endif
