#include "Line_Display.h"

/******** PUBLIC ********/
void LineDisplay::begin(const DevType* dev, uint8_t i2cAddr, int rst, const uint8_t* font, int labelCount, Label &labels) {
  Wire.begin();
  Wire.setClock(400000L);

  labelArrayLen = labelCount;
  labelArrPtr = &labels;

#if rst >= 0
  display.begin(dev, i2cAddr, rst);
#else // RST_PIN >= 0
  display.begin(dev, i2cAddr);
#endif // RST_PIN >= 0

  display.setFont(font);

  if (display.fontRows() > 0)
    displayLines = display.displayRows() / display.fontRows();
  if ((display.fontWidth() + display.letterSpacing()) > 0)
    displayLineChars = display.displayWidth() / (display.fontWidth() + display.letterSpacing());

  screenCount = (int)(((float)labelCount + (float)displayLines - 1) / (float)displayLines);
}

void LineDisplay::changeScreen(int screenNumber) {
  if (screenNumber >= screenCount)
    screenNumber = 0;
  if (screenNumber < 0)
    screenNumber = screenCount - 1;

  currentScreen = screenNumber;

  display.clear();

  for (int i = 0; i < displayLines; i++) {
    //printLabel(labelArrPtr[(currentScreen * displayLines) + i]);
    printLabel((currentScreen * displayLines) + i);
  }
}

void LineDisplay::incrementScreen(int incValue) {
  changeScreen(currentScreen + incValue);
}

void LineDisplay::updateField(uint8_t labelIndex, const char strVal[]) {
  if (labelIndex >= labelArrayLen)
    return;

  // Is the field being displayed on the current screen?
  if (labelIndex / displayLines == currentScreen) {
    if (labelArrPtr[labelIndex].width >= displayLineChars)
      return;

    //int fieldWidth = display.fieldWidth(labelArrPtr[labelIndex].width);
    int fieldWidth = display.strWidth(labelArrPtr[labelIndex].text);

    //display.clearField(fieldWidth, (labelIndex % displayLines) * display.fontRows(), displayLineChars - labelArrPtr[labelIndex].width);
    display.setCol(fieldWidth);
    display.setRow((labelIndex % displayLines) * display.fontRows());
    display.clearToEOL();
    display.print(strVal);
  }
}


/******** PRIVATE ********/

void LineDisplay::printLabel(uint8_t labelIndex) {
  if (labelIndex >= labelArrayLen)
    return;

  const Label &label = labelArrPtr[labelIndex];

  if (label.inverse)
    display.setInvertMode(true);

  display.println(label.text);

  display.setInvertMode(false);
}
