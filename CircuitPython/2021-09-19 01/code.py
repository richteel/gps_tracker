import time
import board
import busio
import adafruit_gps
import rtc

import rjt_buttons
import rjt_log

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


while True:
    # Update the GPS Data each time
    gps.update()

    # Check the buttons each time
    pollButtons()

    # Only update display and log data when interval has elapsed
    current = time.monotonic()
    if current - last_update >= updateInterval:
        last_update = current

        if not gps.timestamp_utc:
            print("No time data from GPS yet")
            continue


        # Set the RTC
        the_rtc.datetime = gps.datetime
        # Time & date from GPS informations
        print("Fix timestamp: {}".format(_format_datetime(gps.timestamp_utc)))

        # Time & date from internal RTC
        print("RTC timestamp: {}".format(_format_datetime(the_rtc.datetime)))

        # Time & date from time.localtime() function
        # local_time = time.localtime(gps.datetime)
        local_time = time.localtime()
        print("Local time: {}".format(_format_datetime(local_time)))

        print("")
