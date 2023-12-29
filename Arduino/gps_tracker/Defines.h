#ifndef DEFINES_H
#define DEFINES_H

// ***** Pins *****
#define NO_PIN 255

//-- GPS --
#define PIN_GPS_CTRL 6
#define PIN_GPS_TX 0
#define PIN_GPS_RX 1
//-- Switches --
#define PIN_SW0 19
#define PIN_SW1 18
#define PIN_SW2 25
#define PIN_SW3 24
//-- Battery Monitoring --
#define PIN_BAT_V A0
#define PIN_USB_D 7
//-- OLED Display --
#define PIN_OLED_SCL 3
#define PIN_OLED_SDA 2
//-- SD Card --
#define PIN_SD_CD 8
#define PIN_SD_CS 9
#define PIN_SD_SCLK 10
#define PIN_SD_DI 11  // MOSI
#define PIN_SD_DO 12  // MISO
//-- Feather --
#define PIN_FEATHER_LED 13
#define PIN_FEATHER_NEO 16

// Screen
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1        // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C  ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32

#endif  // DEFINES_H