import time

import helper

import hw_battery
import hw_display
import hw_gps
import hw_rtc
import hw_sdcard
import hw_switches

import dispItems
import log_battery
import log_gps

import gc

timers = {
    "batteryUpdate" : {"last_update": time.monotonic(), "updateInterval": 1},
    "displayUpdate" : {"last_update": time.monotonic(), "updateInterval": 1},
    "rtcUpdate" : {"last_update": time.monotonic(), "updateInterval": 1},
    "sdCardUpdate" : {"last_update": time.monotonic(), "updateInterval": 1},
    "timeSync" : {"last_update": time.monotonic(), "updateInterval": 300},
    "test" : {"last_update": time.monotonic(), "updateInterval": 10},
    "batteryLog" : {"last_update": time.monotonic(), "updateInterval": 5},
    "gpsLog" : {"last_update": time.monotonic(), "updateInterval": 1}
}

debug = False

def pollButtons():
    b = 0
    while b < hw_switches.getSwitchCount():
        if hw_switches.getSwitchState(b):
            print("Button {} pressed".format(b + 1))
            # write code here to handle button presses
            if b == 0:
                hw_display.toggleEnable()
            elif b == 1:
                hw_display.changeScreen(-1)
            elif b == 2:
                hw_display.changeScreen(1)
            elif b == 3:
                hw_sdcard.toggleEnable()
                hw_gps.toggleEnable()

        b += 1

hw_battery.begin()
hw_gps.begin()
hw_sdcard.begin()
hw_rtc.begin()
hw_display.begin()
hw_switches.begin()

minFreeMemory = gc.mem_free()

while True:
    hw_gps.update()

    # HANDLE SWITCHES
    pollButtons()

    memFree = gc.mem_free()
    if memFree < minFreeMemory:
        minFreeMemory = memFree
        dispItems.info["minMemory"]["value"] = \
            helper.dataToString(minFreeMemory, "{:,}")
        dispItems.info["minMemoryDT"]["value"] = \
            helper.datetimeDisplay(helper.the_rtc, False)

    current = time.monotonic()

    # SYNC RTC
    if current - timers["timeSync"]["last_update"] >= \
            timers["timeSync"]["updateInterval"] or \
            not helper.timeValid(helper.the_rtc):
        timers["timeSync"]["last_update"] = current

        # if helper.gps.has_fix:
        if helper.gps.datetime:
            # Set the RTC
            hw_rtc.syncClock(helper.gps)
            print("")
            print("****************************")
            print("* RTC Time Sync'd with GPS *")
            print("****************************")

    # UPDATE RTC
    if current - timers["rtcUpdate"]["last_update"] >= \
            timers["rtcUpdate"]["updateInterval"]:
        timers["rtcUpdate"]["last_update"] = current

        hw_rtc.update()
        pollButtons()

    # UPDATE BATTERY
    if current - timers["batteryUpdate"]["last_update"] >= \
            timers["batteryUpdate"]["updateInterval"]:
        timers["batteryUpdate"]["last_update"] = current

        hw_battery.update()
        pollButtons()

    # UPDATE SD CARD
    if current - timers["sdCardUpdate"]["last_update"] >= \
            timers["sdCardUpdate"]["updateInterval"]:
        timers["sdCardUpdate"]["last_update"] = current

        hw_sdcard.update()
        pollButtons()

    # UPDATE DISPLAY
    if current - timers["displayUpdate"]["last_update"] >= \
            timers["displayUpdate"]["updateInterval"]:
        timers["displayUpdate"]["last_update"] = current

        hw_display.update()
        pollButtons()

    # LOG BATTERY
    if current - timers["batteryLog"]["last_update"] >= \
            timers["batteryLog"]["updateInterval"] \
            and helper.timeValid(helper.the_rtc):
        timers["batteryLog"]["last_update"] = current

        log_battery.log()
        pollButtons()

    # LOG GPS
    if current - timers["gpsLog"]["last_update"] >= \
            timers["gpsLog"]["updateInterval"] \
            and helper.timeValid(helper.the_rtc):
        timers["gpsLog"]["last_update"] = current

        log_gps.log()
        pollButtons()

    # Test
    if current - timers["test"]["last_update"] >= \
            timers["test"]["updateInterval"]:
        timers["test"]["last_update"] = current

        if debug:
            print("********** DISPLAY **********")
            for item in hw_display.items:
                print("{} {}".format(item["label"], item["value"]))

            print("********** LOG BATTERY **********")
            for item in log_battery.items:
                print("{} {}".format(item["label"], item["value"]))

            print("********** LOG GPS **********")
            for item in log_gps.items:
                print("{} {}".format(item["label"], item["value"]))

