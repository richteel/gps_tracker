#include "SD_Card.h"

// call back for file timestamps
void SdDateTime(uint16_t* date, uint16_t* time) {
  // return date using FAT_DATE macro to format fields
  *date = FAT_DATE(year(), month(), day());

  // return time using FAT_TIME macro to format fields
  *time = FAT_TIME(hour(), minute(), second());
}

/******** PUBLIC ********/

bool SDCard::begin(uint8_t csPin, uint8_t cdPin) {
  chipSelectPin = csPin;
  cardDetectPin = cdPin;

  // Set Card Detect to Input
  pinMode(cardDetectPin, INPUT);

  cardInfo.cardInitialized = false;

  SdFile::dateTimeCallback(SdDateTime);

  lastCardState = !CardPresent();
  cardState = lastCardState;

  return cardInit();
}

bool SDCard::CardPresent() {
  if (cardDetectPin > 0)
    return !digitalRead(cardDetectPin);
  else
    return true;
}

void SDCard::UpdateSDInfo() {
  bool lastCardPresent = cardInfo.cardPresent;

  if (requestReset)
    return;

  clearCardInfo();

  if (!cardInfo.cardPresent)
    return;

  if (lastCardPresent != cardInfo.cardPresent && cardInfo.cardInitialized) {
    requestReset = true;
    return;
  }

  if (!cardInfo.cardInitialized) {
    if (!cardInit())
      return;
  }

  Sd2Card card;
  SdVolume volume;

  if (!card.init(SPI_HALF_SPEED, chipSelectPin))
    return;

  updateCardType(card);

  if (!volume.init(card))
    return;

  updateFatSize(volume);

  if (cardInfo.fatSize == 0)
    return;

  updateCardSize(volume);

  updateFileCount();

  // Only call after card.init!!!!
  SD.begin(chipSelectPin);
}


/******** PRIVATE ********/

bool SDCard::cardInit() {
  if (cardInfo.cardInitialized)
    return cardInfo.cardInitialized;

  // Clear card struct infromation
  clearCardInfo();

  if (!cardInfo.cardPresent)
    return cardInfo.cardInitialized;

  // DO NOT CALL BEFORE CARD.INIT!!!!
  //if (!SD.begin(chipSelectPin)) {
  //  return cardInfo.cardInitialized;
  //}

  cardInfo.cardInitialized = true;

  return cardInfo.cardInitialized;
}

void SDCard::clearCardInfo() {
  cardInfo.cardPresent = CardPresent();
  cardInfo.cardSize = 0;
  strcpy(cardInfo.cardSizeUnits, "");
  strcpy(cardInfo.cardType, "");
  cardInfo.cardFileCount = 0;
  cardInfo.fatSize = 0;
}

void SDCard::updateCardSize(SdVolume &volume) {
  uint32_t volumesize;
  volumesize = volume.blocksPerCluster();    // clusters are collections of blocks
  volumesize *= volume.clusterCount();       // we'll have a lot of clusters
  // SD card blocks are always 512 bytes

  if (volumesize > pow(1024, 3) / 512) {
    cardInfo.cardSize  = ((float)volumesize / (float)2) / ((float)pow(1024, 2));
    strcpy(cardInfo.cardSizeUnits, "GB");
  }
  else if (volumesize > pow(1024, 2) / 512) {
    cardInfo.cardSize  = ((float)volumesize / (float)2) / ((float)pow(1024, 1));
    strcpy(cardInfo.cardSizeUnits, "MB");
  }
  else if (volumesize > pow(1024, 1) / 512) {
    cardInfo.cardSize  = ((float)volumesize / (float)2) / ((float)pow(1024, 0));
    strcpy(cardInfo.cardSizeUnits, "KB");
  }
  else {
    cardInfo.cardSize  = (float)volumesize * 512;
    strcpy(cardInfo.cardSizeUnits, "Bytes");
  }
}

void SDCard::updateCardType(Sd2Card &card) {
  switch (card.type()) {
    case SD_CARD_TYPE_SD1:
      strcpy(cardInfo.cardType, "SD1");
      break;
    case SD_CARD_TYPE_SD2:
      strcpy(cardInfo.cardType, "SD2");
      break;
    case SD_CARD_TYPE_SDHC:
      strcpy(cardInfo.cardType, "SDHC");
      break;
    default:
      strcpy(cardInfo.cardType, "Unknown");
      break;
  }
}

void SDCard::updateFatSize(SdVolume &volume) {
  cardInfo.fatSize = volume.fatType();
}

void SDCard::updateFileCount() {
  File root;
  root = SD.open("/");

  while (true) {
    File entry =  root.openNextFile();

    if (! entry) // no more files
      break;

    cardInfo.cardFileCount++;
  }

  root.close();
}
