#include "mbed.h"
#include "stm32f769i_discovery_ts.h"

#ifndef Touch_h
#define Touch_h

#ifdef __cplusplus
extern "C" {
#endif

class Touch
{
    public:

        Touch();

        void    InitTouch();
        bool    DataAvailable();
        int16_t GetX();
        int16_t GetY();
    
    protected:
        TS_StateTypeDef  TS_State;
        int16_t    last_x, last_y;
};

#ifdef __cplusplus
}
#endif

#endif