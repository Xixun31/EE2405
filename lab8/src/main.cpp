#include "mbed.h"
#include "stm32f7xx.h"
#include "stm32f769i_discovery.h"
#include "lcd.h"
#include "touch.h"

LCD lcd;
Touch myTouch;

int main() {
   printf("Initializing LCD\n");
   lcd.InitLCD();
   lcd.LCD_Clear(0x0); 
   myTouch.InitTouch();
   uint16_t color=0xa000;
   int16_t x, y;
   while(true){
     if (myTouch.DataAvailable()){
         x = myTouch.GetX();
         y = myTouch.GetY();
         lcd.FillRectangle(x, y, 25, 25, color);
         printf("Touch location: X=%d, Y=%d\n", x, y);
         ThisThread::sleep_for(100ms);
     }
   } 
}