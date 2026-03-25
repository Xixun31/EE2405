import numpy as np
import matplotlib.pyplot as plt

# Load data from the file
filename = 'data.txt'  # Change this to 'data.csv' if you're using a CSV file

# Read data from the file
with open(filename, 'r') as file:
    data = file.readlines()

# Convert the data into a list of floats, ensuring to strip any unwanted whitespace or newline characters
y = []
for line in data:
    try:
        # Strip any leading/trailing whitespace and then convert to float
        value = float(line.strip())  # Strip \n, \r, and any extra spaces
        y.append(value)
    except ValueError:
        # Skip lines that cannot be converted to float (e.g., empty lines)
        continue

# Sampling rate (you should set it to the same rate you used during data collection)
Fs = 128.0  # Example: 128 Hz (adjust accordingly)

# Time vector for the time-domain plot
n = len(y)  # Number of samples
t = np.arange(0, n/Fs, 1/Fs)  # Create a time vector

# Perform FFT
frequencies = np.fft.fftfreq(n, 1/Fs)  # Frequency vector
frequencies = frequencies[:n // 2]  # Use only the positive frequencies (one-sided spectrum)

Y = np.fft.fft(y) / n  # Normalize the FFT output
Y = Y[:n // 2]  # Use only the positive frequencies

# Plot the time-domain and frequency-domain plots
plt.figure(figsize=(12, 6))

# Plot the time-domain signal
plt.subplot(2, 1, 1)  # 2 rows, 1 column, plot 1
plt.plot(t, y, color='blue')
plt.title('Time-Domain Signal')
plt.xlabel('Time [s]')
plt.ylabel('Amplitude')
plt.grid(True)

# Plot the frequency-domain (FFT) response
plt.subplot(2, 1, 2)  # 2 rows, 1 column, plot 2
plt.plot(frequencies, np.abs(Y), color='red')
plt.title('Frequency Response (FFT)')
plt.xlabel('Frequency [Hz]')
plt.ylabel('Amplitude')
plt.grid(True)

# Adjust layout for better spacing
plt.tight_layout()

# Show the plots
plt.show()
