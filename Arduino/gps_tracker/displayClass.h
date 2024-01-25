#ifndef DISPLAYCLASS_H
#define DISPLAYCLASS_H

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include "Defines.h"
#include "screens.h"
#include "teelsys.h"

TwoWire myWire = TwoWire(i2c1, PIN_OLED_SDA, PIN_OLED_SCL);
Adafruit_SSD1306 oledDisplay(SCREEN_WIDTH, SCREEN_HEIGHT, &myWire, OLED_RESET);

class displayClass {

public:

  bool displayOn = true;

  bool begin() {
    initialized = false;
    if (!oledDisplay.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
      Serial.println(F("SSD1306 allocation failed"));
      return initialized;
    } else {
      initialized = true;
    }
    // Show initial display buffer contents on the screen --
    // the library initializes this with an Adafruit splash screen.
    // oledDisplay.display();
    showLogo();
    delay(2000);  // Pause for 2 seconds
    // Clear the buffer
    oledDisplay.clearDisplay();

    return initialized;
  }

  void showLogo() {
    oledDisplay.clearDisplay();

    oledDisplay.drawBitmap(
      (oledDisplay.width() - LOGO_WIDTH) / 2,
      (oledDisplay.height() - LOGO_HEIGHT) / 2,
      teelsys, LOGO_WIDTH, LOGO_HEIGHT, 1);
    oledDisplay.display();
  }

  const int lineHeight = 12;
  void showScreen(Screen screen) {
    if (!initialized) {
      return;
    }

    oledDisplay.clearDisplay();

    oledDisplay.setTextSize(1);               // Normal 1:1 pixel scale
    oledDisplay.setTextColor(SSD1306_WHITE);  // Draw white text
    oledDisplay.cp437(true);                  // Use full 256 char 'Code Page 437' font
    oledDisplay.setCursor(0, 0);              // Start at top-left corner
    oledDisplay.write(screen.line1);
    oledDisplay.setCursor(0, lineHeight);
    oledDisplay.write(screen.line2);
    oledDisplay.setCursor(0, lineHeight * 2);
    oledDisplay.write(screen.line3);
    oledDisplay.setCursor(0, lineHeight * 3);
    oledDisplay.write(screen.line4);
    oledDisplay.setCursor(0, lineHeight * 4);
    oledDisplay.write(screen.line5);

    oledDisplay.display();
  }

  void sleepDisplay() {
    oledDisplay.ssd1306_command(SSD1306_DISPLAYOFF);
  }

  void wakeDisplay() {
    oledDisplay.ssd1306_command(SSD1306_DISPLAYON);
  }

  void toggleDisplayOnOff(bool forceOn = false, bool forceOff = false) {
    displayOn = !displayOn;

    if(forceOn) {
      displayOn = true;
    } else if(forceOff) {
      displayOn = false;
    }

    Serial.print("toggleDisplayOnOff: Turn ");
    Serial.println(displayOn ? "On" : "Off");
    if (displayOn) {
      wakeDisplay();
    } else {
      sleepDisplay();
    }
  }


private:
  bool initialized = false;
};


#endif  // DISPLAYCLASS_H