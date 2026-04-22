#include "mbed.h"
#include "stm32f7xx.h"
#include "stm32f769i_discovery.h"
#include "lcd.h"

LCD lcd;

int main() {
  printf("Initializing LCD\n");
  lcd.InitLCD();
  lcd.LCD_Clear(0xFFF0);
  ThisThread::sleep_for(1s);
  lcd.LCD_Clear(0x0000);
  printf("Writing a few empty rectangles on LCD\n"); 
  for (int i = 0; i < 10; i++) {
    int x = 800-i * (80 + i);
    int y = i * 48;
    int w = 80 ;
    int h = 48;
    uint16_t c = rand();
    lcd.Rect(x, y, w, h, c);
    ThisThread::sleep_for(1s);
  }
  printf("Writing a few filled rectangles on LCD\n");
  for (int i = 0; i < 10; i++) {
    int x = i * (80 + i);
    int y = i * 48;
    int w = 80 + i;
    int h = 48;
    uint16_t c = rand();
    lcd.FillRectangle(x, y, w, h, c);
    ThisThread::sleep_for(1s);
  }
}
