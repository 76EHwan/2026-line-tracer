#ifndef __ST7789_H
#define __ST7789_H

#include "main.h"
#include "fonts.h"

/* 1. SPI 포트 설정 (SPI4) */
#define ST7789_SPI_PORT hspi4
extern SPI_HandleTypeDef ST7789_SPI_PORT;

/* * 2. DMA 사용 여부
 * (우선 화면이 정상적으로 나오는지 확인하기 위해 주석 처리하여 안전하게 동작시킵니다.
 * 화면이 나오면 나중에 주석을 풀고 사용하세요.)
 */
// #define USE_DMA

/* 3. 해상도 설정 (1.14인치 135x240) */
#define USING_135X240

/* * 4. 핀 매핑 (매우 중요: 질문자님의 회로 핀 이름으로 교체됨)
 */
#define ST7789_DC_PORT LCD_WR_RS_GPIO_Port
#define ST7789_DC_PIN  LCD_WR_RS_Pin

#define ST7789_CS_PORT LCD_CS_GPIO_Port
#define ST7789_CS_PIN  LCD_CS_Pin

/* 하드웨어 RST 핀이 없으므로 주석 처리합니다. */
// #define ST7789_RST_PORT LCD_RST_GPIO_Port
// #define ST7789_RST_PIN  LCD_RST_Pin

/* 5. 화면 방향 (0, 1, 2, 3 중 선택) */
#define ST7789_ROTATION 2

/* 색상 정의 */
#define WHITE       0xFFFF
#define BLACK       0x0000
#define BLUE        0x001F
#define RED         0xF800
#define MAGENTA     0xF81F
#define GREEN       0x07E0
#define CYAN        0x7FFF
#define YELLOW      0xFFE0
#define GRAY        0X8430

/* 해상도 변수 추출용 */
#ifdef USING_135X240
#define ST7789_WIDTH  135
#define ST7789_HEIGHT 240
#endif

/* 함수 프로토타입 */
void ST7789_Init(void);
void ST7789_Fill_Color(uint16_t color);
void ST7789_DrawImage(uint16_t x, uint16_t y, uint16_t w, uint16_t h, const uint16_t *data);
void ST7789_DrawPixel(uint16_t x, uint16_t y, uint16_t color);
void ST7789_DrawFilledRectangle(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);
void ST7789_WriteChar(uint16_t x, uint16_t y, char ch, FontDef font, uint16_t color, uint16_t bgcolor);
void ST7789_WriteString(uint16_t x, uint16_t y, const char *str, FontDef font, uint16_t color, uint16_t bgcolor);

#endif
