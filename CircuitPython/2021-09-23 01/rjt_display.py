import board
import busio
import displayio
import terminalio
from adafruit_display_text import label
import adafruit_displayio_ssd1306

gpsLabels = [
    "  TeelSys GPS v0.1   ",
    "",
    "  Lat:",
    " Long:",
    "  Alt:",
    " Head:",
    "Speed:",
    "  Sat:",
    " Card:",
    " Type:",
    " Size:",
    "Files:",
]

display = None
splash = None
label_areas = []

def begin(dwidth=128, dheight=64, scl=board.SCL, sda=board.SDA, i2c_address=0x3C):
    global gpsLabels, display, splash

    displayio.release_displays()

    i2c = busio.I2C(scl, sda)
    display_bus = displayio.I2CDisplay(i2c, device_address=i2c_address)
    display = adafruit_displayio_ssd1306.SSD1306(display_bus, width=dwidth,
                                                 height=dheight)
    # Make the display context
    splash = displayio.Group()
    display.show(splash)

def test():
    global gpsLabels, display, label_areas

    color_palette = displayio.Palette(1)
    color_palette[0] = 0xFFFFFF  # White

    for x in range(0, 6):
        print(x)
        label_areas.append(setLabel("123456789012345678901", 0, 0, x))
        splash.append(label_areas[x])

    label_areas[2].text = "ABCDEFGHIJklmnopqrstuvwxyz"

    # text_area = setLabel("123456789012345678901", 0, 0, 1)
    # splash.append(text_area)
    print("Showing Display?")
    # https://learn.adafruit.com/circuitpython-display-support-using-displayio/text

def setLabel(text, maxLen, column, row, inverse=False):
    font_width = 5
    font_height = 8
    space_char = 1
    space_line = 3
    row0_x = 0
    row0_y = 3

    if row > 5:
        raise NotImplementedError("Only 6 rows supported")

    x = ((font_width + space_char) * column) + row0_x
    y = ((font_height + space_line) * row) + row0_y

    text_area = label.Label(terminalio.FONT, text=text, color=0xFFFF00, x=x, y=y)
    return text_area
