#include "LCD.h"

int main()
{
      LCD_init();                     // call the initialise function
      display_to_LCD(0x48);           // ‘H’
      display_to_LCD(0x45);           // ‘E’
      display_to_LCD(0x4C);           // ‘L’
      display_to_LCD(0x4C);           // ‘L’
      display_to_LCD(0x4F);           // ‘O’
      display_to_LCD(0x31);
      display_to_LCD(0x31);
      display_to_LCD(0x32);
      display_to_LCD(0x30);
      display_to_LCD(0x33);
      display_to_LCD(0x30);
      display_to_LCD(0x30);
      display_to_LCD(0x33);
      display_to_LCD(0x38);
}