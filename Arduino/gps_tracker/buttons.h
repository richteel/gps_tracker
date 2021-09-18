#ifndef BUTTONS_H
#define BUTTONS_H

#include <stdint.h>
#include <Arduino.h>

typedef struct {
  int switchPin;
  bool currentState;
  bool lastButtonState;
  unsigned long lastDebounceTime;
} ButtonState;

class Buttons {
  public:
    uint8_t buttonStateArrayLen = 0;

    void begin(int buttonCount, ButtonState buttons[]);
    bool readSwitch(int swIndex);

  private:
    ButtonState *buttonStateArrPtr;
    unsigned long debounceDelay = 50;    // the debounce time; increase if the output flickers

};

#endif
