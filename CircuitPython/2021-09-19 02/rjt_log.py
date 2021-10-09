import os
import adafruit_sdcard
import board
import busio
import digitalio
import storage
import adafruit_gps

cd = None
cs = None
spi = None
sdcard = None
vfs = None

initialized = False
logFileName = ""

led = digitalio.DigitalInOut(board.LED)
led.direction = digitalio.Direction.OUTPUT


def begin(pinCD, pinCS, pinSCK, pinMOSI, pinMISO):
    global cd, cs, spi, sdcard, vfs, initialized, logFileName

    if pinCD is not None:
        cd = digitalio.DigitalInOut(pinCD)
        cd.direction = digitalio.Direction.INPUT

    spi = busio.SPI(pinSCK, pinMOSI, pinMISO)
    cs = digitalio.DigitalInOut(pinCS)

    init()


def init():
    global cd, cs, spi, sdcard, vfs, initialized, logFileName

    if initialized:
        return
    if cd is not None and not isCardPresent():
        return

    sdcard = adafruit_sdcard.SDCard(spi, cs)
    vfs = storage.VfsFat(sdcard)
    storage.mount(vfs, "/sd")
    initialized = True


def doesFileExist(fname):
    if fname is None:
        return False

    for file in os.listdir("/sd/"):
        if file == fname:
            return True

    return False


def getLogFileName(datetime):
    global cd, cs, spi, sdcard, vfs, initialized, logFileName

    logFileName = "{}-{:02}-{:02}.TXT".format(
        datetime.tm_year, datetime.tm_mon, datetime.tm_mday
    )


def isCardPresent():
    global cd, cs, spi, sdcard, vfs, initialized, logFileName

    if cd is None:
        return True

    if cd.value:
        initialized = False

    return not cd.value


def writeField(f, val, endChar):
    if not isCardPresent():
        return

    if f is None:
        return

    if val is not None:
        f.write("{}".format(val))

    f.write(endChar)


def writeLogEntry(gps, datetime):
    global cd, cs, spi, sdcard, vfs, initialized, logFileName

    if not gps.has_fix:
        return
    if not isCardPresent():
        return
    if not initialized:
        init()
        if not initialized:
            return

    # Update the log file name based on the current date
    getLogFileName(datetime)

    writeHeader = not doesFileExist(logFileName)

    filefullpath = '/sd/{}'.format(logFileName)

    with open(filefullpath, "a") as f:
        led.value = True  # turn on LED to indicate we're writing to the file

        if writeHeader:  # Write Header
            f.write("Date/Time\tLat\tLong\tAlt\tHeading\tSpeed\tSatellites\n")

        writeField(
            f,
            "{}-{}-{} {:02}:{:02}:{:02}".format(
                datetime.tm_mon,
                datetime.tm_mday,
                datetime.tm_year,
                datetime.tm_hour,
                datetime.tm_min,
                datetime.tm_sec,
            ),
            "\t",
        )

        writeField(f, gps.latitude, "\t")
        writeField(f, gps.longitude, "\t")
        writeField(f, gps.altitude_m, "\t")
        writeField(f, gps.track_angle_deg, "\t")
        writeField(f, gps.speed_knots, "\t")
        writeField(f, gps.satellites, "\n")

        led.value = False  # turn off LED to indicate we're done
