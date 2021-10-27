# Battery
battPin = None
usbPin = None
minVoltage = 3.35
maxVoltage = 4.2
voltage = 0
battPercent = 0
isCharging = False
isCharged = False
battStatus = ""
lastChargedState = False

# Display
display = None
splash = None
label_areas = []
field_areas = []
font_width = 0
font_height = 0
space_char = 1
space_line = 6
rowOffsetX = 0
rowOffsetY = 0
display_rows = 0
display_cols = 0
currentScreen = 0
battSprite = None
lastBattIconIdx = 0

# GPS
gpsEnable = None
gps = None

# RTC
the_rtc = None

# SD Card
sdCardCD = None
sdCardCS = None
sdCardSPI = None
sdcard = None
sdCardVfs = None
sdCardInitialized = False
sdCardLED = None
total_size_in_mb = 0
available_space_in_mb = 0
fileCount = 0
sdCardEnabled = True
sdcardDebounceTime = 0.2

# Switches
switchPins = []
switchStates = []
switchLastState = []
switchLastDebounceTime = []
switchDebounceTime = 0.05

# Constants
batteryStatus_Discharging = "Discharging"
batteryStatus_Charging = "Charging"
batteryStatus_Charged = "Charged"

def clearDictionaryValues(labelValDict):
    # Clear dictonary values
    for i, k in enumerate(labelValDict):
        labelValDict[k]["value"] = ""

def dataToString(val, format="{}", units=""):
    if val is None:
        return ""

    if format == "":
        format = "{}"

    return (format + " {}").format(val, units)

def datetimeDisplay(obj, includeSeconds=True):
    if obj is None or obj.datetime is None:
        return ""

    if includeSeconds:
        return "{}/{}/{} {:02}:{:02}:{:02}".format(
                    obj.datetime.tm_mon,
                    obj.datetime.tm_mday,
                    obj.datetime.tm_year,
                    obj.datetime.tm_hour,
                    obj.datetime.tm_min,
                    obj.datetime.tm_sec,
                )

    return "{}/{}/{} {:02}:{:02}".format(
                obj.datetime.tm_mon,
                obj.datetime.tm_mday,
                obj.datetime.tm_year,
                obj.datetime.tm_hour,
                obj.datetime.tm_min,
            )

def datetimeFileName(obj):
    if obj is None or obj.datetime is None:
        return ""

    return "{}-{:02}-{:02}".format(
                obj.datetime.tm_year,
                obj.datetime.tm_mon,
                obj.datetime.tm_mday
            )

def datetimeLog(obj):
    if obj is None or obj.datetime is None:
        return ""

    if not timeValid(obj):
        return ""

    return "{}-{}-{} {:02}:{:02}:{:02}".format(
                obj.datetime.tm_mon,
                obj.datetime.tm_mday,
                obj.datetime.tm_year,
                obj.datetime.tm_hour,
                obj.datetime.tm_min,
                obj.datetime.tm_sec,
            )

def timeValid(obj):
    if obj is None or obj.datetime is None:
        return False

    dt = obj.datetime

    if dt.tm_year < 2021 or dt.tm_mon <= 0 or dt.tm_mday <= 0:
        return False

    return True
