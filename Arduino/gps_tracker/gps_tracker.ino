#include "buttons.h"
#include "SD_Card.h"
#include "Line_Display.h"
#include "GPS_Log.h"
#include "GPS_Display.h"
#include <NMEAGPS.h>
#include <GPSport.h>
#include <TimeLib.h>

/***************/
/*** BUTTONS ***/
/***************/
#define buttonCount 4

ButtonState buttonStates[buttonCount] = {
  {12, 0, 0, 0},
  {13, 0, 0, 0},
  {14, 0, 0, 0},
  {15, 0, 0, 0}
};

Buttons gpsButtons;

/***************/
/*** DISPLAY ***/
/***************/
// 0X3C+SA0 - 0x3C or 0x3D
#define I2C_ADDRESS 0x3C
// Define proper RST_PIN if required.
#define RST_PIN -1

#define labelCount 12

Label labels[labelCount] = {
  {"  TeelSys GPS v0.1   ", 21, true},
  {"", 0, false},
  {"  Lat: ", 7, false},
  {" Long: ", 7, false},
  {"  Alt: ", 7, false},
  {" Head: ", 7, false},
  {"Speed: ", 7, false},
  {"  Sat: ", 7, false},
  {" Card: ", 7, false},
  {" Type: ", 7, false},
  {" Size: ", 7, false},
  {"Files: ", 7, false}
};

LineDisplay gpsDisplay;

/***************/
/*** SD CARD ***/
/***************/
#define chipSelect 0
#define cardDetect 16

SDCard sdCard;

/***********/
/*   GPS   */
/***********/
NMEAGPS  gps; // This parses the GPS characters
gps_fix  fix; // This holds on to the latest values

/*******************/
/*   UPDATE TIME   */
/*******************/
unsigned long previousUpdateMillis = 0;
const long updateInterval = 1000;

/***********/
/*   LOG   */
/***********/
GPS_Log gpsLog;

/*************/
/*   RESET   */
/*************/
void(* resetFunc) (void) = 0; //declare reset function @ address 0

/*
// Debug function to print the date and time
void printDigits(int digits) {
  // utility function for digital clock display: prints preceding colon and leading 0
  Serial.print(":");
  if(digits < 10)
    Serial.print('0');
  Serial.print(digits);
}

void digitalClockDisplay(){
  // digital clock display of the time
  Serial.print(month());
  Serial.print("-");
  Serial.print(day());
  Serial.print("-");
  Serial.print(year());
  Serial.print(" ");  
  Serial.print(hour());
  printDigits(minute());
  printDigits(second());
  Serial.println(); 
}
*/

/********************/
/*   SETUP & LOOP   */
/********************/
void setup() {
  // *** SERIAL PORT - DEBUG ***
  Serial.begin(9600);

  if (Serial) {
    Serial.print(F(__FILE__));
    Serial.println( F(": started") );
  }

  // Buttons
  gpsButtons.begin(buttonCount, buttonStates);

  // Display
  //gpsDisplay.begin(&Adafruit128x64, I2C_ADDRESS, RST_PIN, Adafruit5x7, labelCount, labels);
  //gpsDisplay.begin(&Adafruit128x64, I2C_ADDRESS, RST_PIN, X11fixed7x14, labelCount, labels);
  //gpsDisplay.begin(&Adafruit128x64, I2C_ADDRESS, RST_PIN, Corsiva_12, labelCount, labels);
  gpsDisplay.begin(&Adafruit128x64, I2C_ADDRESS, RST_PIN, TeelSys5x14, labelCount, labels);

  gpsDisplay.incrementScreen(0);

  // SD CARD
  sdCard.begin(chipSelect, cardDetect);

  // GPS
  gpsPort.begin(9600);

  // Log
  gpsLog.begin("Date/Time\tLat\tLong\tAlt\tHeading\tSpeed\tSatellites", sdCard);
}

/* int attemptCount = 0; */

void loop() {
  while (gps.available( gpsPort )) {
    fix = gps.read();
  }

  // Set the time
  if (timeStatus() != timeSet) {
     /* attemptCount++;*/
    if (fix.valid.time && fix.valid.date) {
      setTime(fix.dateTime.hours, fix.dateTime.minutes, fix.dateTime.seconds, fix.dateTime.date, fix.dateTime.month, fix.dateTime.full_year());
      if(Serial) {
        Serial.print(F("Time set from GPS to: "));
        //digitalClockDisplay();
      }
       attemptCount = 0;
    }
    Serial.print(F("Attempted to set time "));
    Serial.print(attemptCount);
    Serial.println(F(" times."));
  }

  // Read the switches
  for (int i = 0; i < buttonCount; i++) {
    bool buttonClicked = gpsButtons.readSwitch(i);

    if (buttonClicked) {
      switch (i) {
        case 0:
          break;
        case 1:
          gpsDisplay.incrementScreen(-1);
          break;
        case 2:
          gpsDisplay.incrementScreen(1);
          break;
        case 3:
          break;
        default:
          break;
      }

      if (Serial) {
        Serial.print(F("Button clicked -> "));
        Serial.println(i);
      }
    }
  }

  // Check Card
  if (sdCard.cardState != sdCard.CardPresent()) {
    sdCard.UpdateSDInfo();
  }
  if (sdCard.requestReset) {
    if (Serial)
      Serial.println(F("Reseting in 1 second"));

    delay(1000);
    resetFunc();  //call reset
  }

  unsigned long currentMillis = millis();

  if (currentMillis - previousUpdateMillis >= updateInterval) {
    previousUpdateMillis = currentMillis;

    //updateFields(fix);

    gpsLog.logGPS(fix);
  }
}
