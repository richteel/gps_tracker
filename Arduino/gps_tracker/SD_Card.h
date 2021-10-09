#ifndef __SD_CARD_H__
#define __SD_CARD_H__

#include <SD.h>
#include <SPI.h>
#include <TimeLib.h>

struct SDCardInfo {
  bool cardPresent = false;
  bool cardInitialized = false;
  float cardSize = 0;
  char cardSizeUnits[8] = {0x00};
  char cardType[12] = {0x00};
  unsigned long cardFileCount  = 0;
  uint32_t fatSize = 0;
};

class SDCard {
  public:
    SDCardInfo cardInfo;
    bool requestReset = false;
    bool lastCardState = false;
    bool cardState = false;

    bool begin(uint8_t csPin, uint8_t cdPin);
    bool CardPresent();
    void UpdateSDInfo();


  private:
    uint8_t cardDetectPin = 0;
    uint8_t chipSelectPin = 0;

    bool cardInit();
    void clearCardInfo();
    void updateCardSize(SdVolume &volume);
    void updateCardType(Sd2Card &card);
    void updateFatSize(SdVolume &volume);
    void updateFileCount();
};




#endif
