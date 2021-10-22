import helper
import logItems

import hw_sdcard

items = [
    logItems.rtc["datetime"],
    logItems.gps["lat"],
    logItems.gps["long"],
    logItems.gps["alt"],
    logItems.gps["heading"],
    logItems.gps["speed"],
    logItems.gps["satellites"],
    logItems.gps["fixQ"],
    logItems.gps["fixQ3d"]
]

def getLogFileName():
    fileDate = helper.datetimeFileName(helper.the_rtc)

    if len(fileDate) == 0:
        return ""

    return "gps {}.txt".format(fileDate)

def log():
    if not hw_sdcard.isReady():
        return

    fileName = getLogFileName()

    if len(fileName) == 0:
        return

    writeHeader(fileName)
    writeData(fileName)

def writeData(fileName):
    rowText = ""

    for item in items:
        if len(rowText) > 0:
            rowText += "\t"

        rowText += item["value"]

    hw_sdcard.writeToFile(fileName, rowText)

def writeHeader(fileName):
    if hw_sdcard.doesFileExist(fileName):
        return

    headerText = ""

    for item in items:
        if len(headerText) > 0:
            headerText += "\t"

        headerText += item["label"]

    hw_sdcard.writeToFile(fileName, headerText)

