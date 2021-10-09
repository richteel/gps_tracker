import time
import board
import busio
import adafruit_gps
import rtc
import gc

import rjt_buttons
import rjt_log

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

rjt_buttons.begin([board.D9, board.D10, board.D11])
rjt_log.begin(board.D24, board.A3, board.A0, board.A1, board.A2)

print("Card Present") if rjt_log.isCardPresent() else print("Card not present")

print("*** Setup Complete - Entering loop ***")


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
    b = 0
    while b < rjt_buttons.getButtonCount():
        if rjt_buttons.getButtonState(b):
            print("Button {} pressed".format(b + 1))
            # write code here to handle button presses
        b += 1


# print("Time\tGPS\tRTC")

while True:
    # Update the GPS Data each time
    gps.update()

    # Check the buttons each time
    pollButtons()

    # Sync the RTC with GPS Time when time sync internal has lapsed
    current = time.monotonic()
    if current - last_time_sync >= timeSyncInterval or not rtcTimeSynced:
        if not gps.has_fix:
            print("(SYNC) Waiting for fix....")
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

        rjt_log.writeLogEntry(gps, time.localtime())

        # print("")