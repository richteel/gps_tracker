import os
import adafruit_sdcard
import busio
import digitalio
import storage
import time

cd = None
cs = None
spi = None
sdcard = None
vfs = None

initialized = False
logFileName = ""

led = None

total_size_in_mb = 0
available_space_in_mb = 0
fileCount = 0

def begin(pinCD, pinCS, pinSCK, pinMOSI, pinMISO, pinLED=None):
    global cd, cs, spi, sdcard, vfs, initialized, logFileName, led

    if pinCD is not None:
        cd = digitalio.DigitalInOut(pinCD)
        cd.direction = digitalio.Direction.INPUT

    if pinLED is not None:
        led = digitalio.DigitalInOut(pinLED)
        led.direction = digitalio.Direction.OUTPUT

    spi = busio.SPI(pinSCK, pinMOSI, pinMISO)
    cs = digitalio.DigitalInOut(pinCS)

    init()

def init():
    global cd, cs, spi, sdcard, vfs, initialized, logFileName, led, \
           total_size_in_mb, available_space_in_mb, fileCount

    if initialized:
        return
    # Make certain that the card has settled/debounce
    # Needed when inserting a card
    time.sleep(0.5)

    total_size_in_mb = 0

    if cd is not None and not isCardPresent():
        return

    if led is not None:
        led.value = True  # turn on LED to indicate we're accessing card

    sdcard = adafruit_sdcard.SDCard(spi, cs)
    vfs = storage.VfsFat(sdcard)
    storage.mount(vfs, "/sd")
    initialized = True

    # Calculate disk space
    # Avoid performing this disk size calculation on very large disks (takes too long)
    # Works just fine on 8GB disks, hangs on 256GB disks
    vfs_stats = os.statvfs("/sd")
    # Total size = block size * number of blocks
    total_size_in_mb = int(vfs_stats[0] * vfs_stats[2] / (1024 * 1024))
    # Free space = block size * number of available blocks
    available_space_in_mb = int(vfs_stats[0] * vfs_stats[3] / (1024 * 1024))

    fileCount = 0
    path = "/sd"
    for file in os.listdir(path):
        stats = os.stat(path + "/" + file)
        isdir = stats[0] & 0x4000

        if not isdir:
            fileCount += 1

    if led is not None:
        led.value = False  # turn off LED to indicate we're done accessing card

def clearInitFlag():
    global cd, cs, spi, sdcard, vfs, initialized, logFileName, led, total_size_in_mb
    initialized = False
    total_size_in_mb = 0

    if vfs is None:
        return
    storage.umount(vfs)
    vfs = None
    sdcard = None


def doesFileExist(fname):
    if fname is None:
        return False

    for file in os.listdir("/sd/"):
        if file == fname:
            return True

    return False

def getLogFileName(datetime):
    global cd, cs, spi, sdcard, vfs, initialized, logFileName, led

    logFileName = "{}-{:02}-{:02}.TXT".format(
        datetime.tm_year, datetime.tm_mon, datetime.tm_mday
    )

def getLogFileNameWithDte(datetime, prefix, suffix):
    global cd, cs, spi, sdcard, vfs, initialized, logFileName, led

    return "{}{}-{:02}-{:02}{}".format(
        prefix, datetime.tm_year, datetime.tm_mon, datetime.tm_mday, suffix
    )


def isCardPresent():
    global cd, cs, spi, sdcard, vfs, initialized, logFileName, led

    if cd is None:
        return True

    isPresent = cd.value

    if not isPresent:
        clearInitFlag()

    return isPresent


def knots2kpm(speed_kn):
    if speed_kn is None:
        return None
    return speed_kn * 1.852

def updateStatus():
    global cd, cs, spi, sdcard, vfs, initialized, logFileName, led, \
           total_size_in_mb, available_space_in_mb, fileCount

    fileCount = 0

    if not isCardPresent():
        total_size_in_mb = 0
        available_space_in_mb = 0
        return

    if not initialized:
        init()

    if not initialized:
        total_size_in_mb = 0
        available_space_in_mb = 0
        return

    if led is not None:
        led.value = True  # turn on LED to indicate we're accessing card

    vfs_stats = os.statvfs("/sd")
    total_size_in_mb = int(vfs_stats[0] * vfs_stats[2] / (1024 * 1024))
    # Free space = block size * number of available blocks
    available_space_in_mb = int(vfs_stats[0] * vfs_stats[3] / (1024 * 1024))

    fileCount = 0
    path = "/sd"
    for file in os.listdir(path):
        stats = os.stat(path + "/" + file)
        isdir = stats[0] & 0x4000

        if not isdir:
            fileCount += 1

    if led is not None:
        led.value = False  # turn off LED to indicate we're done accessing card

def writeField(f, val, endChar):
    if not isCardPresent():
        return

    if f is None:
        return

    if val is not None:
        f.write("{}".format(val))

    f.write(endChar)


def writeLogEntry(datetime, gps):
    global cd, cs, spi, sdcard, vfs, initialized, logFileName, led

    if not gps.has_fix:
        return
    if not isCardPresent():
        return
    if not initialized:
        print("Initializing Card - Card Present = {}".format(isCardPresent()))
        init()
        if not initialized:
            return

    # Update the log file name based on the current date
    getLogFileName(datetime)

    writeHeader = not doesFileExist(logFileName)

    filefullpath = "/sd/{}".format(logFileName)

    with open(filefullpath, "a") as f:
        if led is not None:
            led.value = True  # turn on LED to indicate we're writing to the file

        if writeHeader:  # Write Header
            f.write(
                "Date/Time\tLat\tLong\tAlt\tHeading\tSpeed\t"
                + "Satellites\tFix Quality\tFix Quality 3D\n"
            )

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
        writeField(f, knots2kpm(gps.speed_knots), "\t")
        writeField(f, gps.satellites, "\t")
        # GPS quality indicator
        # 0 - fix not available,
        # 1 - GPS fix,
        # 2 - Differential GPS fix (values above 2 are 2.3 features)
        # 3 - PPS fix
        # 4 - Real Time Kinematic
        # 5 - Float RTK
        # 6 - estimated (dead reckoning)
        # 7 - Manual input mode
        # 8 - Simulation mode
        writeField(f, gps.fix_quality, "\t")
        writeField(f, gps.fix_quality_3d, "\n")

        if led is not None:
            led.value = False  # turn off LED to indicate we're done


def write2LogFile(fileName, header, logEntry, datetime):
    global cd, cs, spi, sdcard, vfs, initialized, logFileName, led

    if not isCardPresent():
        return
    if not initialized:
        print("Initializing Card - Card Present = {}".format(isCardPresent()))
        init()
        if not initialized:
            return

    writeHeader = not doesFileExist(fileName)

    filefullpath = "/sd/{}".format(fileName)

    with open(filefullpath, "a") as f:
        if led is not None:
            led.value = True  # turn on LED to indicate we're writing to the file

        if writeHeader:  # Write Header
            f.write("{}\n".format(header))

        dte = "{}-{}-{} {:02}:{:02}:{:02}".format(
                datetime.tm_mon,
                datetime.tm_mday,
                datetime.tm_year,
                datetime.tm_hour,
                datetime.tm_min,
                datetime.tm_sec,
        )

        f.write("{}\t{}\n".format(dte, logEntry))

        if led is not None:
            led.value = False  # turn off LED to indicate we're done


