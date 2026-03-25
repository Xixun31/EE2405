import serial  # Import the serial module to communicate with serial devices.

# Open the serial port at '/dev/ttyACM0' with a baud rate of 115200
# and simultaneously open 'data.txt' for writing.
with serial.Serial('/dev/ttyACM0', 115200) as s, open('data.txt', 'w') as file:
    s.reset_input_buffer()  # Clear any existing data in the serial input buffer.
    
    for _ in range(128):  # Read and process 128 lines of data.
        line = s.readline().strip().decode()  # Read a line, strip whitespace, and decode bytes to string.
        
        # Check if the line represents a valid number (integer or float).
        # The `replace('.', '')` part allows decimal numbers by removing the dot before checking `isdigit()`.
        if line.replace('.', '').isdigit():
            file.write(line + '\n')  # Write the valid number to the file, followed by a newline.

# Print confirmation message after data is successfully written.
print("Data saved to 'data.txt'")
