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
#include <Dictionary.h>

/*****************************************************************************
 *                               Project Files                               *
 *****************************************************************************/
#include "Defines.h"
#include "StructsAndEnums.h"

/*****************************************************************************
 *                                  GLOBALS                                  *
 *****************************************************************************/
// Flags to make certain that required initialization is complete before tasks start
bool setup_complete = false;
int waitLoopsToPrintTaskInfo = 100;

Switch switches[4];

/*****************************************************************************
 *                                   QUEUES                                  *
 *****************************************************************************/
static const uint8_t log_queue_len = 10;
static QueueHandle_t log_queue;

static const uint8_t switch_queue_len = 5;
static QueueHandle_t switch_queue;

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
  // sprintf(msgBuffer, "%s\t%s\t%s %s\t%d\t%d\t%d", timeStr, errorlevel, FILE_NAME, message, xPortGetCoreID(), rp2040.getFreeHeap(), uxTaskGetStackHighWaterMark(NULL));
  sprintf(msgBuffer, "%s %s\t%d\t%d\t%d", "sketch_dec21a.ino", message, xPortGetCoreID(), rp2040.getFreeHeap(), uxTaskGetStackHighWaterMark(NULL));

  Serial.println(msgBuffer);
}

/*****************************************************************************
 *                                   TASKS                                   *
 *****************************************************************************/
void checkSwitches(void *param) {
  // Make certain that setup has completed
  while (!setup_complete) {
    vTaskDelay(10 / portTICK_PERIOD_MS);
  }

  int loopCount = 0;
  char msgBuffer[255];

  unsigned long lastDebounceTime = 0;  // the last time the output pin was toggled
  unsigned long debounceDelay = 35;    // the debounce time; increase if the output flickers

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
  char msgBuffer[255];

  Switch item;

  while (1) {

    while (xQueueReceive(switch_queue, (void *)&item, 0) == pdTRUE) {
        if(item.state == SwitchStates::Pressed) {
          Serial.println();
        }
        Serial.printf("SW%d %s\n", item.index, switchStateName[item.state]);
        if(item.state == SwitchStates::Up) {
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

  // Create Queue
  switch_queue = xQueueCreate(switch_queue_len, sizeof(Switch));

  setup_complete = true;

  xTaskCreate(checkSwitches, "CHECK_SWITCHES", 2048, nullptr, 1, nullptr);

  xTaskCreate(handleSwitches, "HANDLE_SWITCHES", 2048, nullptr, 1, nullptr);
}

/*****************************************************************************
 *                                    LOOP                                   *
 *****************************************************************************/
void loop() {
  // put your main code here, to run repeatedly:
}
