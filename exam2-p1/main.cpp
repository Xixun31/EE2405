#include "mbed.h"
#include "LCD.h"

AnalogOut Aout(D13); // channel A
AnalogIn Ain(A0); // AWG input
DigitalOut Dout1(D11); // interupt c1 GPIO pin
DigitalOut Dout2(D12); // interupt c2 GPIO pin
DigitalOut Dout3(D13); // interupt c3 GPIO pin
// InterruptIn c0(D10);
InterruptIn c1(D11); // interupt for (1) waveforms stay at 0V
InterruptIn c2(D12); // interupt for (2) waveform transit from 0V to 2V (peak)
InterruptIn c3(D13); // interupt for (3) waveform transit from 2V to 0V
EventQueue queue(32 * EVENTS_EVENT_SIZE); // to print to LCD out from ISR

Thread thread, t;
Timer timer1, timer2, timer3; // measure the three time characteristics

int mysample = 256;
int i;
uint16_t ADCdata[256];
auto ms = 0;

void ADC_thread() // ADC thread
{
  for (i = 0; i < mysample; i++){
    ADCdata[i] = Ain.read_u16();
    ThisThread::sleep_for(1000ms/mysample);
  }
}

void DAC_thread() // DAC thread
{
  for (i = 0; i < mysample; i++){
    Aout = Ain;
    ThisThread::sleep_for(1000ms/mysample);
  }
}

void python(){ //send to python
  for (i = 0; i < mysample; i++){
    printf("%f\r\n", ADCdata[i]);
    ThisThread::sleep_for(1ms);
  }
}

void led() { //print to lcd
  int time = (int)ms;
  while(time != 0)
  {
    display_to_LCD(time % 10);
    time /= 10;
  }
}

void p1() // Timing interupt 1
{
  timer1.stop();
  ms = chrono::duration_cast<chrono::milliseconds>(timer1.elapsed_time()).count();
  queue.call(led);
  timer2.start();
}

void p2() // Timing interupt 2
{
  timer2.stop();
  ms = chrono::duration_cast<chrono::milliseconds>(timer2.elapsed_time()).count();
  queue.call(led);
  timer3.start();
}

void p3() // Timing interupt 3
{
  timer3.stop();
  ms = chrono::duration_cast<chrono::milliseconds>(timer3.elapsed_time()).count();
  queue.call(led);
}

int main(){
  // init all
  thread.start(ADC_thread);
  thread.start(DAC_thread);
  c1.rise(&p1);
  c2.rise(&p2);
  c3.rise(&p3);
  timer1.start();
  t.start(callback(&queue, &EventQueue::dispatch_forever));
  LCD_init();
  // (1) waveforms stay at 0V
  while(Ain != 0){ }
  display_to_LCD(0x54);
  display_to_LCD(0x31);
  display_to_LCD(0x3A);
  Dout1 = 1;
  // (2) waveform transit from 0V to 2V (peak)
  while(Ain != 2){ }
  // set_location(char ')');
  display_to_LCD(0x54);
  display_to_LCD(0x32);
  display_to_LCD(0x3A);
  Dout2 = 1;
  // (3) waveform transit from 2V to 0V
  while(Ain != 0){ }
  // set_location(char '0');
  display_to_LCD(0x54);
  display_to_LCD(0x33);
  display_to_LCD(0x3A);
  Dout3 = 1;
  // transmit the captured samples to a Python script until the whole cycle
  thread.start(python);

}
