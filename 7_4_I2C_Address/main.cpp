#include "mbed.h"

#define MAXIMUM_BUFFER_SIZE 6

static DigitalOut led1(LED1); // led1 = PA_5
static DigitalOut led2(LED2); // led2 = PB_14

Thread thread1;
Thread thread2;

I2C i2c_m(D14, D15); //SDA, SCL
I2CSlave i2c_s(PC_1, PC_0); //SDA, SCL
char slaveaddr = 0x90;
char cmdaddr=0x00;
char ledaddr=0x04;

void decode_cmd(char input, char& cmd, char& led){
  switch(input){
    case '0': cmd=0; led='0'; break;
    case '1': cmd=0; led='1'; break;
    case '2': cmd=0; led='2'; break;
    case '3': cmd=1; led='0'; break;
    case '4': cmd=1; led='1'; break;
    case '5': cmd=1; led='2'; break;
    default: cmd=0; led='0';
  }
}

//1. master writes two bytes to set commmand: 0x00+cmd
//2. master writes two bytes to set led: 0x04+led
void master_thread() {
  char buf1[MAXIMUM_BUFFER_SIZE] = {'3', '0', '4', '1', '5', '2'};
  char buf[2]; //I2C buffer
  printf("Blinking LED1 and LED2 in order twice\n");
  for (int i = 0; i < 6; i++) {
    char cmd=0, led=0;
    decode_cmd(buf1[i], cmd, led);
    buf[0]=0x00; buf[1]=cmd;
    i2c_m.write(slaveaddr, buf, 2);
    buf[0]=0x04; buf[1]=led;
    i2c_m.write(slaveaddr, buf, 2);
    ThisThread::sleep_for(1s);
  }
  printf("Waiting for command from terminal.\n 0: turn off both. 1: turn off LED1. 2: turn off LED2.\n 3: turn on both. 4: turn on LED1. 5: turn on LED2.\n");
  
  while(1){
    char input;
    scanf("%c", &input);
    char cmd=0, led=0;
    decode_cmd(input, cmd, led);
    buf[0]=0x00; buf[1]=cmd;
    i2c_m.write(slaveaddr, buf, 2);
    buf[0]=0x04; buf[1]=led;
    i2c_m.write(slaveaddr, buf, 2);
  }
  
}

void LED_command(char cmd, char input){
  if (input == '1') {
    led1 = cmd;
  } else if (input == '2') {
    led2 = cmd;
  } else {
    led1 = cmd;
    led2 = cmd;
  }
 
}

//register 0x00: 0: turn off, 1: turn on
//register 0x04: 0: both leds, 1: led1, 2: led2
void slave_thread() {
  led1 = 0;
  led2 = 0;
  char buf2[MAXIMUM_BUFFER_SIZE] = {0}; // Initialize buffer with zeros.
  char cmd, led;
  i2c_s.address(slaveaddr);
  while (1) {
    int status = i2c_s.receive();
    switch (status) {
      case I2CSlave::ReadAddressed: // no implementation
          break;
      case I2CSlave::WriteGeneral: // we do not differentiate general write
      case I2CSlave::WriteAddressed:
          i2c_s.read(buf2, 2);   //read cmd address+cmd
          if(buf2[0]==0x00){
            cmd=buf2[1];
          } else if (buf2[0]==0x04){
            led=buf2[1];
            LED_command(cmd, led);
          }
          printf("Read Addressed: buf2[0]=0x%02X, buf2[1]=0x%02X\n", buf2[0], buf2[1]);
          buf2[0]=0; buf2[1]=0; //clear buf[0]
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
