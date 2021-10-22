import adafruit_sdcard
import busio
import digitalio
import gc
import os
import storage

import dispItems
import helper
import logItems
import pins

def begin():
    if pins.sdCardCD is not None:
        helper.sdCardCD = digitalio.DigitalInOut(pins.sdCardCD)
        helper.sdCardCD.direction = digitalio.Direction.INPUT

    if pins.sdCardLED is not None:
        helper.sdCardLED = digitalio.DigitalInOut(pins.sdCardLED)
        helper.sdCardLED.direction = digitalio.Direction.OUTPUT

    helper.sdCardSPI = busio.SPI(pins.sdCardSCK, pins.sdCardMOSI, pins.sdCardMISO)
    helper.sdCardCS = digitalio.DigitalInOut(pins.sdCardCS)

    init()

def clearInitFlag():
    helper.sdCardInitialized = False

    if helper.sdCardVfs is None:
        return
    storage.umount(helper.sdCardVfs)
    helper.sdCardVfs = None
    helper.sdcard = None

def doesFileExist(fname):
    if not isReady():
        return

    if fname is None:
        return False

    for file in os.listdir("/sd/"):
        if file == fname:
            return True

    return False

def init():
    if isInitialized():
        return

    # Make certain that the card has settled/debounce
    # Needed when inserting a card
    # time.sleep(helper.sdcardDebounceTime)

    if helper.sdCardCD is not None and not isCardPresent():
        return

    try:
        helper.sdcard = adafruit_sdcard.SDCard(helper.sdCardSPI, helper.sdCardCS)

        helper.sdCardVfs = storage.VfsFat(helper.sdcard)
        storage.mount(helper.sdCardVfs, "/sd")
        helper.sdCardInitialized = True
    except OSError as err:
        print("ERROR: (init) Failed to access card. {}".format(err))
        clearInitFlag()

def isCardPresent():
    if helper.sdCardCD is None:
        return isInitialized() and isEnabled()

    isPresent = helper.sdCardCD.value

    if not isPresent:
        clearInitFlag()

    return isPresent

def isEnabled():
    return helper.sdCardEnabled

def isInitialized():
    return helper.sdCardInitialized

def isReady():
    if isEnabled() and isCardPresent() and not isInitialized():
        init()

    return isEnabled() and isCardPresent() and isInitialized()

def setEnable(val):
    if val == helper.sdCardEnabled:
        return

    helper.sdCardEnabled = val

    if not val:
        clearInitFlag()

    if val and not isInitialized():
        init()

def toggleEnable():
    setEnable(not isEnabled())

def update():
    # Clear dictonary values
    helper.clearDictionaryValues(dispItems.sdcard)
    helper.clearDictionaryValues(logItems.sdcard)
    gc.collect()

    cardPresent = isCardPresent()

    dispItems.sdcard["cardPresent"]["value"] = \
        "Present" if cardPresent else "Not Present"
    logItems.sdcard["cardPresent"]["value"] = helper.dataToString(cardPresent)

    dispItems.info["sdcardEnabled"]["value"] = helper.dataToString(isEnabled())

    if isReady():
        try:
            # Calculate disk space
            # Avoid performing this disk size calculation on
            #   very large disks (takes too long)
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

            dispItems.sdcard["cardSizeMb"]["value"] = \
                helper.dataToString(total_size_in_mb, "{:,}", "MB")
            logItems.sdcard["cardSizeMb"]["value"] = \
                helper.dataToString(total_size_in_mb)

            dispItems.sdcard["cardFreeMb"]["value"] = \
                helper.dataToString(available_space_in_mb, "{:,}", "MB")
            logItems.sdcard["cardFreeMb"]["value"] = \
                helper.dataToString(available_space_in_mb)

            dispItems.sdcard["cardFiles"]["value"] = \
                helper.dataToString(fileCount, "{:,}")
            logItems.sdcard["cardFiles"]["value"] = fileCount
        except OSError as err:
            print("ERROR: (update) Failed to access card. {}".format(err))
            clearInitFlag()

def writeToFile(fileName, text):
    if not isReady():
        return

    if not isInitialized():
        return

    filefullpath = "/sd/{}".format(fileName)

    try:
        with open(filefullpath, "a") as f:
            # turn on LED to indicate we're writing to the file
            if helper.sdCardLED is not None:
                helper.sdCardLED.value = True

            f.write("{}\n".format(text))

            # turn off LED to indicate we're done
            if helper.sdCardLED is not None:
                helper.sdCardLED.value = False
    except OSError as err:
        print("ERROR: (writeToFile) Failed to access card. {}".format(err))
        clearInitFlag()
