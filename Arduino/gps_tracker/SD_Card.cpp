#include "SD_Card.h"

#define FILE_NAME_CARD "[SD_Card.cpp]"

// Constructor: MISO, CS, SCK, MOSI, CD
SD_Card::SD_Card(pin_size_t miso, pin_size_t cs, pin_size_t sck, pin_size_t mosi, pin_size_t cd) {
  _miso = miso;
  _cs = cs;
  _sck = sck;
  _mosi = mosi;
  _cd = cd;
}

// *** Public ***
bool SD_Card::begin() {
  // Ensure the SPI pinout the SD card is connected to is configured properly
  SPI1.setRX(_miso);
  SPI1.setTX(_mosi);
  SPI1.setSCK(_sck);

  // If there is a Card Detect Pin, set the cardPresent flag based on the value of the pin
  if (_cd != NO_PIN) {
    pinMode(_cd, INPUT);
  }

  clearConfig();
  // Call isCardPresent to do the following
  //  - Set the cardPresent flag
  //  - Call SD.begin
  //  - Load the configuration file if an SD Card is present
  isCardPresent();

  // FSInfo fs_info;
  SDFS.info(fs_info);

  updateCardInfo();

  return cardPresent;
}

void SD_Card::clearConfig() {
  sdCardConfig.gmtOffset = 0;
  strlcpy(sdCardConfig.tzAbbr, "GMT", 4);
  sdCardConfig.metric = true;
  sdCardConfig.displayOffSecs = 15;

  printConfig();
}

bool SD_Card::deleteFile(const char *fileFullName) {
  if (!cardPresent) {
    return false;
  }

  if (fileExists(fileFullName)) {
    SD.remove(fileFullName);
    return true;
  }
  return false;
}

bool SD_Card::fileExists(const char *fileFullName) {
  if (!cardPresent) {
    return false;
  }

  bool fileExists = SD.exists(fileFullName);
  return fileExists;
}

bool SD_Card::isCardPresent() {
  bool last_cardPresent = cardPresent;
  cardPresent = true;

  // If card detect pin is present, use it to determine if there is a sd card.
  if (_cd != NO_PIN) {
    cardPresent = digitalRead(_cd);
    // If there is no card, call SD.end.
    if (!cardPresent) {
      SD.end();
    }
  } else {
    SD.end();
    cardPresent = SD.begin(_cs, SPI1);
  }

  // Attempt to reload the config file if the card was just inserted
  if (cardPresent && !last_cardPresent) {
    SD.begin(_cs, SPI1);
    DEBUGV("%s isCardPresent - START: Reading Config File\n", FILE_NAME_CARD);
    loadConfig();
    printConfig();
    DEBUGV("%s isCardPresent - FINISHED: Reading Config File\n", FILE_NAME_CARD);
  }

  return cardPresent;
}

bool SD_Card::loadConfig(const char *fileFullName) {
  if (strlen(fileFullName) > 0) {
    strlcpy(sdCardConfigFullName, fileFullName, sizeof(sdCardConfigFullName));
  }

  // Reset to default values
  clearConfig();

  if (!fileExists(sdCardConfigFullName)) {
    Serial.printf("%s loadConfig - return false 0 sdCardConfigFullName: %s (FILE NOT FOUND)\n", FILE_NAME_CARD, sdCardConfigFullName);
    return false;
  }

  File f = readFile(sdCardConfigFullName);

  if (!f) {
    Serial.printf("%s loadConfig - return false 1 sdCardConfigFullName: %s (FILE READ FAILED)\n", FILE_NAME_CARD, sdCardConfigFullName);
    return false;
  }

  /*
    https://arduinojson.org/v6/assistant/#/step1
      Data structures	48	Bytes needed to stores the JSON objects and arrays in memory 
      Strings	28	Bytes needed to stores the strings in memory 
      Total (minimum)	76	Minimum capacity for the JsonDocument.
      Total (recommended)	96	Including some slack in case the strings change, and rounded to a power of two
  */
  const int config_json_capacity = 120;  // JSON_OBJECT_SIZE(5);

  StaticJsonDocument<config_json_capacity> doc;

  // Deserialize the JSON document
  DeserializationError error = deserializeJson(doc, f);
  if (error) {
    DEBUGV("%s loadConfig - return false 2 ERROR: %s\n", FILE_NAME_CARD, error);
    Serial.printf("%s loadConfig - return false 2 ERROR: %s\n", FILE_NAME_CARD, error);
    return false;
  } else {
    sdCardConfig.gmtOffset = doc["gmtOffset"];
    strlcpy(sdCardConfig.tzAbbr, doc["tzAbbr"], sizeof(sdCardConfig.tzAbbr));
    sdCardConfig.metric = doc["metric"];
    sdCardConfig.displayOffSecs = doc["displayOffSecs"];
  }

  f.close();

  return true;
}

void SD_Card::printConfig() {
  Config *config = &sdCardConfig;
  int cnt = 0;

  Serial.printf("%s printConfig - GMT Offset = %d\n", FILE_NAME_CARD, config->gmtOffset);
  Serial.printf("%s printConfig - TZ Abbreviation = %s\n", FILE_NAME_CARD, config->tzAbbr);
  Serial.printf("%s printConfig - Metric = %s\n", FILE_NAME_CARD, config->metric ? "Yes" : "No");
  Serial.printf("%s printConfig - Display Off Seconds = %d\n", FILE_NAME_CARD, config->displayOffSecs);
}

File SD_Card::readFile(const char *fileFullName) {
  if (!cardPresent) {
    return FileImplPtr();
  }

  return SD.open(fileFullName, FILE_READ);
}

bool SD_Card::renameFile(const char *fromFileFullName, const char *toFileFullName) {
  if (!cardPresent) {
    return false;
  }

  if (fileExists(toFileFullName)) {
    deleteFile(toFileFullName);
  }

  if (fileExists(fromFileFullName)) {
    SD.rename(fromFileFullName, toFileFullName);
    return true;
  }

  return false;
}

bool SD_Card::saveConfig() {
  
  return true;
}

void SD_Card::updateCardInfo(bool updateCardPresent) {
  cardInfo.cardInserted = false;
  cardInfo.totalKBytes = 0;
  cardInfo.usedKBytes = 0;
  cardInfo.fileCount = 0;
  strlcpy(cardInfo.cardType, "\0", sizeof(cardInfo.cardType));
  strlcpy(cardInfo.totalSize, "\0", sizeof(cardInfo.totalSize));
  strlcpy(cardInfo.usedSize, "\0", sizeof(cardInfo.usedSize));
  strlcpy(cardInfo.freeSize, "\0", sizeof(cardInfo.freeSize));

  if (updateCardPresent) {
    cardInfo.cardInserted = isCardPresent();
  } else {
    cardInfo.cardInserted = cardPresent;
  }

  if (!cardInfo.cardInserted) {
    return;
  }

  // // Assign Total and Used Bytes
  cardInfo.usedKBytes = fs_info.usedBytes / 1024.0f;
  cardInfo.totalKBytes = SD.totalClusters() * (SD.clusterSize() / 1024.0f);

  // Update file count once per minute
  if (!fileCountInit || lastUpdateCount >= 240) {
    // Assign File Count
    cardInfo.fileCount = fileCount();
    lastFileCount = cardInfo.fileCount;
    lastUpdateCount = 0;
    fileCountInit = true;
  } else {
    cardInfo.fileCount = lastFileCount;
  }

  // Assign Card Type
  switch (SD.type()) {
    case 0:
      strlcpy(cardInfo.cardType, "SD1", 15);
      break;
    case 1:
      strlcpy(cardInfo.cardType, "SD2", 15);
      break;
    case 3:
      strlcpy(cardInfo.cardType, "SDHC/SDXC", 15);
      break;
    default:
      strlcpy(cardInfo.cardType, "Unknown", 15);
  }

  // Assign formatted sizes
  formatedSizeString(cardInfo.totalSize, cardInfo.totalKBytes, sizeof(cardInfo.totalSize));
  formatedSizeString(cardInfo.usedSize, cardInfo.usedKBytes, sizeof(cardInfo.usedSize));
  formatedSizeString(cardInfo.freeSize, cardInfo.totalKBytes - cardInfo.usedKBytes, sizeof(cardInfo.freeSize));
}

bool SD_Card::writeLogEntry(const char *fileFullName, const char *message) {
  if (!cardPresent) {
    return false;
  }

  File logFile = SD.open(fileFullName, FILE_WRITE);
  logFile.println(message);
  logFile.close();

  return true;
}


// *** Private ***
int SD_Card::countFiles(File dir) {
  int count = 0;

  while (true) {
    File entry = dir.openNextFile();
    if (!entry) {
      // no more files or directories
      return count;
    }

    if (entry.isDirectory()) {
      count += countFiles(entry);
    } else {
      count++;
    }
    entry.close();
  }

  return count;
}

int SD_Card::fileCount() {
  if (!isCardPresent()) {
    return 0;
  }

  File root = SD.open("/");
  return countFiles(root);
}

void SD_Card::formatedSizeString(char *buffer, size_t kBytes, int maxLen) {
  float size = (float)kBytes;
  if (kBytes < 1024) {
    snprintf(buffer, maxLen, "%8d Kb", kBytes);
    return;
  }
  size = size / 1024.0f;
  if (size < 1024) {
    snprintf(buffer, maxLen, "%8.3f Mb", size);
    return;
  }
  size = size / 1024.0f;
  if (size < 1024) {
    snprintf(buffer, maxLen, "%8.3f Gb", size);
    return;
  }
  size = size / 1024.0f;
  snprintf(buffer, maxLen, "%8.3f Tb", size);
}
