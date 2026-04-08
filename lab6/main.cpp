#include "LCD.h"

int main()
{
      LCD_init();                     // call the initialise function

      while (1)
      {
            display_to_LCD(0x48);           // ‘H’
            display_to_LCD(0x45);           // ‘E’
            display_to_LCD(0x4C);           // ‘L’
            display_to_LCD(0x4C);           // ‘L’
            display_to_LCD(0x4F);           // ‘O’
            
            ThisThread::sleep_for(500ms);
            set_location(0x00);

            display_to_LCD(0x31);
            display_to_LCD(0x31);
            display_to_LCD(0x32);
            display_to_LCD(0x30);
            display_to_LCD(0x33);
            display_to_LCD(0x30);
            display_to_LCD(0x30);
            display_to_LCD(0x33);
            display_to_LCD(0x38);

            ThisThread::sleep_for(500ms);
            set_location(0x00);


            display_to_LCD((int)' ');
            display_to_LCD((int)' ');
            display_to_LCD((int)' ');
            display_to_LCD((int)' ');
            display_to_LCD((int)' ');
            display_to_LCD((int)' ');
            display_to_LCD((int)' ');
            display_to_LCD((int)' ');
            display_to_LCD((int)' ');

            ThisThread::sleep_for(20ms);
            set_location(0x00);

      }

      

}