#ifndef SD_CARD_H
#define SD_CARD_H

#include <SPI.h>
#include <SD.h>
#include <ArduinoJson.h>
#include "config.h"
#include "Defines.h"
#include "StructsAndEnums.h"
#include <map>

class SD_Card {
public:
  // Constructor: MISO, CS, SCK, MOSI, CD
  SD_Card(pin_size_t miso = PIN_SD_DO, pin_size_t cs = PIN_SD_CS, pin_size_t sck = PIN_SD_SCLK, pin_size_t mosi = PIN_SD_DI, pin_size_t cd = PIN_SD_CD);

  bool begin();

  void clearConfig();

  bool deleteFile(const char *fileFullName);

  bool isCardPresent();

  bool fileExists(const char *fileFullName);

  bool loadConfig(const char *fileFullName = "");

  void printConfig();

  File readFile(const char *fileFullName);

  bool renameFile(const char *fromFileFullName, const char *toFileFullName);

  bool saveConfig();

  void updateCardInfo(bool updateCardPresent = false);

  bool writeLogEntry(const char *fileFullName = "gps.log", const char *message = "");

  bool cardPresent = false;
  char sdCardConfigFullName[128] = "config.txt";
  Config sdCardConfig = Config();
  CardInfo cardInfo;

private:
  pin_size_t _miso, _cs, _sck, _mosi, _cd;

  // Used for updateCardInfo to update file count about once per minute
  int lastFileCount = 0;
  int lastUpdateCount = 0;
  bool fileCountInit = false;

  FSInfo fs_info;

  int countFiles(File dir);

  int fileCount();

  void formatedSizeString(char *buffer, size_t bytes, int maxLen = 15);
};


#endif  // SD_CARD_H