//Display ADC input data
#include "mbed.h"
#include "TextLCD.h"

TextLCD lcd(D2, D3, D4, D5, D6, D7);
AnalogIn Ain(A0);

int main()
{
      float percentage;
      int D;
      while (1)
      {
            percentage = Ain * 100;
            D = int(percentage);
            float B = percentage - D;
            int C = B * 1000000;

            lcd.printf("%d.", D);
            lcd.printf("%d", C);
            ThisThread::sleep_for(250ms);
            lcd.cls();
      }
}