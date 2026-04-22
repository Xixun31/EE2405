#include "mbed.h"
#include "TextLCD.h"

// 1. 基本設定
static BufferedSerial pc(USBTX, USBRX);
I2C i2c_lcd(D14, D15); 

// 注意：這裡的地址 0x4E 是 7-bit 地址 0x27 左移後的結果
// 如果你的硬體地址是 0x90，請將這裡改為 0x90
TextLCD_I2C lcd(&i2c_lcd, 0x4E, TextLCD::LCD16x2); 

int main()
{
   // 不需要呼叫 LCD_init(); 函式庫已經幫你做好了
   printf("LCD Initialized via Library.\n");

   // --- 重複傳送字元給 Picoscope 觀察 ---
   lcd.cls(); // 清除螢幕
   lcd.locate(0, 0); // 定位到第一行第一格
   
   printf("Starting for-loop for Picoscope verification...\n");
   
   for (int i = 0; i < 100; i++)
   {
      lcd.putc('A'); // 透過 I2C 傳送字元 'A'
      // 故意不加 locate，讓它一直印，或是印滿一整行
      ThisThread::sleep_for(100ms); 
   }
   // ----------------------------------------------

   lcd.printf("\nLoop Done!"); 
}