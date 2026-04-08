#include "mbed.h"
#include "TextLCD.h"

DigitalOut led(LED1);      //LED1 = PA_5
TextLCD lcd(D2, D3, D4, D5, D6, D7);

int main()
{
      int x=0;
      lcd.printf("Hello World!\n");
      while(true)
      {
            led = !led;             // toggle led
            lcd.locate(5,1);
            lcd.printf("%5i",x);    //counter display
            ThisThread::sleep_for(1s);
            x++;
      }
}