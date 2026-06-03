#include "mbed.h"

class parallax_laserping {
    public:
        parallax_laserping( DigitalInOut& input ) {
            dio = &input;
            t.reset();
        }

        float laserping(){
            float ret;
            dio->output();
            dio->write(0);
            wait_us(200);
            dio->write(1);
            wait_us(5);
            dio->write(0);
            wait_us(5);
    
            dio->input();
            Timer timeout;
            timeout.start();
            while(dio->read()==0) {
                if (timeout.read_ms() > 50) {
                    return -1.0f;
                }
            }
            t.start();
            while(dio->read()==1) {
                if (timeout.read_ms() > 200) {
                    t.stop();
                    t.reset();
                    return -1.0f;
                }
            }
            ret = t.read();
            t.stop();
            t.reset();
            timeout.stop();
            return ret;
        }

        float laserping_cm(){
            float raw = laserping();
            return raw < 0 ? -1.0f : raw * 17150.0f;
        }
        operator float(){
            float raw = laserping();
            return raw < 0 ? -1.0f : raw * 17150.0f;
        }
    private:
        Timer t;
        DigitalInOut *dio;
};
