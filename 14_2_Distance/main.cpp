#include "mbed.h"
#include "bbcar.h"
Ticker servo_ticker;
Ticker servo_feedback_ticker;

PwmIn servo0_f(D9), servo1_f(D10);
PwmOut servo0_c(D11), servo1_c(D12);
BBCar car(servo0_c, servo0_f, servo1_c, servo1_f, servo_ticker, servo_feedback_ticker);

int main() {
   while(1){
      car.goCertainDistance(61.2);   //6.5*3.14*3

      while(car.checkDistance(1)){
         ThisThread::sleep_for(500ms);
      }
      car.stop();

      ThisThread::sleep_for(3s);
      printf("error distance = %f\n", (car.servo0.targetAngle - car.servo0.angle)*6.5*3.14/360);
   }
}
