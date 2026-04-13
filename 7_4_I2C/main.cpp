#include "mbed.h"

#define MAXIMUM_BUFFER_SIZE 6

static DigitalOut led1(LED1); // led1 = PA_5
static DigitalOut led2(LED2); // led2 = PB_14

Thread thread1;
Thread thread2;

I2C i2c_m(D14, D15); //SDA, SCL
I2CSlave i2c_s(PC_1, PC_0); //SDA, SCL
char slaveaddr = 0x90;

void master_thread() {
  char buf1[MAXIMUM_BUFFER_SIZE] = {'0', '1', '2', '0', '1', '2'};
  printf("Blinking LED1 and LED2 in order twice\n");
  for (int i = 0; i < 6; i++) {
    i2c_m.write(slaveaddr, &buf1[i], 1);
    ThisThread::sleep_for(1s);
  }
  printf("Waiting for command from terminal. 0: turn off both. 1: turn on LED1. 2: turn on LED2.\n");
  
  while(1){
    char input;
    scanf("%c", &input);
    i2c_m.write(slaveaddr, &input, 1);
  }
  
}

void LED_command(char input){
  if (input == '1') {
    led1 = 1;
    led2 = 0;
  } else if (input == '2') {
    led1 = 0;
    led2 = 1;
  } else {
    led1 = 0;
    led2 = 0;
  }
 
}

void slave_thread() {
  led1 = 0;
  led2 = 0;
  char buf2[MAXIMUM_BUFFER_SIZE] = {0}; // Initialize buffer with zeros.
  i2c_s.address(slaveaddr);
  while (1) {
    int status = i2c_s.receive();
    switch (status) {
      case I2CSlave::ReadAddressed: // no implementation
          break;
      case I2CSlave::WriteGeneral:
          i2c_s.read(buf2, 1);   //read one byte from master
          LED_command(buf2[0]);
          printf("Read General: %s\n", buf2);
          buf2[0]=0; //clear buf[0]
          break;
      case I2CSlave::WriteAddressed:
          i2c_s.read(buf2, 1);   //same case; read one byte from master
          LED_command(buf2[0]);
          printf("Read Addressed: %s\n", buf2);
          buf2[0]=0; //clear buf[0]
          break;
        }
  }
}

int main() {
  i2c_m.frequency(100000); //Frequency has to be 100000, 400000, 1000000
  i2c_s.frequency(100000);
  thread1.start(master_thread);
  thread2.start(slave_thread);
}
