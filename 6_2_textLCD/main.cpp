#include "mbed.h"
#include "TextLCD.h"

TextLCD lcd(D2, D3, D4, D5, D6, D7); // RS, E, DB4-DB7

int main()
{
      lcd.printf("HELLO\n");
      for (char x = 0x00; x <= 0x09; x++)
      { // display numbers 0-9
            lcd.printf("%u", x);
      }
}