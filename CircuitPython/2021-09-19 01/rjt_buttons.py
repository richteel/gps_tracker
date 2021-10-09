# rjt_buttons.py

import time
import digitalio

buttonPins = []
buttonState = []
buttonStateLast = []
buttonlastDebounceTime = []

debounceTime = 0.05

def begin(pinList):
    global buttonPins, buttonState, buttonStateLast, buttonlastDebounceTime, debounceTime

    for pin in pinList:
        buttonPins.append(digitalio.DigitalInOut(pin))
        buttonState.append(False)
        buttonStateLast.append(False)
        buttonlastDebounceTime.append(time.monotonic())

    for x in buttonPins:
        x.direction = digitalio.Direction.INPUT


def getButtonState(i):
    global buttonPins, buttonState, buttonStateLast, buttonlastDebounceTime, debounceTime

    returnVal = False
    reading = buttonPins[i].value

    if reading != buttonStateLast[i]:
        # reset the debouncing timer
        buttonlastDebounceTime[i] = time.monotonic()

    if time.monotonic() - buttonlastDebounceTime[i] > debounceTime:
        # whatever the reading is at, it's been there for longer than the debounce
        # delay, so take it as the actual current state:

        # if the button state has changed:
        if reading != buttonState[i]:
            buttonState[i] = reading

            if buttonState[i]:
                returnVal = True

    # save the reading. Next time through the loop, it'll be the lastButtonState
    buttonStateLast[i] = reading

    return returnVal


def getButtonCount():
    global buttonPins, buttonState, buttonStateLast, buttonlastDebounceTime, debounceTime

    return len(buttonPins)
