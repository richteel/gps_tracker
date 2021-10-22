import busio
import displayio
import gc
import terminalio
from adafruit_display_text import label
import adafruit_displayio_ssd1306
import math
import adafruit_imageload

import dispItems
import helper
import pins

items = [
    dispItems.info["version"],
    dispItems.rtc["datetime"],
    dispItems.gps["lat"],
    dispItems.gps["long"],
    dispItems.gps["alt"],
    dispItems.gps["heading"],
    dispItems.gps["speed"],
    dispItems.gps["satellites"],
    dispItems.gps["fixQ"],
    dispItems.battery["voltage"],
    dispItems.battery["percent"],
    dispItems.battery["status"],
    dispItems.sdcard["cardPresent"],
    dispItems.sdcard["cardSizeMb"],
    dispItems.sdcard["cardFreeMb"],
    dispItems.sdcard["cardFiles"],
    dispItems.info["minMemory"],
    dispItems.info["minMemoryDT"],
    dispItems.info["gpsEnabled"],
    dispItems.info["sdcardEnabled"]
]

def begin():
    displayio.release_displays()

    i2c = busio.I2C(pins.displayScl, pins.displaySda)
    display_bus = displayio.I2CDisplay(i2c, device_address=pins.displayI2cAddress)
    helper.display = \
        adafruit_displayio_ssd1306.SSD1306(display_bus, width=pins.displayWidth,
                                           height=pins.displayHeight)
    # Make the display context
    helper.splash = displayio.Group()

    helper.font_width, helper.font_height = \
        terminalio.FONT.get_bounding_box()
    helper.font_width = helper.font_width - helper.space_char
    helper.font_height = helper.font_height - helper.space_line
    helper.display_cols = \
        math.floor((pins.displayWidth + helper.space_char) /
                   (helper.font_width + helper.space_char))
    helper.display_rows = \
        math.floor((pins.displayHeight + helper.space_line) /
                   (helper.font_height + helper.space_line))

    helper.rowOffsetX = math.floor(helper.space_char/2)
    helper.rowOffsetY = math.floor(helper.space_line/2)

    changeScreen(0)

def batteryIconInit():
    # Add Battery Icon
    # Load the sprite sheet (bitmap)
    batterySpriteSheet, batterySpritePallet = \
        adafruit_imageload.load("/battery.bmp",
                                bitmap=displayio.Bitmap,
                                palette=displayio.Palette)

    # Create a sprite (tilegrid)
    helper.battSprite = \
        displayio.TileGrid(batterySpriteSheet,
                           pixel_shader=batterySpritePallet,
                           width=1,
                           height=1,
                           tile_width=16,
                           tile_height=8)

    helper.battSprite.x = 112
    helper.battSprite.y = 0

    helper.splash.append(helper.battSprite)

def changeScreen(incValue):
    helper.currentScreen = helper.currentScreen + incValue
    fieldCount = len(items)

    if helper.currentScreen < 0:
        helper.currentScreen = \
            math.floor((fieldCount - 1) / helper.display_rows)
    elif helper.currentScreen > (fieldCount - 1) / helper.display_rows:
        helper.currentScreen = 0

    changeScreenLabels()
    batteryIconInit()
    update()

def changeScreenLabels():
    helper.label_areas.clear()
    helper.field_areas.clear()
    gc.collect()

    helper.splash = displayio.Group()

    for i in range(0, helper.display_rows):
        labelIndex = (helper.currentScreen * helper.display_rows) + i

        if labelIndex >= len(items):
            break

        helper.label_areas.append(setLabel(items[labelIndex]["label"], 0, i))
        helper.field_areas.append(setLabel("", len(items[labelIndex]["label"]) + 1, i))
        helper.splash.append(helper.label_areas[i])
        helper.splash.append(helper.field_areas[i])

def isEnabled():
    return helper.display.is_awake

def setEnable(val):
    if val:
        helper.display.wake()
    else:
        helper.display.sleep()

def setLabel(text, column, row):
    if row > (helper.display_rows - 1):
        raise NotImplementedError("Only {} rows supported".format(helper.display_rows))

    x = ((helper.font_width + helper.space_char) * column) + helper.rowOffsetX
    y = ((helper.font_height + helper.space_line) * row) + helper.rowOffsetY

    text_area = label.Label(terminalio.FONT, text=text, color=0xFFFF00, x=x, y=y)

    return text_area

def toggleEnable():
    setEnable(not isEnabled())

def update(test=False):
    updateText()
    updateBatteryIcon()
    helper.display.show(helper.splash)

    if test:
        print("********************")

        for item in items:
            print("{} {}".format(item["label"], item["value"]))

        print("********************")

def updateBatteryIcon():
    idx = 0

    if helper.isCharged:
        idx = 6
    elif helper.isCharging and helper.lastBattIconIdx != 5:
        idx = 5 if helper.lastBattIconIdx == 4 else 4
    elif helper.battPercent > 0.66:
        idx = 3
    elif helper.battPercent > 0.33:
        idx = 2
    elif helper.battPercent > helper.minVoltage:
        idx = 1

    helper.lastBattIconIdx = idx
    helper.battSprite[0] = idx
    gc.collect()

def updateText():
    for i in range(0, helper.display_rows):
        fieldIndex = (helper.currentScreen * helper.display_rows) + i

        if fieldIndex >= len(items):
            break

        helper.field_areas[i].text = items[fieldIndex]["value"]
