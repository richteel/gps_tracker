/*
          Hardware Pinouts
  --- Adafruit RP2040 Feather (4884) ---
  https://www.adafruit.com/product/4884
  https://cdn-learn.adafruit.com/assets/assets/000/107/203/original/adafruit_products_feather-rp2040-pins.png?1639162603

  Pin   C   Description
  -- GPS --
  D4    6   GPS PWR_CTRL (6) 0=Off/1=On
  TX    0   GPS RXA (4)
  RX    1   GPS TXA (3)
  -- Switches --
  MOSI  19  SW1   Left
  SCK   18  SW2   Left Middle
  D25   25  SW3   Right Middle
  D24   24  SW4   Right
  -- Battery Monitoring --
  A0    26  Battery Voltage
  D5    7   USB Detect
  -- OLED Display --
  SCL   3   OLED SCL
  SDA   2   OLED SDA
  -- SD Card --
  D6    8   SD Card Card Detect
  D9    0   SD Card CS
  D10   10  SD Card SCLK
  D11   11  SD Card DI
  D12   12  SD Card DO
  -- Feather --
  D13   13  LED
  D16   16  Neopixel  // Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

  OLED Info
  -----------
  I2C Address: 0x3C
  Width: 128
  Height: 64

  GPS Info
  -----------
  BAUD: 9600
  TImeout: 10
*/

/*****************************************************************************
 *                                  DEFINES                                  *
 *****************************************************************************/
#define DEBUG 0
#define PRINTSCREENS 0
#define FILE_NAME "[SpeechTimer.ino]"

/*****************************************************************************
 *                              FreeRTOS Setup                               *
 *****************************************************************************/
#if !defined(ESP_PLATFORM) && !defined(ARDUINO_ARCH_MBED_RP2040) && !defined(ARDUINO_ARCH_RP2040)
#pragma message("Unsupported platform")
#endif

// ESP32:
#if defined(ESP_PLATFORM)
TaskHandle_t task_loop1;
void esploop1(void *pvParameters) {
  setup1();

  for (;;)
    loop1();
}
#endif

#if defined(ARDUINO_ARCH_MBED_RP2040) || defined(ARDUINO_ARCH_RP2040)
#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>
#define xPortGetCoreID get_core_num
#endif

/*****************************************************************************
 *                               Include Files                               *
 *****************************************************************************/
// #include <Dictionary.h>
#include <Adafruit_GPS.h>
#include <TimeLib.h>
#include <Adafruit_NeoPixel.h>

/*****************************************************************************
 *                               Project Files                               *
 *****************************************************************************/
#include "Defines.h"
#include "StructsAndEnums.h"
#include "screens.h"
#include "config.h"
#include "batteryClass.h"
#include "SD_Card.h"
#include "displayClass.h"

/*****************************************************************************
 *                                  DEFINES                                  *
 *****************************************************************************/
// What's the name of the hardware serial port connected to the GPS module?
#define GPSSerial Serial1
// Set GPSECHO to 'false' to turn off echoing the GPS data to the Serial console
// Set to 'true' if you want to debug and listen to the raw GPS sentences
// #define GPSECHO false

/*****************************************************************************
 *                                  GLOBALS                                  *
 *****************************************************************************/
// Flags to make certain that required initialization is complete before tasks start
bool setup_complete = false;
int waitLoopsToPrintTaskInfo = 100;

Switch switches[4];

// Module Objects
// Connect to the GPS on the hardware port
Adafruit_GPS GPS(&GPSSerial);
SD_Card sdCard = SD_Card();
batteryClass battery;
displayClass display;
Adafruit_NeoPixel pixels(1, PIN_FEATHER_NEO, NEO_GRB + NEO_KHZ800);

// GPS Log
time_t gps_last_logged_time;
int gps_log_interval = 10;

// Display
int currentScreenIndex = 0;

// Feather LED and Neopixel

/*****************************************************************************
 *                                 LOG FILES                                 *
 *****************************************************************************/
char logDebug[] = "/logs/debug.log";
char logDebugArchive[] = "/logs/debug_1.log";

/*****************************************************************************
 *                                   QUEUES                                  *
 *****************************************************************************/
static const uint8_t log_queue_len = 10;
static QueueHandle_t log_queue;

static const uint8_t switch_queue_len = 5;
static QueueHandle_t switch_queue;

static const uint8_t gps_queue_len = 5;
static QueueHandle_t gps_queue;

/*****************************************************************************
 *                                   MUTEX                                   *
 *****************************************************************************/
static SemaphoreHandle_t sdcard_mutex;
static SemaphoreHandle_t rtc_mutex;

/*****************************************************************************
 *                               INO FUNCTIONS                               *
 *****************************************************************************/
void debugMessage(const char *message) {
  return;
  // char timeStr[10];
  char msgBuffer[512];
  // clockRtc.getTimeString(timeStr);
  // Format the message
  snprintf(msgBuffer, sizeof(msgBuffer), "%s %s\t%d\t%d\t%d", "sketch_dec21a.ino", message, xPortGetCoreID(), rp2040.getFreeHeap(), uxTaskGetStackHighWaterMark(NULL));

  Serial.println(msgBuffer);
}

void gpsWriteToLog(GpsInfo gpsData) {
  /*
  ISO Time format
    https://en.wikipedia.org/wiki/GPS_Exchange_Format
    2009-10-17T18:37:26Z
    strftime
    %Y-%m-%dT%TZ
    size_t strftime (char* ptr, size_t maxsize, const char* format,                 const struct tm* timeptr );
  */

  // 2023-12-28T02:02:48Z,39.012863,-77.386467,100.199997,288.170013,0.490000
  // 2023-12-28T02:02:04Z,39.012882,-77.386467,100.900002,288.170013,0.439000
  if (sdCard.cardPresent && gpsData.fix && gpsData.epoch - gps_last_logged_time >= gps_log_interval) {
    gps_last_logged_time = gpsData.epoch;

    char filename[80];
    char logBuffer[80];
    snprintf(filename, sizeof(filename), "/gpsdata/%04d%02d%02d.txt", year(gpsData.epoch), gpsData.month, gpsData.day);
    snprintf(logBuffer, sizeof(logBuffer), "%s,%s,%f,%f,%f,%f,%f", gpsData.utc_date, gpsData.utc_time, gpsData.latitudeDegrees, gpsData.longitudeDegrees, gpsData.altitude, gpsData.angle, (gpsData.speed * 1000.852));

    if (xSemaphoreTake(sdcard_mutex, 10) == pdTRUE) {
      // Write file header if file does not exist already
      if (!sdCard.fileExists(filename)) {
        sdCard.writeLogEntry(filename, "utc_d,utc_t,lat,lon,alt,head,speed");
      }
      sdCard.writeLogEntry(filename, logBuffer);
      xSemaphoreGive(sdcard_mutex);
    } else {
      Serial.printf("%s: %s gpsWriteToLog - Failed to write gps entry to the SD Card. (Could not obtain mutex.)\n", debugLevelName[DebugLevels::Error], FILE_NAME);
    }
    // Serial.printf("%s\t%s\n", filename, logBuffer);
  }
}

void ledOnOff(bool turnOn) {
  digitalWrite(PIN_FEATHER_LED, turnOn);
}

void ledToggle() {
  digitalWrite(PIN_FEATHER_LED, !digitalRead(PIN_FEATHER_LED));
}

void neopixelToggle(bool toggleRed, bool toggleBlue, bool toggleGreen) {
  uint32_t c = pixels.getPixelColor(0);
  uint32_t maskToggle = pixels.Color(toggleRed ? 255 : 0, toggleBlue ? 255 : 0, toggleGreen ? 255 : 0);
  uint32_t maskKeep = ~maskToggle;
  uint32_t valueToggle = c & maskToggle;
  uint32_t valueKeep = c & maskKeep;

  if (valueToggle == 0) {
    valueToggle = maskToggle;
  } else {
    valueToggle = 0;
  }

  uint32_t valueNew = valueKeep | valueToggle;

  pixels.setPixelColor(0, valueNew);
  pixels.show();
}

int lastSec = 0;
int secRepeat = 0;

void printScreens() {
  if (!PRINTSCREENS) {
    return;
  }

  int secNow = second(now());
  if (secNow != lastSec) {
    lastSec = secNow;
    secRepeat = 1;
  } else {
    secRepeat++;
  }

  for (int i = 0; i < secRepeat; i++) {
    Serial.print(".");
  }
  Serial.println();


  for (int i = 0; i < NUM_OF_SCREENS; i++) {
    Serial.printf("----- SCREEN %d -----\n", i + 1);
    Serial.println(screens[i].line1);
    Serial.println(screens[i].line2);
    Serial.println(screens[i].line3);
    Serial.println(screens[i].line4);
    Serial.println(screens[i].line5);
  }
}

void updateScreenCardInfo() {
  sdCard.updateCardInfo();
  snprintf(screens[4].line1, 22, " Card: %s", sdCard.cardInfo.cardInserted ? "Present" : "Missing");
  snprintf(screens[4].line2, 22, " Type: %s", sdCard.cardInfo.cardType);
  snprintf(screens[4].line3, 22, "Total: %s", sdCard.cardInfo.totalSize);
  snprintf(screens[4].line4, 22, " Free: %s", sdCard.cardInfo.freeSize);
  snprintf(screens[4].line5, 22, "Files: %8d", sdCard.cardInfo.fileCount);
}

void updateScreenDateTime() {
  time_t time_gmt = now();
  time_t time_local = time_gmt + (sdCard.sdCardConfig.gmtOffset * SECS_PER_HOUR);

  snprintf(screens[0].line2, 22, "   Date Time (UTC)");
  snprintf(screens[0].line3, 22, "%02d/%02d/%d %02d:%02d:%02d", month(time_gmt), day(time_gmt), year(time_gmt), hour(time_gmt), minute(time_gmt), second(time_gmt));
  snprintf(screens[0].line4, 22, "     Local (%s)", sdCard.sdCardConfig.tzAbbr);
  snprintf(screens[0].line5, 22, "%02d/%02d/%d %02d:%02d:%02d", month(time_local), day(time_local), year(time_local), hour(time_local), minute(time_local), second(time_local));
}

void updateScreenMemoryInfo() {
  int total = rp2040.getTotalHeap();
  int free = rp2040.getFreeHeap();
  float percentFree = ((float)free / (float)total) * 100.0f;
  float freq = ((float)rp2040.f_cpu()) / 1000000.0f;
  snprintf(screens[5].line1, 22, "    MEMORY: Heap");
  snprintf(screens[5].line2, 22, "Total: %6d b", total);
  snprintf(screens[5].line3, 22, " Free: %6d b", free);
  snprintf(screens[5].line4, 22, "       %6.2f %%", percentFree);
  snprintf(screens[5].line5, 22, " Freq: %6.2f MHz", freq);
}

/*****************************************************************************
 *                                   TASKS                                   *
 *****************************************************************************/
void checkBattery(void *param) {
  // Make certain that setup has completed
  while (!setup_complete) {
    vTaskDelay(10 / portTICK_PERIOD_MS);
  }

  int loopCount = 0;

  while (1) {
    battery.updateBattery();

    snprintf(screens[3].line1, 22, "Battery: %6.2f V", battery.batteryVoltage);
    snprintf(screens[3].line2, 22, "Battery: %6.2f %%", battery.batteryPercent);
    snprintf(screens[3].line3, 22, " Status: %s", batteryChargeStateName[battery.batteryChargeState]);

    loopCount++;
    // Print message with stack usage once every ten loops
    if (loopCount > waitLoopsToPrintTaskInfo) {
      debugMessage("checkBattery");
      loopCount = 0;
    }
    vTaskDelay(500 / portTICK_PERIOD_MS);
  }  // End Loop
}

void checkGps(void *param) {
  // Make certain that setup has completed
  while (!setup_complete) {
    vTaskDelay(10 / portTICK_PERIOD_MS);
  }

  int loopCount = 0;

  uint32_t timer = millis();

  pinMode(PIN_GPS_CTRL, OUTPUT);
  digitalWrite(PIN_GPS_CTRL, HIGH);

  // 9600 NMEA is the default baud rate for Adafruit MTK GPS's- some use 4800
  GPS.begin(9600);
  // uncomment this line to turn on RMC (recommended minimum) and GGA (fix data) including altitude
  GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);
  // uncomment this line to turn on only the "minimum recommended" data
  //GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCONLY);
  // For parsing data, we don't suggest using anything but either RMC only or RMC+GGA since
  // the parser doesn't care about other sentences at this time
  // Set the update rate
  GPS.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ);  // 1 Hz update rate
  // For the parsing code to work nicely and have time to sort thru the data, and
  // print it out we don't suggest using anything higher than 1 Hz
  // Request updates on antenna status, comment out to keep quiet
  GPS.sendCommand(PGCMD_ANTENNA);

  vTaskDelay(1000 / portTICK_PERIOD_MS);

  // Ask for firmware version
  GPSSerial.println(PMTK_Q_RELEASE);

  while (1) {
    // DELETE NEXT 3 LINES!!!
    // read data from the GPS in the 'main loop'
    char c = GPS.read();
    // Serial.printf("GPS Data: %c\n", c);

    GpsInfo gpsInfo;
    // if a sentence is received, we can check the checksum, parse it...
    if (GPS.newNMEAreceived()) {
      // a tricky thing here is if we print the NMEA sentence, or data
      // we end up not listening and catching other sentences!
      // so be very wary if using OUTPUT_ALLDATA and trying to print out data
      // Commented this line to stop dumping raw data // Serial.print(GPS.lastNMEA()); // this also sets the newNMEAreceived() flag to false
      if (GPS.parse(GPS.lastNMEA())) {  // this also sets the newNMEAreceived() flag to false
        loopCount++;
        continue;
      }
    }

    // approximately every 2 seconds or so, print out the current stats
    if (millis() - timer > 2010) {
      timer = millis();  // reset the timer
      // gpsInfo.epoch = { 2000 + GPS.year, GPS.month, GPS.day, GPS.hour, GPS.minute, GPS.seconds, GPS.milliseconds };
      // rtc_set_datetime(&gpsInfo.epoch);
      tmElements_t myElements = { GPS.seconds, GPS.minute, GPS.hour, 0, GPS.day, GPS.month, (uint8_t)(GPS.year + 30) };
      gpsInfo.epoch = makeTime(myElements);
      if (timeStatus() != timeSet || gpsInfo.epoch != now()) {
        // setTime(GPS.hour, GPS.minute, GPS.seconds, GPS.day, GPS.month, GPS.year);
        setTime(gpsInfo.epoch);
        Serial.println("\n\n*** Time Set***\n");
        ledOnOff(true);
      }
      gpsInfo.hour = GPS.hour;
      gpsInfo.minute = GPS.minute;
      gpsInfo.seconds = GPS.seconds;
      gpsInfo.year = GPS.year;
      gpsInfo.month = GPS.month;
      gpsInfo.day = GPS.day;
      gpsInfo.latitudeDegrees = GPS.latitudeDegrees;
      gpsInfo.longitudeDegrees = GPS.longitudeDegrees;
      gpsInfo.altitude = GPS.altitude;
      gpsInfo.speed = GPS.speed;
      gpsInfo.angle = GPS.angle;
      gpsInfo.fixquality = (GPS_FIX_QUALITY)GPS.fixquality;
      gpsInfo.fixquality_3d = (GPS_FIX_QUALITY_3D)GPS.fixquality_3d;
      gpsInfo.satellites = GPS.satellites;
      gpsInfo.fix = GPS.fix;
      strftime(gpsInfo.isoTime, sizeof(gpsInfo.isoTime), "%Y-%m-%dT%TZ", gmtime(&gpsInfo.epoch));
      strftime(gpsInfo.utc_date, sizeof(gpsInfo.utc_date), "%Y-%m-%d", gmtime(&gpsInfo.epoch));
      strftime(gpsInfo.utc_time, sizeof(gpsInfo.utc_date), "%T", gmtime(&gpsInfo.epoch));

      if (xQueueSend(gps_queue, (void *)&gpsInfo, 10) != pdTRUE) {
        Serial.println(F("checkGps - GPS Queue Full -----------------------------"));
      }
    }

    loopCount++;
    // Print message with stack usage once every ten loops
    if (loopCount > waitLoopsToPrintTaskInfo) {
      debugMessage("checkGps");
      loopCount = 0;
      ledOnOff(false);
    }
    // vTaskDelay(2000 / portTICK_PERIOD_MS);
  }  // End Loop
}

void checkSdCard(void *param) {
  // Make certain that setup has completed
  while (!setup_complete) {
    vTaskDelay(10 / portTICK_PERIOD_MS);
  }

  int loopCount = 0;
  Log_Entry item;

  // Force initial loading of config file
  sdCard.cardPresent = false;

  while (1) {
    // Handle if the card was removed and/or reinserted
    sdCard.isCardPresent();

    // Check if there are log entries to write to the SD Card
    bool writeToFile = false;
    while (xQueueReceive(log_queue, (void *)&item, 0) == pdTRUE) {
      if (sdCard.cardPresent && strlen(item.logfile) > 0) {
        writeToFile = true;
      }

      if (writeToFile) {
        if (xSemaphoreTake(sdcard_mutex, 10) == pdTRUE) {
          sdCard.writeLogEntry(item.logfile, item.message);
          xSemaphoreGive(sdcard_mutex);
        } else {
          Serial.printf("%s: %s checkSdCard - Failed to write log entry to the SD Card. (Could not obtain mutex.)\n", debugLevelName[DebugLevels::Error], FILE_NAME);
          writeToFile = false;
        }
      } else {
        // DEBUGV("%s\n", item.message);
        Serial.printf("%s\n", item.message);
      }
    }

    loopCount++;
    // Print message with stack usage once every ten loops
    if (loopCount > waitLoopsToPrintTaskInfo) {
      debugMessage("checkSdCard");
      loopCount = 0;
    }
    vTaskDelay(250 / portTICK_PERIOD_MS);
  }
}

void checkSwitches(void *param) {
  // Make certain that setup has completed
  while (!setup_complete) {
    vTaskDelay(10 / portTICK_PERIOD_MS);
  }

  int loopCount = 0;

  unsigned long lastDebounceTime = 0;  // the last time the output pin was toggled
  unsigned long debounceDelay = 35;    // the debounce time; increase if the output flickers

  uint32_t displayOnTimer = millis();

  switches[0].pin = PIN_SW0;
  switches[1].pin = PIN_SW1;
  switches[2].pin = PIN_SW2;
  switches[3].pin = PIN_SW3;

  // Setup Switches
  for (int i = 0; i < 4; i++) {
    switches[i].index = i;
    pinMode(switches[i].pin, INPUT);
  }

  while (1) {
    bool statechanged = false;
    // read the state of the switches into a local variable:
    for (int i = 0; i < 4; i++) {
      switches[i].current = digitalRead(switches[i].pin);
      if (switches[i].current != switches[i].last) {
        statechanged = true;
      }
    }

    // If the switch changed, due to noise or pressing:
    if (statechanged) {
      // wait for things to calm down
      vTaskDelay(debounceDelay / portTICK_PERIOD_MS);

      displayOnTimer = millis();
    } else {
      if (display.displayOn && (millis() - displayOnTimer) > (sdCard.sdCardConfig.displayOffSecs * 1000)) {
        display.toggleDisplayOnOff(false, true);
      }
    }

    for (int i = 0; i < 4; i++) {
      SwitchStates lastState = switches[i].state;

      if (switches[i].current && !switches[i].last) {
        switches[i].state = SwitchStates::Pressed;
      } else if (!switches[i].current && switches[i].last) {
        switches[i].state = SwitchStates::Released;
      } else if (switches[i].current == 1) {
        switches[i].state = SwitchStates::Down;
      } else {
        switches[i].state = SwitchStates::Up;
      }

      switches[i].last = switches[i].current;

      if (switches[i].state != lastState) {
        if (xQueueSend(switch_queue, (void *)&switches[i], 10) != pdTRUE) {
          Serial.println(F("checkSwitches - Switch Queue Full -----------------------------"));
        }
      }
    }

    loopCount++;
    // Print message with stack usage once every ten loops
    if (loopCount > waitLoopsToPrintTaskInfo) {
      debugMessage("checkSwitches");
      loopCount = 0;
    }
    vTaskDelay(debounceDelay / portTICK_PERIOD_MS);
  }  // End Loop
}

void handleSwitches(void *param) {
  // Make certain that setup has completed
  while (!setup_complete) {
    vTaskDelay(10 / portTICK_PERIOD_MS);
  }

  int loopCount = 0;

  Switch item;

  while (1) {

    while (xQueueReceive(switch_queue, (void *)&item, 0) == pdTRUE) {
      // Serial.printf("SW%d %s\n", item.index, switchStateName[item.state]);
      if (item.state == SwitchStates::Pressed) {
        if (!display.displayOn) {
          display.toggleDisplayOnOff(true, false);
          pixels.setPixelColor(0, 0);
          pixels.show();
        } else {
          switch (item.index) {
            case 0:
              // display.toggleDisplayOnOff();
              // pixels.setPixelColor(0, 0);
              // pixels.show();
              break;
            case 1:
              if (display.displayOn) {
                currentScreenIndex--;
                if (currentScreenIndex < 0) {
                  currentScreenIndex = NUM_OF_SCREENS - 1;
                }
              } else {
                neopixelToggle(1, 0, 0);
              }
              break;
            case 2:
              if (display.displayOn) {
                currentScreenIndex++;
                if (currentScreenIndex >= NUM_OF_SCREENS) {
                  currentScreenIndex = 0;
                }
              } else {
                neopixelToggle(0, 1, 0);
              }
              break;
            case 3:
              if (display.displayOn) {
                sdCard.sdCardConfig.metric = !sdCard.sdCardConfig.metric;
              } else {
                neopixelToggle(0, 0, 1);
              }
              break;
            default:
              break;
          }
        }
      }
      Serial.printf("Switch %d %s\n", item.index, switchStateName[item.state]);
      if (item.state == SwitchStates::Up) {
        Serial.println();
      }
    }

    loopCount++;
    // Print message with stack usage once every ten loops
    if (loopCount > waitLoopsToPrintTaskInfo) {
      debugMessage("handleSwitches");
      loopCount = 0;
    }
    vTaskDelay(150 / portTICK_PERIOD_MS);
  }  // End Loop
}

void logGps(void *param) {
  // Make certain that setup has completed
  while (!setup_complete) {
    vTaskDelay(10 / portTICK_PERIOD_MS);
  }

  int loopCount = 0;

  GpsInfo item;

  while (1) {
    while (xQueueReceive(gps_queue, (void *)&item, 0) == pdTRUE) {
      snprintf(screens[1].line1, 22, "  Lat: %8.4f %c", abs(item.latitudeDegrees), item.latitudeDegrees > 0 ? 'N' : 'S');
      snprintf(screens[1].line2, 22, " Long: %8.4f %c", abs(item.longitudeDegrees), item.longitudeDegrees > 0 ? 'E' : 'W');
      if (sdCard.sdCardConfig.metric) {
        snprintf(screens[1].line3, 22, "  Alt: %7.2f m", item.altitude);
        snprintf(screens[1].line4, 22, "Speed: %7.2f kph", item.speed * 1.852);
      } else {
        snprintf(screens[1].line3, 22, "  Alt: %7.2f ft", item.altitude * 3.28084);
        snprintf(screens[1].line4, 22, "Speed: %7.2f mph", item.speed * 1.15078);
      }
      snprintf(screens[1].line5, 22, " Head: %7.2f deg", item.angle);

      snprintf(screens[2].line1, 22, " Satellites: %d", item.satellites);
      snprintf(screens[2].line2, 22, "    GPS Fix: %s", item.fix ? "Yes" : "No");
      snprintf(screens[2].line3, 22, "Fix Quality: %s", gpsFixQualityName[item.fixquality]);
      snprintf(screens[2].line4, 22, " 3D Quality: %s", gpsFixQuality3dName[item.fixquality_3d]);
      snprintf(screens[2].line5, 22, "GPS Enabled: %s", digitalRead(PIN_GPS_CTRL) == HIGH ? "Yes" : "No");

      gpsWriteToLog(item);
    }

    loopCount++;
    // Print message with stack usage once every ten loops
    if (loopCount > waitLoopsToPrintTaskInfo * 1000) {
      debugMessage("handleSwitches");
      loopCount = 0;
    }
    vTaskDelay(150 / portTICK_PERIOD_MS);
  }  // End Loop
}

void updateScreens(void *param) {
  // Make certain that setup has completed
  while (!setup_complete) {
    vTaskDelay(10 / portTICK_PERIOD_MS);
  }

  int loopCount = 0;

  while (1) {
    updateScreenDateTime();

    if (xSemaphoreTake(sdcard_mutex, 10) == pdTRUE) {
      updateScreenCardInfo();
      xSemaphoreGive(sdcard_mutex);
    } else {
      Serial.printf("%s: %s updateScreens - Failed to update SD Card infomation. (Could not obtain mutex.)\n", debugLevelName[DebugLevels::Error], FILE_NAME);
    }

    updateScreenMemoryInfo();

    printScreens();
    display.showScreen(screens[currentScreenIndex]);

    loopCount++;
    // Print message with stack usage once every ten loops
    if (loopCount > waitLoopsToPrintTaskInfo) {
      debugMessage("updateScreens");
      loopCount = 0;
    }
    vTaskDelay(250 / portTICK_PERIOD_MS);
  }  // End Loop
}


/*****************************************************************************
 *                                   SETUP                                   *
 *****************************************************************************/
void setup() {
  Serial.begin(115200);

  while (DEBUG && !Serial) {
    delay(1);  // wait for serial port to connect.
  }

  // Print 5 blank lines on startup
  for (int i = 0; i < 5; i++) {
    Serial.println("");
  }

  Serial.println("### SETUP STARTING ###");

  display.begin();

  // START: SD Card
  sdCard.begin();
  // Load the config file if the card is present before we do anything else
  if (sdCard.isCardPresent()) {
    // *** Archive old backup files ***
    sdCard.renameFile(logDebug, logDebugArchive);
  }
  // END: SD Card

  setSyncInterval(14400);  // Update the time every 4 hours.
  // After 4 hours, calling timeStatus() will return timeNeedsSync.

  snprintf(screens[0].line1, 22, "TeelSys GPS v0.5 C++");

  battery.begin(PIN_BAT_V, PIN_USB_D);

  // Setup Feather's LED and Neopixel
  pinMode(PIN_FEATHER_LED, OUTPUT);
  pixels.begin();
  ledOnOff(false);
  pixels.setPixelColor(0, 0);
  pixels.show();


  // Create Queues
  switch_queue = xQueueCreate(switch_queue_len, sizeof(Switch));
  gps_queue = xQueueCreate(gps_queue_len, sizeof(GpsInfo));
  log_queue = xQueueCreate(log_queue_len, sizeof(Log_Entry));

  // Create Mutex before starting tasks
  sdcard_mutex = xSemaphoreCreateMutex();
  rtc_mutex = xSemaphoreCreateMutex();

  setup_complete = true;

  xTaskCreate(checkSwitches, "CHECK_SWITCHES", 2048, nullptr, 1, nullptr);
  xTaskCreate(checkGps, "CHECK_GPS", 2048, nullptr, 1, nullptr);
  xTaskCreate(checkBattery, "CHECK_BATTERY", 2048, nullptr, 1, nullptr);
  xTaskCreate(checkSdCard, "SDCARD", 2048, nullptr, 1, nullptr);

  xTaskCreate(handleSwitches, "HANDLE_SWITCHES", 2048, nullptr, 1, nullptr);
  xTaskCreate(logGps, "LOG_GPS", 2048, nullptr, 1, nullptr);
  xTaskCreate(updateScreens, "UPDATE_SCREENS", 2048, nullptr, 1, nullptr);

  Serial.println("### SETUP ENDING ###");
}

/*****************************************************************************
 *                                    LOOP                                   *
 *****************************************************************************/
void loop() {
  // put your main code here, to run repeatedly:
}
