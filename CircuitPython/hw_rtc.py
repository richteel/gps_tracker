import gc
import rtc
import time

import dispItems
import helper
import logItems

def begin():
    helper.the_rtc = rtc.RTC()
    rtc.set_time_source(helper.the_rtc)
    helper.the_rtc.datetime = time.struct_time((1970, 1, 1, 0, 0, 0, 3, 1, 0))

def syncClock(gps):
    helper.the_rtc.datetime = gps.datetime

def update():
    # Clear dictonary values
    helper.clearDictionaryValues(dispItems.rtc)
    helper.clearDictionaryValues(logItems.rtc)
    gc.collect()

    dispItems.rtc["datetime"]["value"] = helper.datetimeDisplay(helper.the_rtc)
    logItems.rtc["datetime"]["value"] = helper.datetimeLog(helper.the_rtc)

    if time is not None:
        dispItems.rtc["unixtime"]["value"] = helper.dataToString(time.time())
        logItems.rtc["unixtime"]["value"] = helper.dataToString(time.time())
