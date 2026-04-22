/** lcd.h
 *  \brief mbed library for F769NI  LCD Controller.
 *  \copyright GNU Public License, v2. or later
 *
 * Copyright (C)2025 Jing-Jia Liou. All right reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to:
 *
 * Free Software Foundation, Inc.
 * 51 Franklin St, 5th Floor, Boston, MA 02110-1301, USA
 *
 *********************************************************************/

#include "mbed.h"

#ifndef LCD_H
#define LCD_H

#ifdef __cplusplus
extern "C" {
#endif

#define PORTRAIT            0
#define LANDSCAPE           1

#define LCD_X 800
#define LCD_Y 480
#define TFT_HOR_RES     800
#define TFT_VER_RES     480
#define TFT_NO_TEARING  0    /*1: no tearing but slower*/

class LCD
{
    public:

        LCD();
        void InitLCD(uint8_t orientation = LANDSCAPE);
        void FillRectangle(unsigned int x, unsigned int y, unsigned int w, unsigned int h, uint16_t c);
        //void FillArea(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t* color_p);
        //void DrawPixel(uint16_t color);
        void LCD_Clear(uint16_t j);
        void H_line(unsigned int x, unsigned int y, unsigned int l, uint16_t c);
        void V_line(unsigned int x, unsigned int y, unsigned int l, uint16_t c);
        void Rect(unsigned int x,unsigned int y,unsigned int w,unsigned int h, uint16_t c);
};

#ifdef __cplusplus
}
#endif

#endif
