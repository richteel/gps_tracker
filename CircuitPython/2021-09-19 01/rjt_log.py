import time

import adafruit_sdcard
import board
import busio
import digitalio
import microcontroller
import storage

cd = None       # Card Detect (Card inserted = False)
spi = None      #
cs = None       #
sdcard = None   #
vfs = None      #

initialized = False


def begin(pinCD, pinCS, pinSCK, pinMOSI, pinMISO):
    global cd, spi, cs, sdcard, vfs, initialized

    if pinCD is not None:
        cd = digitalio.DigitalInOut(pinCD)

    cs = digitalio.DigitalInOut(pinCS)
    cs.direction = digitalio.Direction.INPUT
    spi = busio.SPI(pinSCK, pinMOSI, pinMISO)

    init()


def init():
    global cd, spi, cs, sdcard, vfs, initialized

    if initialized:
        return

    if cd is None or isCardPresent():
        sdcard = adafruit_sdcard.SDCard(spi, cs)
        vfs = storage.VfsFat(sdcard)
        storage.mount(vfs, "/sd")
        initialized = True


def isCardPresent():
    global cd, spi, cs, sdcard, vfs, initialized

    if cd is None:
        return True
    else:
        return not cd.value
