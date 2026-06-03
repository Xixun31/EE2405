#include "mbed.h"
#include "bbcar.h"

Ticker servo_ticker;
Ticker servo_feedback_ticker;
PwmIn servo0_f(D9), servo1_f(D10);
PwmOut servo0_c(D11), servo1_c(D12);
BBCar car(servo0_c, servo0_f, servo1_c, servo1_f, servo_ticker, servo_feedback_ticker);

int main()
{
   while (1)
   {
      car.goStraight(50);
      ThisThread::sleep_for(5s);
      car.stop();
      ThisThread::sleep_for(5s);
   }
}