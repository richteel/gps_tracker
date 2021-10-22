import time
import digitalio

import helper
import pins

def begin():
    for pin in pins.switchPins:
        helper.switchPins.append(digitalio.DigitalInOut(pin))
        helper.switchStates.append(False)
        helper.switchLastState.append(False)
        helper.switchLastDebounceTime.append(time.monotonic())

    for x in helper.switchPins:
        x.direction = digitalio.Direction.INPUT

def getSwitchState(i):
    returnVal = False
    reading = helper.switchPins[i].value

    if reading != helper.switchLastState[i]:
        # reset the debouncing timer
        helper.switchLastDebounceTime[i] = time.monotonic()

    if time.monotonic() - helper.switchLastDebounceTime[i] > helper.switchDebounceTime:
        # whatever the reading is at, it's been there for longer than the debounce
        # delay, so take it as the actual current state:

        # if the button state has changed:
        if reading != helper.switchStates[i]:
            helper.switchStates[i] = reading

            if helper.switchStates[i]:
                returnVal = True

    # save the reading. Next time through the loop, it'll be the lastButtonState
    helper.switchLastState[i] = reading

    return returnVal

def getSwitchCount():
    return len(helper.switchPins)
