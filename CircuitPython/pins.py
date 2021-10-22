import board

# Battery
battVoltage = board.A0
usbDetect = board.D5

# Display
displayWidth = 128
displayHeight = 64
displayScl = board.SCL
displaySda = board.SDA
displayI2cAddress = 0x3C

# GPS
gpsTX = board.TX
gpsRX = board.RX
gpsBaud = 9600
gpsTimeOut = 10
gpsEnable = board.D4

# SD Card
sdCardCD = board.D6
sdCardCS = board.D9
sdCardSCK = board.D10
sdCardMOSI = board.D11
sdCardMISO = board.D12
sdCardLED = board.LED

# Switches
switchPins = [board.MOSI, board.SCK, board.D25, board.D24]
