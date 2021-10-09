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

# Main loop runs forever update display and log information once per second
last_update = time.monotonic()
updateInterval = 1

# Main loop runs forever update rtc every 5 minutes
last_time_sync = time.monotonic()
timeSyncInterval = 300
rtcTimeSynced = False

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

rjt_display.test()

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
            if b == 3:
                rjt_log.clearInitFlag()
                flagUpdateLog = not flagUpdateLog
                gps_enable.value = not gps_enable.value

        b += 1

while True:
    # Check the buttons each time
    pollButtons()

    # Set a flag to let us know if the update internal has passed
    current = time.monotonic()
    interval_elapsed = current - last_update >= updateInterval

    # If the update interval has passed, reset the last update time
    if interval_elapsed:
        last_update = current

        # Check battery state
        battVoltage = rjt_battery.get_voltage()
        isUSB = battVoltage > 2

        # plot the battery voltage
        print((battVoltage,))

        if isUSB != flagCharging:
            flagCharging = isUSB
            print("Charging = {}".format(flagCharging))

    # Update the GPS Data each time
    if isGPS_Enabled():
        gps.update()
    else:
        continue

    # Check if we need to resync the RTC
    current = time.monotonic()
    if current - last_time_sync >= timeSyncInterval:
        if not gps.has_fix:
            # Printing out too fast seems to lock up Mu Editor
            # print("(SYNC) Waiting for fix....")
            continue
        last_time_sync = current

        # Set the RTC
        the_rtc.datetime = gps.datetime
        print("")
        print("****************************")
        print("* RTC Time Sync'd with GPS *")
        print("****************************")

        rtcTimeSynced = True

    # Only log data when the interval has lapsed
    if interval_elapsed:
        # print("Free Memory: {:,}".format(gc.mem_free()))
        # print((gc.mem_free(), ))

        # if not gps.timestamp_utc:
        if not gps.has_fix:
            print("Waiting for fix....")
            continue

        if gps.datetime.tm_year < 2000:
            continue

        # Only log data if the log is turned on
        # Logging may be turned on or off from the menu
        if flagUpdateLog:
            rjt_log.writeLogEntry(gps, time.localtime())