import adafruit_gps
import busio
import digitalio
import gc

import dispItems
import helper
import logItems
import pins

def begin():
    # Create a serial connection for the GPS connection using default speed and timeout
    uart = busio.UART(pins.gpsTX, pins.gpsRX,
                      baudrate=pins.gpsBaud, timeout=pins.gpsTimeOut)

    # Create a GPS module instance.
    helper.gps = adafruit_gps.GPS(uart, debug=False)  # Use UART/pyserial

    if pins.gpsEnable is not None:
        helper.gpsEnable = digitalio.DigitalInOut(pins.gpsEnable)
        helper.gpsEnable.direction = digitalio.Direction.OUTPUT
        helper.gpsEnable.value = True

def isEnabled():
    if helper.gpsEnable is None:
        return True

    return helper.gpsEnable.value

def knots2kph(speed_kn):
    if speed_kn is None:
        return None

    return speed_kn * 1.852

def setEnable(val):
    if helper.gpsEnable is None:
        return

    helper.gpsEnable.value = val

def toggleEnable():
    setEnable(not isEnabled())

def update():
    # Clear dictonary values
    helper.clearDictionaryValues(dispItems.gps)
    helper.clearDictionaryValues(logItems.gps)
    gc.collect()

    dispItems.info["gpsEnabled"]["value"] = helper.dataToString(isEnabled())

    if isEnabled():
        helper.gps.update()

        dispItems.gps["datetime"]["value"] = helper.datetimeDisplay(helper.gps)
        logItems.gps["datetime"]["value"] = helper.datetimeLog(helper.gps)

        if helper.gps.latitude is not None:
            dispItems.gps["lat"]["value"] = \
                helper.dataToString(abs(helper.gps.latitude),
                                    "{:.4f}",
                                    "S" if helper.gps.latitude < 0 else "N")
            logItems.gps["lat"]["value"] = helper.dataToString(helper.gps.latitude)

        if helper.gps.longitude is not None:
            dispItems.gps["long"]["value"] = \
                helper.dataToString(abs(helper.gps.longitude),
                                    "{:.4f}",
                                    "W" if helper.gps.longitude < 0 else "E")
            logItems.gps["long"]["value"] = helper.dataToString(helper.gps.longitude)

        dispItems.gps["alt"]["value"] = \
            helper.dataToString(helper.gps.altitude_m, "{:,.1f}", "m")
        logItems.gps["alt"]["value"] = helper.dataToString(helper.gps.altitude_m)

        dispItems.gps["heading"]["value"] = \
            helper.dataToString(helper.gps.track_angle_deg, "{:.2f}", "deg")
        logItems.gps["heading"]["value"] = \
            helper.dataToString(helper.gps.track_angle_deg)

        if helper.gps.speed_knots is not None:
            speedKph = knots2kph(helper.gps.speed_knots)
            dispItems.gps["speed"]["value"] = \
                helper.dataToString(speedKph, "{:.3f}", "km/h")
            logItems.gps["speed"]["value"] = helper.dataToString(speedKph)

        dispItems.gps["satellites"]["value"] = \
            helper.dataToString(helper.gps.satellites)
        logItems.gps["satellites"]["value"] = helper.dataToString(helper.gps.satellites)

        dispItems.gps["fixQ"]["value"] = \
            helper.dataToString(helper.gps.fix_quality_3d, "{}", "(3D)")
        logItems.gps["fixQ"]["value"] = helper.dataToString(helper.gps.fix_quality)

        logItems.gps["fixQ3d"]["value"] = helper.dataToString(helper.gps.fix_quality_3d)

