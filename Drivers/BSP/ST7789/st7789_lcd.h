/*
 * st7789_lcd.h
 *
 *  Created on: 2026. 4. 29.
 *      Author: kth59
 */

#ifndef BSP_ST7789_ST7789_LCD_H
#define BSP_ST7789_ST7789_LCD_H

#include "main.h"
#include "st7789.h"
#include <stdio.h>
#include "SDcard.h"
// #include "user_init.h"  // 필요시 주석 해제

#define POINT_COLOR_DEFAULT  WHITE
#define BACK_COLOR_DEFAULT   BLACK

extern uint16_t LCD7789_POINT_COLOR;
extern uint16_t LCD7789_BACK_COLOR;
extern uint16_t LCD7789_BACK_BRIGHT;

void LCD7789_Test(void);
void LCD7789_SetBrightness(uint32_t Brightness);
uint32_t LCD7789_GetBrightness(void);
void LCD7789_SoftPWMEnable(uint8_t enable);
uint8_t LCD7789_SoftPWMIsEnable(void);
void LCD7789_SoftPWMCtrlInit(void);
void LCD7789_SoftPWMCtrlDeInit(void);
void LCD7789_SoftPWMCtrlRun(void);
void LCD7789_Light(uint32_t Brightness_Dis, uint32_t time);
void LCD7789_Clear(void);
void LCD7789_Printf(uint16_t x, uint16_t y, const char *text, ...);
void LCD7789_Display_Random_BMP_From_SD(const TCHAR *address);


#endif /* BSP_ST7789_ST7789_LCD_H_ */
