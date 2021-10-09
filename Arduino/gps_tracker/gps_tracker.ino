#include "buttons.h"
#include "SD_Card.h"
#include "Line_Display.h"
#include "GPS_Log.h"
#include <GPSport.h>

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
  {"  Lat:", 7, false},
  {" Long:", 7, false},
  {"  Alt:", 7, false},
  {" Head:", 7, false},
  {"Speed:", 7, false},
  {"  Sat:", 7, false},
  {" Card:", 7, false},
  {" Type:", 7, false},
  {" Size:", 7, false},
  {"Files:", 7, false}
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

void updateDisplay() {
  char buff[22];
  // Date Time
  sprintf(buff, "%02d-%02d-%d %02d:%02d:%02d", month(), day(), year(), hour(), minute(), second());
  gpsDisplay.updateField(1, buff);

  // Lattitude
  sprintf(buff, " ");
  if (fix.valid.location) {
    dtostrf(abs(fix.latitude()), 10, 6, buff);
    sprintf(buff, "%s %c", buff, fix.latitude() > 0 ? 'N' : 'S');
  }
  gpsDisplay.updateField(2, buff);

  // Longitude
  sprintf(buff, " ");
  if (fix.valid.location) {
    dtostrf(abs(fix.longitude()), 10, 6, buff);
    sprintf(buff, "%s %c", buff, fix.longitude() > 0 ? 'E' : 'W');
  }
  gpsDisplay.updateField(3, buff);

  // Altitude
  sprintf(buff, " ");
  if (fix.valid.altitude) {
    dtostrf(fix.altitude(), 10, 3, buff);
    sprintf(buff, "%s %c", buff, 'M');
  }
  gpsDisplay.updateField(4, buff);

  // Heading
  sprintf(buff, " ");
  if (fix.valid.heading) {
    dtostrf(fix.heading(), 10, 3, buff);
    sprintf(buff, "%s %s", buff, "deg");
  }
  gpsDisplay.updateField(5, buff);

  // Speed
  sprintf(buff, " ");
  if (fix.valid.speed) {
    dtostrf(fix.speed_kph(), 10, 3, buff);
    sprintf(buff, "%s %s", buff, "KPH");
  }
  gpsDisplay.updateField(6, buff);

  // Satellites
  sprintf(buff, "%6d", fix.satellites);
  gpsDisplay.updateField(7, buff);

  /*
    {" Card: ", 7, false},
    {" Type: ", 7, false},
    {" Size: ", 7, false},
    {"Files: ", 7, false}
  */
}

/********************/
/*   SETUP & LOOP   */
/********************/
void setup() {
  // *** SERIAL PORT - DEBUG ***
  Serial.begin(9600);

  if (Serial) {
    //Serial.print(F(__FILE__));
    //Serial.println(F(": started"));
  }

  // Buttons
  gpsButtons.begin(buttonCount, buttonStates[0]);

  // Display
  //gpsDisplay.begin(&Adafruit128x64, I2C_ADDRESS, RST_PIN, Adafruit5x7, labelCount, labels);
  //gpsDisplay.begin(&Adafruit128x64, I2C_ADDRESS, RST_PIN, X11fixed7x14, labelCount, labels);
  //gpsDisplay.begin(&Adafruit128x64, I2C_ADDRESS, RST_PIN, Corsiva_12, labelCount, labels);
  gpsDisplay.begin(&Adafruit128x64, I2C_ADDRESS, RST_PIN, TeelSys5x14, labelCount, labels[0]);

  gpsDisplay.incrementScreen(0);

  // SD CARD
  sdCard.begin(chipSelect, cardDetect);

  // GPS
  gpsPort.begin(9600);

  // Log
  gpsLog.begin("Date/Time\tLat\tLong\tAlt\tHeading\tSpeed\tSatellites", sdCard);
}


void loop() {
  while (gps.available( gpsPort )) {
    fix = gps.read();
  }

  // Set the time
  if (timeStatus() != timeSet) {
    if (fix.valid.time && fix.valid.date) {
      setTime(fix.dateTime.hours, fix.dateTime.minutes, fix.dateTime.seconds, fix.dateTime.date, fix.dateTime.month, fix.dateTime.full_year());
      //if (Serial) {
      //  Serial.print(F("Time set from GPS"));
      //}
    }
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

      //if (Serial) {
      //  Serial.print(F("Button clicked -> "));
      //  Serial.println(i);
      //}
    }
  }

  // Check Card

  if (sdCard.cardState != sdCard.CardPresent()) {
    sdCard.UpdateSDInfo();
  }
  if (sdCard.requestReset) {
    //if (Serial)
    //  Serial.println(F("Reseting in 1 second"));

    delay(1000);
    resetFunc();  //call reset
  }


  unsigned long currentMillis = millis();

  if (currentMillis - previousUpdateMillis >= updateInterval) {
    previousUpdateMillis = currentMillis;

    gpsLog.logGPS(fix);

    updateDisplay();
  }
}
