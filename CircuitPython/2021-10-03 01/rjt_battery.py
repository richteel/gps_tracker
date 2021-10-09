from analogio import AnalogIn

analog_in = None

def begin(battPin):
    global analog_in

    analog_in = AnalogIn(battPin)

def get_voltage():
    global analog_in

    return 2 * ((analog_in.value * 3.3) / 65536)
