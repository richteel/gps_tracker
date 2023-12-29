# gps_tracker
GPS Tracker and data logger

The files here include hardware, case, PCB, Python and C++ code, documentation, and reference files.


## Bill of Materials (BOM) ##
- Adafruit Feather RP2040
	- Qty: 1
	- Source: Adafruit [https://www.adafruit.com/product/4884](https://www.adafruit.com/product/4884 "Adafruit Feather RP2040")
- Adafruit Micro SD Breakout
	- Qty: 1
	- Source: Adafruit [https://www.adafruit.com/product/4682](https://www.adafruit.com/product/4682 "Adafruit Micro SD SPI or SDIO Card Breakout Board - 3V ONLY!")
- SPDT Slide Switch
	- Qty: 2
	- Source: Adafruit [https://www.adafruit.com/product/805](https://www.adafruit.com/product/805 "SPDT Slide Switch")
- Tactile Switch Push Button (6mm slim)
	- Qty: 4
	- Source: Adafruit [https://www.adafruit.com/product/1489](https://www.adafruit.com/product/1489 "Tactile Switch Push Button (6mm slim)")
- ST-PH 2-Pin SMT Right Angle Connector
	- Qty: 1
	- Source: Adafruit [https://www.adafruit.com/product/1769](https://www.adafruit.com/product/1769 "ST-PH 2-Pin SMT Right Angle Connector")
- GPS Receiver - GP-635T (50 Channel)
	- Qty: 1
	- Source: Sparkfun [https://www.sparkfun.com/products/retired/11571](https://www.sparkfun.com/products/retired/11571 "GPS Receiver - GP-635T (50 Channel)")
- JST SH Jumper 6 Wire Assembly - 8"
	- Qty: 1
	- Source: Sparkfun [https://www.sparkfun.com/products/10361](https://www.sparkfun.com/products/10361 "JST SH Jumper 6 Wire Assembly - 8 inch")
- I2C 128x64 OLED Display
	- Qty: 1
	- Source: Amazon [https://www.amazon.com/gp/product/B08VNRH5HR/ref=ppx_yo_dt_b_search_asin_title?ie=UTF8&th=1](https://www.amazon.com/gp/product/B08VNRH5HR/ref=ppx_yo_dt_b_search_asin_title?ie=UTF8&th=1 "I2C 128x64 OLED Display")
- 3.7V 680mAh 102530 Lipo Battery Rechargeable Lithium Polymer ion Battery Pack with JST Connector
	- Qty: 1
	- Source: Amazon [https://www.amazon.com/gp/product/B07BTPNS2B/ref=ppx_yo_dt_b_search_asin_title?ie=UTF8&psc=1](https://www.amazon.com/gp/product/B07BTPNS2B/ref=ppx_yo_dt_b_search_asin_title?ie=UTF8&psc=1 "3.7V 680mAh 102530 Lipo Battery Rechargeable Lithium Polymer ion Battery Pack with JST Connector")
	- **WARNING:** The battery from this source has reversed polarity. Change the pins to match correct orientation!
- CR2032 Coin Button Cell Battery Holder Case
	- Qty: 1
	- Source: Amazon [https://www.amazon.com/gp/product/B06XF3K4NP/ref=ppx_yo_dt_b_search_asin_image?ie=UTF8&psc=1](https://www.amazon.com/gp/product/B06XF3K4NP/ref=ppx_yo_dt_b_search_asin_image?ie=UTF8&psc=1 "CR2032 Coin Button Cell Battery Holder Case")
- CR2032 Battery
	- Qty: 1
	- Source: Battery Junction [https://www.batteryjunction.com/sony-cr2032-5pack](https://www.batteryjunction.com/sony-cr2032-5pack "CR2032 Battery")
- Resistor 4.7K Ω (0805 package)
	- Qty: 4
	- Source: DigiKey [https://www.digikey.com/en/products/filter/chip-resistor-surface-mount/52...](https://www.digikey.com/en/products/filter/chip-resistor-surface-mount/52?s=N4IgjCBcoEwAwA4CsVQGMoDMCGAbAzgKYA0IA9lANogAsAdAOwAEA1gPIAWAtviALqkADgBcoIAKoA7AJbC2mALKFs%2BAK4AnQiAC%2BpALQxUIDJGHrVJclRAo%2BukHoCcRk2YukKkahDv3DXkEQ4JCZNfGl8YTJ1flIUaBBBKDAhJMgYJHsANiNpABMxPTA4CCFRSBAQUmEAT0EtCpUMbW0gA ""Filter for Resistor 4.7K Ω (0805 package)")
- Resistor 2M Ω (0805 package)
	- Qty: 4
	- Source: DigiKey [https://www.digikey.com/en/products/filter/chip-resistor-surface-mount/52...](https://www.digikey.com/en/products/filter/chip-resistor-surface-mount/52?s=N4IgjCBcoEwAwA4CsVQGMoDMCGAbAzgKYA0IA9lANogwAEAsgPIAWAtviALqkAOALlBABVAHYBLPo0z1C2fAFcAToRABfUgFoYqEBkh9F8kuSogUndSA0BOHXoNHSFSNQgXL2lyERwktZfhi%2BHxkilykKNAgPFBgvDGQMEiWAGw6YgAmghpgcBC8ApAgIKR8AJ48KkVyGKqqQA ""Filter for Resistor 2M Ω (0805 package)") 
- Capacitor 0.1μF (0805 package)
	- Qty: 2
	- Source: DigiKey [https://www.digikey.com/en/products/filter/ceramic-capacitors/60...](https://www.digikey.com/en/products/filter/ceramic-capacitors/60?s=N4IgjCBcoEwAwBYCcVQGMoDMCGAbAzgKYA0IA9lANohwB0YABAK0BiIAuqQA4AuUIAVQB2ASx4B5TAFlC2fAFcAToRABfUgFoYqEBkg9F8kuSogArB3UgNKaLqgGjpCpGoR2qq9tc0AHHDMGAGFsLmw0MTJFDlIANh0RABN%2BDTA4CG4%2BSBAQUh4ATy4VbLkMTyA "Filter for Capacitor 0.1μF (0805 package)")
- PCB
	- Qty: 1
	- Source: Shared on OSH Park [https://oshpark.com/shared_projects/GzuBZVdm](https://oshpark.com/shared_projects/GzuBZVdm "PCB")


# Log File #

The GPS Log Files are named with the format of YYYYMMDD.txt, where the letters are the following:

- YYYY: 4-digit Year
- MM: 2-digit Month
- DD: 2-digit Day

The data is comma separated, with the following columns.

- ISO 8601 formatted time-stamp (UTC) e.g. 2009-10-17T18:37:26Z
- Latitude in Degrees
- Longitude in Degrees
- Elevation in Meters
- Heading in Degrees
- Speed in Kpm

