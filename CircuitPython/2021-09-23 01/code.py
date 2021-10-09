import time
import board
import busio
import adafruit_gps
import rtc
import gc

import rjt_buttons
import rjt_log
import rjt_display
import rjt_battery

gc.collect()  # Make Room

# Create a serial connection for the GPS connection using default speed and timeout
uart = busio.UART(board.TX, board.RX, baudrate=9600, timeout=10)

# Create a GPS module instance.
gps = adafruit_gps.GPS(uart, debug=False)  # Use UART/pyserial

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

        b += 1


# print("Time\tGPS\tRTC")

q = 0

while True:
    # Update the GPS Data each time
    gps.update()

    # Check the buttons each time
    pollButtons()

    # Sync the RTC with GPS Time when time sync internal has lapsed
    current = time.monotonic()
    if current - last_time_sync >= timeSyncInterval or not rtcTimeSynced:
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

    # Only update display and log data when interval has lapsed
    current = time.monotonic()

    if current - last_update >= updateInterval:
        last_update = current

        # print("Free Memory: {:,}".format(gc.mem_free()))
        # print((gc.mem_free(), ))

        battVoltage = rjt_battery.get_voltage()
        isUSB = battVoltage > 2

        if q > 20:
            print("voltage = {}".format(battVoltage))
            q = 0

        q += 1

        if isUSB != flagCharging:
            flagCharging = isUSB
            print("Charging = {}".format(flagCharging))

        # if not gps.timestamp_utc:
        if not gps.has_fix:
            print("Waiting for fix....")
            continue
        # print(
        #    "{}\t{}\t{}".format(
        #        _format_datetime(time.localtime()),
        #        _format_datetime(gps.timestamp_utc),
        #        _format_datetime(the_rtc.datetime),
        #    )
        # )

        if gps.datetime.tm_year < 2000:
            continue

        if flagUpdateLog:
            rjt_log.writeLogEntry(gps, time.localtime())

        # print("")
