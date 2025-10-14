import serial
import time

PORT = "/dev/ttyUSB6"
BAUDRATE = 9600
TIMEOUT = 5
DELAY_TIME = 1

# OPEN SERIAL PORT
serial_port = serial.Serial(
        port=PORT,
        baudrate=BAUDRATE,
        timeout=TIMEOUT
)

def send_AT_command(command, delay):
        serial_port.write((command + '\r\n').encode())
        time.sleep(delay)
        response = serial_port.read_all().decode()
        return response

try:
        while True:
                cmd = input()
                response = send_AT_command(cmd, DELAY_TIME)
                print(response)
except KeyboardInterrupt:
        pass
finally:
        if serial_port.is_open:
                serial_port.close()
                print("Connection closed")
