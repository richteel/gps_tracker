#include "buttons.h"


/******** PUBLIC ********/
void Buttons::begin(int buttonCount, ButtonState buttons[]) {
  buttonStateArrayLen = buttonCount;
  buttonStateArrPtr = &buttons[0];

  for (int i = 0; i < buttonCount; i++) {
    pinMode(buttons[i].switchPin, INPUT);
  }
}

bool Buttons::readSwitch(int swIndex) {
  // buttonStateArrPtr[swIndex].switchPin
  // buttonStateArrPtr[swIndex].currentState
  // buttonStateArrPtr[swIndex].lastButtonState
  // buttonStateArrPtr[swIndex].lastDebounceTime

  bool retval = false;
  int reading = digitalRead(buttonStateArrPtr[swIndex].switchPin);

  if (reading != buttonStateArrPtr[swIndex].lastButtonState) {
    // reset the debouncing timer
    buttonStateArrPtr[swIndex].lastDebounceTime = millis();
  }

  if ((millis() - buttonStateArrPtr[swIndex].lastDebounceTime) > debounceDelay) {
    // whatever the reading is at, it's been there for longer than the debounce
    // delay, so take it as the actual current state:

    // if the button state has changed:
    if (reading != buttonStateArrPtr[swIndex].currentState) {
      buttonStateArrPtr[swIndex].currentState = reading;

      if (buttonStateArrPtr[swIndex].currentState == HIGH) {
        retval = true;
      }
    }
  }

  // save the reading. Next time through the loop, it'll be the lastButtonState:
  buttonStateArrPtr[swIndex].lastButtonState = reading;
  return retval;
}
