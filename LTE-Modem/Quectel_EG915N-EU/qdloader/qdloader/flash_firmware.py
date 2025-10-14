import subprocess
import os
import sys
import time

QDLOADER_PATH = "out/QDloader"
FIRMWARE_FILE_PATH = "firmware/8915DM_cat1_EG915UEUABR03A01M08.pac" # I created a firmware directory where all firmwares can be placed
DEVICE_PORT = "/dev/ttyUSB0"

count_down = 5

print()
print("===========================================================")
print(f"""Press Enter to begin flashing. Remember to double check that the following are correct:\n
	Firmware_Path: {FIRMWARE_FILE_PATH}\n
	QDloader_Path: {QDLOADER_PATH}\n
	Device_Port: {DEVICE_PORT}
""")
print("===========================================================")
print()

if not os.path.isfile(QDLOADER_PATH):
	sys.exit(f"QDloader not found at {QDLOADER_PATH}")
if not os.path.isfile(FIRMWARE_FILE_PATH):
	sys.exit(f"Firmware file not found at {FIRMWARE_FILE_PATH}")
if not FIRMWARE_FILE_PATH.endswith(".pac"):
	sys.exit(f"Firmware file has to be a '.pac' file")

def flash_firmware(qdloader ,firmware, port):

	try:
		flash_result = subprocess.run([f"sudo", qdloader, "-f", firmware, "-s", port], check=False)

		if flash_result.returncode == 0:
			print()
			print(f"Flash was successful! Firmware was flashed to {port}!")
		else:
			print()
			print(f"Flash was unsuccessful! Firmware was not flashed to {port}!")
	except Exception as e:
		print(f"Error while flashing: {e}")
		sys.exit(1)

while True:
	inp = input()
	if inp == "":
		for i in range(count_down):
			print(f"Flash will start in {count_down - i}. DO NOT INTERRUPT FLASH")
			time.sleep(1)
		flash_firmware(QDLOADER_PATH, FIRMWARE_FILE_PATH, DEVICE_PORT)
		break

