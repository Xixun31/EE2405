import serial
import time

Fs = 128.0  # sampling rate
Ts = 1.0/Fs  # sampling interval
y = []

serdev = '/dev/ttyACM0'
s = serial.Serial(serdev, 115200)

# Read data from the serial port
for x in range(0, int(Fs)):
    line = s.readline()  # Read an echo string from B_L4S5I_IOT01A terminated with '\n'
    y.append(float(line.strip()))

# Save data to a text file
with open('data.txt', 'w') as file:
    for value in y:
        file.write(f"{value}\n")

print(f"Data saved to 'data.txt'")

s.close()

