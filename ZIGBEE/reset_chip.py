import time
import signal
import sys
from gpiozero import LED

# GPIO setup using gpiozero
gpio17 = LED(17)  # Example GPIO pin 17 (output)
gpio16 = LED(16)  # Example GPIO reset pin 16 (output)

# Function to perform cleanup when Ctrl+C is pressed
def cleanup_gpio(signal, frame):
    print("Ctrl+C pressed, cleaning up GPIO...")
    gpio17.off()  # Reset GPIO17 to low
    gpio16.on()   # Assert GPIO16 (reset line) high (logic level for reset)
    sys.exit(0)   # Exit the program

# Set GPIO17 to low
gpio17.off()
print("GPIO17 set to LOW")

# Assert reset on GPIO16 (reset the MGM210PB22)
gpio16.off()
print("GPIO16 set to LOW, triggering reset on MGM210PB22")

# Wait to simulate the reset and boot mode (adjust sleep time as necessary)
time.sleep(5)  # Wait for the boot mode to be entered (adjust timing if necessary)

# Set up signal handler for Ctrl+C
signal.signal(signal.SIGINT, cleanup_gpio)

# Step 3: After some time or when ready, exit boot mode and release GPIOs
# This simulates the release of reset and GPIO17
gpio16.on()  # Release reset on GPIO16
print("GPIO16 set to HIGH, releasing reset")

# GPIO17 can be released (set to HIGH) as well if needed:
# gpio17.on()
# print("GPIO17 set to HIGH")

# The program will continue executing its tasks. No blocking, just waiting for exit
# You can place your actual logic here instead of `time.sleep(10)` if needed.
time.sleep(10)  # Simulating waiting for the program to do its work.

# Infinite loop to keep the program running
try:
    while True:
        # Main logic of your program
        print("Running... Press Ctrl+C to exit.")
        time.sleep(1)  # Just waiting here, you can put actual logic here
except KeyboardInterrupt:
    pass  # This is just to ensure cleanup is handled via the signal
