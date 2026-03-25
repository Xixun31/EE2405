#include "mbed.h"

AnalogOut Aout(D7);
AnalogIn Ain(A0);

int mysample = 256;
int i;

float ADCdata[256];

int main(){
  for (i = 0; i < mysample; i++){
    Aout = Ain;
    ADCdata[i] = Ain;
    ThisThread::sleep_for(1000ms/mysample);
  }
  for (i = 0; i < mysample; i++){
    printf("%f\r\n", ADCdata[i]);
    ThisThread::sleep_for(1ms);
  }
}
