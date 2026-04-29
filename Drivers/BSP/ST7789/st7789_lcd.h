/*
 * st7789_lcd.h
 *
 * Created on: 2026. 4. 29.
 * Author: kth59
 */

#ifndef BSP_ST7789_ST7789_LCD_H_
#define BSP_ST7789_ST7789_LCD_H_

#include "main.h"
#include "st7789.h" // ST7735 대신 floyd-fish ST7789 드라이버 포함
#include <stdio.h>
#include "SDcard.h"

/* st7789.h에 없는 추가 색상 정의 (필요시 사용) */
#define POINT_COLOR_DEFAULT  WHITE
#define BACK_COLOR_DEFAULT   BLACK

extern uint16_t POINT_COLOR; // 현재 글자색
extern uint16_t BACK_COLOR;  // 현재 배경색
extern uint16_t BACK_BRIGHT; // 백라이트 밝기

/* 함수 프로토타입 */
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
