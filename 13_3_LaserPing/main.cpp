#include "mbed.h"

DigitalInOut laserping(D12);
Timer t;

int main() {

    float val;

    while(1) {

        //Make laserping signal an output port
        laserping.output();
        laserping.write(0);
        wait_us(200);
        //Send 5us pulse
        laserping.write(1);
        wait_us(5);
        laserping.write(0);
        wait_us(5);

        laserping.input();
        //Measure when the pulse returns
        while(laserping.read()==0);
        t.start();
        while(laserping.read()==1);
        val = t.read();
        printf("LaserPing = %lf cm.\r\n", val*17150.0f);
        t.stop();
        t.reset();

        ThisThread::sleep_for(1s);
    }
}