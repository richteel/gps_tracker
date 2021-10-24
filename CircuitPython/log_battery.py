import helper
import logItems

import hw_sdcard

items = [
    logItems.rtc["datetime"],
    logItems.rtc["unixtime"],
    logItems.battery["voltage"],
    logItems.battery["percent"],
    logItems.battery["status"]
]

def getLogFileName():
    fileDate = helper.datetimeFileName(helper.the_rtc)

    if len(fileDate) == 0:
        return ""

    return "battery {}.txt".format(fileDate)

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
    col = 0

    for item in items:
        if col != 0:
            rowText += "\t"

        # First item is always date if it is not present do not write the data
        if col == 0 and (item["value"] is None or len(item["value"]) == 0):
            return

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

