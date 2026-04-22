
/*********************
 *      INCLUDES
 *********************/
#include "hal_stm_lvgl/touchpad/touch.h"
#include "hal_stm_lvgl/tft/lcd.h"

#include "stm32f7xx.h"
#include "stm32f769i_discovery.h"
#include "stm32f769i_discovery_ts.h"

Touch::Touch(){
}

void Touch::InitTouch()
{
  BSP_TS_Init(TFT_HOR_RES, TFT_VER_RES);
  printf("Touch pad driver initialized.\n"); //debug
}

bool Touch::DataAvailable()
{
    BSP_TS_GetState(&TS_State);
    return TS_State.touchDetected;
}

int16_t Touch::GetX() {
    last_x=TS_State.touchX[0];
    return last_x;
}

int16_t Touch::GetY() {
    last_y=TS_State.touchY[0];
    return last_y;
}
