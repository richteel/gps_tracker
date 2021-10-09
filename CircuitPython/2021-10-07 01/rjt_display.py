import board
import busio
import displayio
import terminalio
from adafruit_display_text import label
import adafruit_displayio_ssd1306
import math

displayData = [
    ["  TeelSys GPS v0.1   ", ""],
    ["", ""],
    ["  Lat:", ""],
    [" Long:", ""],
    ["  Alt:", ""],
    [" Head:", ""],
    ["Speed:", ""],
    ["  Sat:", ""],
    ["Fix Q:", ""],
    [" Card:", ""],
    [" Size:", ""],
    [" Free:", ""],
    ["Files:", ""],
    [" Batt:", ""],
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
    global displayData, display, splash, label_areas, field_areas, \
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
    global displayData, display, splash, label_areas, field_areas, \
           font_width, font_height, space_char, space_line, row0_x, row0_y, \
           display_rows, display_cols, currentScreen, batteryVoltage

    currentScreen = currentScreen + incValue
    fieldCount = len(displayData)

    if currentScreen < 0:
        currentScreen = math.floor((fieldCount - 1) / display_rows)
    elif currentScreen > (fieldCount - 1) / display_rows:
        currentScreen = 0

    updateLabels()
    updateFields()

def clearDisplay():
    global displayData, display, splash, label_areas, field_areas, \
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
    global displayData, display, splash, label_areas, field_areas, \
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

def updateData(idx, val, format="{}", units=""):
    global displayData, display, splash, label_areas, field_areas, \
           font_width, font_height, space_char, space_line, row0_x, row0_y, \
           display_rows, display_cols, currentScreen, batteryVoltage

    # Clear the current value string
    displayData[idx][1] = ""

    if val is not None:
        if format == "":
            format = "{}"
        displayData[idx][1] = (format + " {}").format(val, units)

def updateDataBattery(voltage):
    global displayData, display, splash, label_areas, field_areas, \
           font_width, font_height, space_char, space_line, row0_x, row0_y, \
           display_rows, display_cols, currentScreen, batteryVoltage

    batteryVoltage = voltage
    updateData(13, voltage, "{:.2f}", "V")

def updateDataGps(datetime, gps):
    global displayData, display, splash, label_areas, field_areas, \
           font_width, font_height, space_char, space_line, row0_x, row0_y, \
           display_rows, display_cols, currentScreen, batteryVoltage

    updateData(1, "{}/{}/{} {:02}:{:02}:{:02}".format(
                datetime.tm_mon,
                datetime.tm_mday,
                datetime.tm_year,
                datetime.tm_hour,
                datetime.tm_min,
                datetime.tm_sec,
            ))
    updateData(2, abs(gps.latitude), "{:.4f}",
               "S" if gps.latitude < 0 else "N")
    updateData(3, abs(gps.longitude), "{:.4f}",
               "W" if gps.longitude < 0 else "E")
    updateData(4, gps.altitude_m, "{:,.1f}", "m")
    updateData(5, gps.track_angle_deg, "{:.2f}", "deg")
    updateData(6, knots2kpm(gps.speed_knots), "{:.3f}", "km/h")
    updateData(7, gps.satellites)
    updateData(8, gps.fix_quality_3d, "{}", "(3D)")

def updateDataSdCard(log):
    global displayData, display, splash, label_areas, field_areas, \
           font_width, font_height, space_char, space_line, row0_x, row0_y, \
           display_rows, display_cols, currentScreen, batteryVoltage

    updateData(9, "Present" if log.initialized else "No card")
    updateData(10, log.total_size_in_mb, "{:,}", "MB")
    updateData(11, log.available_space_in_mb, "{:,}", "MB")
    updateData(12, log.fileCount)

def updateFields():
    global displayData, display, splash, label_areas, field_areas, \
           font_width, font_height, space_char, space_line, row0_x, row0_y, \
           display_rows, display_cols, currentScreen, batteryVoltage

    for i in range(0, display_rows):
        fieldIndex = (currentScreen * display_rows) + i

        if fieldIndex >= len(displayData):
            break

        field_areas[i].text = ""

        field_areas[i].text = displayData[fieldIndex][1]

def updateLabels():
    global displayData, display, splash, label_areas, field_areas, \
           font_width, font_height, space_char, space_line, row0_x, row0_y, \
           display_rows, display_cols, currentScreen

    clearDisplay()

    label_areas.clear()
    field_areas.clear()

    for i in range(0, display_rows):
        labelIndex = (currentScreen * display_rows) + i

        if labelIndex >= len(displayData):
            break

        label_areas.append(setLabel(displayData[labelIndex][0], 0, i))
        field_areas.append(setLabel("", len(displayData[labelIndex][0]) + 1,  i))
        splash.append(label_areas[i])
        splash.append(field_areas[i])

# -------------------------------------

def printDisplayInfo():
    global displayData, display, splash, label_areas, field_areas, \
           font_width, font_height, space_char, space_line, row0_x, row0_y, \
           display_rows, display_cols, currentScreen, batteryVoltage

    print("Display Width = {}".format(display.width))
    print("Display Height = {}".format(display.height))
    print("Display Columns = {}".format(display_cols))
    print("Display Rows = {}".format(display_rows))
    print("Font Width = {}".format(font_width))
    print("Font Height = {}".format(font_height))
