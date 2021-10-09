import board
import busio
import displayio
import terminalio
from adafruit_display_text import label
import adafruit_displayio_ssd1306
import math

gpsLabels = [
    "  TeelSys GPS v0.1   ",
    "",
    "  Lat:",
    " Long:",
    "  Alt:",
    " Head:",
    "Speed:",
    "  Sat:",
    "Fix Q:",
    " Card:",
    " Size:",
    " Free:",
    "Files:",
    " Batt:",
]

display = None
splash = None
label_areas = []
field_areas = []
font_width = 0
font_height = 0
space_char = 1
space_line = 6
row0_x = 0
row0_y = 0
display_rows = 0
display_cols = 0
currentScreen = 0

batteryVoltage = 0

def begin(dwidth=128, dheight=64, scl=board.SCL, sda=board.SDA, i2c_address=0x3C):
    global gpsLabels, display, splash, label_areas, field_areas, \
           font_width, font_height, space_char, space_line, row0_x, row0_y, \
           display_rows, display_cols, currentScreen, batteryVoltage

    displayio.release_displays()

    i2c = busio.I2C(scl, sda)
    display_bus = displayio.I2CDisplay(i2c, device_address=i2c_address)
    display = adafruit_displayio_ssd1306.SSD1306(display_bus, width=dwidth,
                                                 height=dheight)
    # Make the display context
    splash = displayio.Group()
    display.show(splash)

    font_width, font_height = terminalio.FONT.get_bounding_box()
    font_width = font_width - space_char
    font_height = font_height - space_line
    display_cols = math.floor((dwidth + space_char)/(font_width + space_char))
    display_rows = math.floor((dheight + space_line)/(font_height + space_line))
    row0_x = math.floor(space_char/2)
    row0_y = math.floor(space_line/2)

    printDisplayInfo()

def changeScreen(incValue):
    global gpsLabels, display, splash, label_areas, field_areas, \
           font_width, font_height, space_char, space_line, row0_x, row0_y, \
           display_rows, display_cols, currentScreen, batteryVoltage

    currentScreen = currentScreen + incValue
    fieldCount = len(gpsLabels)

    if currentScreen < 0:
        currentScreen = math.floor((fieldCount - 1) / display_rows)
    elif currentScreen > (fieldCount - 1) / display_rows:
        currentScreen = 0

    updateLabels()

def clearDisplay():
    global gpsLabels, display, splash, label_areas, field_areas, \
           font_width, font_height, space_char, space_line, row0_x, row0_y, \
           display_rows, display_cols, currentScreen, batteryVoltage

    for r in range(0, display_rows):
        if label_areas is not None and r < len(label_areas):
            label_areas[r].text = ""

        if field_areas is not None and r < len(field_areas):
            field_areas[r].text = ""

    splash = displayio.Group()
    display.show(splash)

    print("(Clear) Splash = {}".format(len(splash)))

def knots2kpm(speed_kn):
    if speed_kn is None:
        return None
    return speed_kn * 1.852

def setLabel(text, column, row):
    global gpsLabels, display, splash, label_areas, field_areas, \
           font_width, font_height, space_char, space_line, row0_x, row0_y, \
           display_rows, display_cols, currentScreen, batteryVoltage

    if row > (display_rows - 1):
        raise NotImplementedError("Only {} rows supported".format(display_rows))

    x = ((font_width + space_char) * column) + row0_x
    y = ((font_height + space_line) * row) + row0_y

    text_area = label.Label(terminalio.FONT, text=text, color=0xFFFF00, x=x, y=y)

    return text_area

def sleepWake():
    if (display.is_awake):
        display.sleep()
    else:
        display.wake()

def updateField(row, val, format="{}", units=""):
    if val is not None:
        if format == "":
            format = "{}"
        field_areas[row].text = (format + " {}").format(val, units)

def updateFields(datetime, gps, log):
    global gpsLabels, display, splash, label_areas, field_areas, \
           font_width, font_height, space_char, space_line, row0_x, row0_y, \
           display_rows, display_cols, currentScreen, batteryVoltage

    for i in range(0, display_rows):
        fieldIndex = (currentScreen * display_rows) + i

        if fieldIndex >= len(gpsLabels):
            break

        field_areas[i].text = ""

        if fieldIndex == 1:  # Date/Time
            updateField(i, "{}/{}/{} {:02}:{:02}:{:02}".format(
                datetime.tm_mon,
                datetime.tm_mday,
                datetime.tm_year,
                datetime.tm_hour,
                datetime.tm_min,
                datetime.tm_sec,
            ))
        elif fieldIndex == 2:    # Lattitude
            updateField(i, abs(gps.latitude), "{:.4f}",
                        "S" if gps.latitude < 0 else "N")
        elif fieldIndex == 3:    # Longitude
            updateField(i, abs(gps.longitude), "{:.4f}",
                        "W" if gps.longitude < 0 else "E")
        elif fieldIndex == 4:    # Altitude
            updateField(i, gps.altitude_m, "{:,.1f}", "m")
        elif fieldIndex == 5:    # Heading
            updateField(i, gps.track_angle_deg, "{:.2f}", "deg")
        elif fieldIndex == 6:    # Speed
            updateField(i, knots2kpm(gps.speed_knots), "{:.3f}", "km/h")
        elif fieldIndex == 7:    # Satellites
            updateField(i, gps.satellites)
        elif fieldIndex == 8:    # Fix Quality 3D
            updateField(i, gps.fix_quality_3d, "{}", "(3D)")
        elif fieldIndex == 9:    # Card Inserted
            updateField(i, "Present" if log.initialized else "No card")
        elif fieldIndex == 10:  # Card Size
            updateField(i, log.total_size_in_mb, "{:,}", "MB")
        elif fieldIndex == 11:  # Card Free Space
            updateField(i, log.available_space_in_mb, "{:,}", "MB")
        elif fieldIndex == 12:   # File Count
            updateField(i, log.fileCount)
        elif fieldIndex == 13:   # Battery
            updateField(i, batteryVoltage, "{:.2f}", "V")

def updateLabels():
    global gpsLabels, display, splash, label_areas, field_areas, \
           font_width, font_height, space_char, space_line, row0_x, row0_y, \
           display_rows, display_cols, currentScreen

    clearDisplay()

    label_areas.clear()
    field_areas.clear()

    for i in range(0, display_rows):
        labelIndex = (currentScreen * display_rows) + i

        if labelIndex >= len(gpsLabels):
            break

        label_areas.append(setLabel(gpsLabels[labelIndex], 0, i))
        field_areas.append(setLabel("", len(gpsLabels[labelIndex]) + 1,  i))
        splash.append(label_areas[i])
        splash.append(field_areas[i])

# -------------------------------------

def printDisplayInfo():
    global gpsLabels, display, splash, label_areas, field_areas, \
           font_width, font_height, space_char, space_line, row0_x, row0_y, \
           display_rows, display_cols, currentScreen, batteryVoltage

    print("Display Width = {}".format(display.width))
    print("Display Height = {}".format(display.height))
    print("Display Columns = {}".format(display_cols))
    print("Display Rows = {}".format(display_rows))
    print("Font Width = {}".format(font_width))
    print("Font Height = {}".format(font_height))
