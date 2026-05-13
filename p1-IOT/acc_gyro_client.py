import matplotlib.pyplot as plt
import logging

import erpc
from acc_gyro import *
import sys

# Limit for data points
x_limit = 64

acce_x = []
acce_y = []
acce_z = []
gyro_x = []
gyro_y = []
gyro_z = []

if len(sys.argv) != 2:
    print("Usage: python acc_gyro_client.py <serial port to use>")
    exit()

# Initialize all erpc infrastructure
xport = erpc.transport.SerialTransport(sys.argv[1], 9600)
client_mgr = erpc.client.ClientManager(xport, erpc.basic_codec.BasicCodec)
client = client.acc_gyroServiceClient(client_mgr)

for i in range(x_limit):
    try:
        acce_x_ref = erpc.Reference()
        acce_y_ref = erpc.Reference()
        acce_z_ref = erpc.Reference()
        gyro_x_ref = erpc.Reference()
        gyro_y_ref = erpc.Reference()
        gyro_z_ref = erpc.Reference()

        client.GetAccelerometer(acce_x_ref, acce_y_ref, acce_z_ref)
        client.GetGyro(gyro_x_ref, gyro_y_ref, gyro_z_ref)

        acce_x.append(acce_x_ref.value)
        acce_y.append(acce_y_ref.value)
        acce_z.append(acce_z_ref.value)
        gyro_x.append(gyro_x_ref.value)
        gyro_y.append(gyro_y_ref.value)
        gyro_z.append(gyro_z_ref.value)

        print(
            f"{i+1:02d}: acce=({acce_x_ref.value}, {acce_y_ref.value}, {acce_z_ref.value}), "
            f"gyro=({gyro_x_ref.value}, {gyro_y_ref.value}, {gyro_z_ref.value})"
        )

    except:
        logging.exception("")

# Setup plot
fig, ax = plt.subplots(2, 1)

ax[0].plot(acce_x, label='acce x')
ax[0].plot(acce_y, label='acce y')
ax[0].plot(acce_z, label='acce z')
ax[0].set_title("Accelerometer")
ax[0].legend()
ax[0].set_xlim(0, x_limit)

ax[1].plot(gyro_x, label='gyro x')
ax[1].plot(gyro_y, label='gyro y')
ax[1].plot(gyro_z, label='gyro z')
ax[1].set_title("Gyroscope")
ax[1].legend()
ax[1].set_xlim(0, x_limit)

plt.tight_layout()
plt.show()
