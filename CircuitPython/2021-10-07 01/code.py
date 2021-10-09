import time
import board
import busio
import adafruit_gps
import rtc
import gc
import digitalio

import rjt_buttons
import rjt_log
import rjt_display
import rjt_battery

gc.collect()  # Make Room

# Create a serial connection for the GPS connection using default speed and timeout
uart = busio.UART(board.TX, board.RX, baudrate=9600, timeout=10)

# Create a GPS module instance.
gps = adafruit_gps.GPS(uart, debug=False)  # Use UART/pyserial

# Define GPS Enable Pin
# Set to None if not used
# gps_enable = None
gps_enable = digitalio.DigitalInOut(board.D4)

if gps_enable is not None:
    gps_enable.direction = digitalio.Direction.OUTPUT
    gps_enable.value = True

# Set GPS as time source
rtc.set_time_source(gps)
the_rtc = rtc.RTC()

##################
# START - TIMERS #
##################
# Main loop runs forever update display and log information once per second
last_update = time.monotonic()
updateInterval = 1

# Main loop runs forever update rtc every 5 minutes
last_time_sync = time.monotonic()
timeSyncInterval = 300
rtcTimeSynced = False

# Check the battery every 5 seconds
last_battery_update = time.monotonic()
batteryUpdateInterval = 5

################
# END - TIMERS #
################

rjt_buttons.begin([board.MOSI, board.SCK, board.D25, board.D24])
rjt_log.begin(board.D6, board.D9, board.D10, board.D11, board.D12, board.LED)
#       begin(pinCD, pinCS, pinSCK, pinMOSI, pinMISO)

rjt_battery.begin(board.A0)

# Flags for states
flagUpdateLog = True
flagCharging = False

print("Card Present") if rjt_log.isCardPresent() else print("Card not present")

print("*** Setup Complete - Entering loop ***")

rjt_display.begin()

# rjt_display.test()

rjt_display.updateLabels()


def _format_datetime(datetime):
    return "{:02}/{:02}/{} {:02}:{:02}:{:02}".format(
        datetime.tm_mon,
        datetime.tm_mday,
        datetime.tm_year,
        datetime.tm_hour,
        datetime.tm_min,
        datetime.tm_sec,
    )


def isGPS_Enabled():
    if gps_enable is None:
        return True

    return gps_enable.value


def pollButtons():
    global flagUpdateLog

    b = 0
    while b < rjt_buttons.getButtonCount():
        if rjt_buttons.getButtonState(b):
            print("Button {} pressed".format(b + 1))
            # write code here to handle button presses
            if b == 0:
                rjt_display.sleepWake()
            elif b == 1:
                rjt_display.changeScreen(-1)
            elif b == 2:
                rjt_display.changeScreen(1)
            elif b == 3:
                rjt_log.clearInitFlag()
                flagUpdateLog = not flagUpdateLog
                gps_enable.value = not gps_enable.value

        b += 1

while True:
    # Check the buttons each time
    pollButtons()

    # Update the GPS Data each time
    if isGPS_Enabled():
        gps.update()

    # Handle Timers
    current = time.monotonic()
    # START - Time Sync Timer
    if current - last_time_sync >= timeSyncInterval or not rtcTimeSynced:
        last_time_sync = current

        if gps.has_fix:
            # Set the RTC
            the_rtc.datetime = gps.datetime
            print("")
            print("****************************")
            print("* RTC Time Sync'd with GPS *")
            print("****************************")

            rtcTimeSynced = True

    # Only log data when the interval has lapsed
    if current - last_update >= updateInterval:
        last_update = current

        # if not gps.timestamp_utc:
        if not gps.has_fix:
            print("Waiting for fix....")
        elif gps.datetime.tm_year > 2000 and flagUpdateLog:
            # Only log data if the log is turned on
            # Logging may be turned on or off from the menu
            rjt_log.writeLogEntry(time.localtime(), gps)
            # rjt_display.updateFields(time.localtime(), gps, rjt_log)
            rjt_display.updateDataGps(time.localtime(), gps)

        if flagUpdateLog:
            rjt_log.updateStatus()
            rjt_display.updateDataSdCard(rjt_log)

        rjt_display.updateFields()

    # Update the battery information
    if current - last_battery_update >= batteryUpdateInterval:
        last_battery_update = current

        # Check battery state
        rjt_display.updateDataBattery(rjt_battery.get_voltage())
        isUSB = rjt_display.batteryVoltage > 4

        # time will throw errors until time is synced so only
        # update the display if rtcTimeSynced is true
        if rtcTimeSynced and flagUpdateLog and gps.datetime.tm_year > 2000:
            batteryLogFile = rjt_log.getLogFileNameWithDte(time.localtime(),
                                                           "battery ", ".txt")
            rjt_log.write2LogFile(batteryLogFile,
                                  "Date/Time\tEpoch\tBattery Voltage",
                                  "{}\t{}".format(time.time(),
                                                  rjt_display.batteryVoltage),
                                  time.localtime())

        if isUSB != flagCharging:
            flagCharging = isUSB
            print("Charging = {}".format(flagCharging))
