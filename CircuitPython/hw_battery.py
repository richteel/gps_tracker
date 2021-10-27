from analogio import AnalogIn
import digitalio
import gc

import dispItems
import helper
import logItems
import pins

def begin():
    helper.battPin = AnalogIn(pins.battVoltage)
    helper.usbPin = digitalio.DigitalInOut(pins.usbDetect)
    helper.usbPin.direction = digitalio.Direction.INPUT

def get_voltage():
    return 2 * ((helper.battPin.value * 3.3) / 65536)

def get_voltagePercent():
    return (helper.voltage - helper.minVoltage) / \
        (helper.maxVoltage - helper.minVoltage)

def isCharging():
    return helper.usbPin.value

def update():
    # Clear dictonary values
    helper.clearDictionaryValues(dispItems.battery)
    helper.clearDictionaryValues(logItems.battery)
    gc.collect()

    helper.voltage = get_voltage()
    helper.battPercent = get_voltagePercent()
    helper.isCharging = isCharging()
    helper.isCharged = helper.isCharging and \
        (helper.voltage >= helper.maxVoltage or helper.lastChargedState)
    helper.status = helper.batteryStatus_Discharging
    if helper.isCharging:
        helper.status = helper.batteryStatus_Charging
        if helper.isCharged:
            helper.status = helper.batteryStatus_Charged

    helper.lastChargedState = helper.isCharged

    dispItems.battery["voltage"]["value"] = \
        helper.dataToString(helper.voltage, "{:.2f}", "V")
    logItems.battery["voltage"]["value"] = helper.dataToString(helper.voltage)

    dispItems.battery["percent"]["value"] = \
        helper.dataToString(helper.battPercent * 100, "{:.2f}", "%")
    logItems.battery["percent"]["value"] = helper.dataToString(helper.battPercent)

    dispItems.battery["status"]["value"] = helper.dataToString(helper.status)
    logItems.battery["status"]["value"] = helper.dataToString(helper.status)
