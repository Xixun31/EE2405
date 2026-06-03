#include "mbed.h"
#include "bbcar.h"

DigitalOut led1(LED1);
Ticker servo_ticker;
Ticker servo_feedback_ticker;

PwmIn servo0_f(D9), servo1_f(D10);
PwmOut servo0_c(D11), servo1_c(D12);

DigitalInOut pin8(D8);

BBCar car(servo0_c, servo0_f, servo1_c, servo1_f, servo_ticker, servo_feedback_ticker);

int main() {
   // parallax_ping  ping1(pin8);
   parallax_laserping  ping1(pin8);
   car.goStraight(100);
   while(1) {
      if((float)ping1>15) led1 = 1;
      else {
         led1 = 0;
         car.stop();
         break;
      }
      ThisThread::sleep_for(10ms);
   }
}
