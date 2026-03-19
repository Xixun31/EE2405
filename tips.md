###compile
ninja MbedCEHelloWorld

###flash
cp MbedCEHelloWorld.bin /Volumes/DIS_L4S5VI/
sync

###serial port
ls /dev/cu.usbmodem*
minicom -D /dev/cu.usbmodem1103 -b 115200
To exit minicom press Esc + X