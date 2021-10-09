#ifndef __LINE_DISPLAY_H__
#define __LINE_DISPLAY_H__

#include <Wire.h>
#include "SSD1306Ascii.h"
#include "SSD1306AsciiWire.h"


typedef struct {
  const char * const text;
  int width;
  bool inverse;
} Label;


class LineDisplay {
  public:
    SSD1306AsciiWire display;
    uint8_t displayLines = 0;     // Text Lines per screen with current font
    uint8_t displayLineChars = 0; // Characters per line with current font
    uint8_t screenCount = 0;      // Number of screens determined from displayLines and Labels
    uint8_t currentScreen = 0;
    uint8_t labelArrayLen = 0;

    void begin(const DevType* dev, uint8_t i2cAddr, int rst, const uint8_t* font, int labelCount, Label &labels);

    void changeScreen(int screenNumber);
    void incrementScreen(int incValue);
    void updateField(uint8_t labelIndex, const char strVal[]);

  private:
    Label *labelArrPtr;

    void printLabel(uint8_t labelIndex);
};


#endif

/*
  https://forum.arduino.cc/t/struggling-passing-multi-dimensional-arrays-to-class-solved/315241/2
  https://create.arduino.cc/projecthub/LuckyResistor/c-templates-for-embedded-code-95b42b
*/
