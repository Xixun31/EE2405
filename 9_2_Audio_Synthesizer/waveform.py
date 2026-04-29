import numpy as np
import serial
import time

# generate the sine waveform table
waveformLength = 128
t = np.linspace(0, 2*np.pi, waveformLength)
waveformTable = (np.sin(t) + 1.0) / 2.0

# output formatter
formatter = lambda x: "%.3f" % x

# send the waveform table to mbed
serdev = '/dev/ttyACM0'
s = serial.Serial(serdev)
print("Sending waveform.")

i=0
for data in waveformTable:
    s.write(bytes(formatter(data), 'UTF-8'))
    i=i+1
    print("Sent " + str(i) + " data.")
    time.sleep(0.001)
print("Waveform sent.")

# receive the wavefrom from mbed
while(s.readable()):
    line=s.readline()
    print(line.decode("utf-8"))
    line=line.rstrip(b'\r\n')
    if(line==b'end'):
        break

s.close()
