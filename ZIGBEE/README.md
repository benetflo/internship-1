# flash_zigbee_firmware.py USAGE

## This program was developed on a Windows machine using PowerShell.
## It connects via SSH to a CM5 module running Raspberry Pi OS Lite, which serves as the host device.
## The target Zigbee module, an MGM210PB221JIA, is connected to the CM5 through the serial interface /dev/ttyAMA3

### First edit the file and change the constant variables at the top to fit your setup.

``python flash_zigbee_firmware.py``

### or using python3 depending on your python version.

``python3 flash_zigbee_firmware.py``

### Now wait for the script to either finish resetting the chip (if unsuccessful) OR the probe to succesfully finish before pressing (CTRL + C)

### Have fun :)

