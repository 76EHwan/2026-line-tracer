#include "st7789.h"

extern uint16_t ST7789_WIDTH_R;
extern uint16_t ST7789_HEIGHT_R;

#if ST7789_ROTATION == 0 || ST7789_ROTATION == 2
uint16_t ST7789_WIDTH_R = ST7789_WIDTH;
uint16_t ST7789_HEIGHT_R = ST7789_HEIGHT;
#else
uint16_t ST7789_WIDTH_R = ST7789_HEIGHT;
uint16_t ST7789_HEIGHT_R = ST7789_WIDTH;
#endif

#ifndef ST7789_CS_PORT
#define ST7789_Select()
#define ST7789_UnSelect()
#else
#define ST7789_Select()    HAL_GPIO_WritePin(ST7789_CS_PORT, ST7789_CS_PIN, GPIO_PIN_RESET)
#define ST7789_UnSelect()  HAL_GPIO_WritePin(ST7789_CS_PORT, ST7789_CS_PIN, GPIO_PIN_SET)
#endif

static void ST7789_WriteCommand(uint8_t cmd) {
	ST7789_Select();
	HAL_GPIO_WritePin(ST7789_DC_PORT, ST7789_DC_PIN, GPIO_PIN_RESET);
	HAL_SPI_Transmit(&ST7789_SPI_PORT, &cmd, 1, HAL_MAX_DELAY);
	ST7789_UnSelect();
}

static void ST7789_WriteSmallData(uint8_t data) {
	ST7789_Select();
	HAL_GPIO_WritePin(ST7789_DC_PORT, ST7789_DC_PIN, GPIO_PIN_SET);
	HAL_SPI_Transmit(&ST7789_SPI_PORT, &data, 1, HAL_MAX_DELAY);
	ST7789_UnSelect();
}

static void ST7789_WriteData(uint8_t *buff, size_t buff_size) {
	ST7789_Select();
	HAL_GPIO_WritePin(ST7789_DC_PORT, ST7789_DC_PIN, GPIO_PIN_SET);
	HAL_SPI_Transmit(&ST7789_SPI_PORT, buff, buff_size, HAL_MAX_DELAY);
	ST7789_UnSelect();
}

static void ST7789_SetAddressWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) {
	uint16_t x_start = x0, x_end = x1;
	uint16_t y_start = y0, y_end = y1;

	/* 1.14인치 135x240 해상도에 대한 방향별 물리 메모리 오프셋 보정 */
#if ST7789_ROTATION == 0
	x_start += 52; x_end += 52;
	y_start += 40; y_end += 40;
#elif ST7789_ROTATION == 1
	x_start += 40; x_end += 40;
	y_start += 53; y_end += 53;
#elif ST7789_ROTATION == 2
	x_start += 53; x_end += 53;
	y_start += 40; y_end += 40;
#elif ST7789_ROTATION == 3
	x_start += 40; x_end += 40;
	y_start += 52; y_end += 52;
#endif

	ST7789_WriteCommand(0x2A);
	ST7789_WriteSmallData(x_start >> 8);
	ST7789_WriteSmallData(x_start & 0xFF);
	ST7789_WriteSmallData(x_end >> 8);
	ST7789_WriteSmallData(x_end & 0xFF);

	ST7789_WriteCommand(0x2B);
	ST7789_WriteSmallData(y_start >> 8);
	ST7789_WriteSmallData(y_start & 0xFF);
	ST7789_WriteSmallData(y_end >> 8);
	ST7789_WriteSmallData(y_end & 0xFF);

	ST7789_WriteCommand(0x2C);
}

void ST7789_Init(void) {
	HAL_Delay(25);

    /* 1. 하드웨어 핀 대신 소프트웨어 리셋 호출 (필수) */
	ST7789_WriteCommand(0x01); // Software Reset
	HAL_Delay(150);

    /* 2. Sleep Out */
	ST7789_WriteCommand(0x11); // Sleep Out
	HAL_Delay(120);

	ST7789_WriteCommand(0x3A); // Color Mode
	ST7789_WriteSmallData(0x55); // 16-bit RGB565

	ST7789_WriteCommand(0x36); // MADCTL (방향 제어)
#if ST7789_ROTATION == 0
	ST7789_WriteSmallData(0x00);
#elif ST7789_ROTATION == 1
	ST7789_WriteSmallData(0xA0);
#elif ST7789_ROTATION == 2
	ST7789_WriteSmallData(0xC0);
#elif ST7789_ROTATION == 3
	ST7789_WriteSmallData(0x60);
#endif

	ST7789_WriteCommand(0x21); // Display Inversion ON (IPS 패널 필수)
	HAL_Delay(10);
	ST7789_WriteCommand(0x13); // Normal Display Mode ON
	HAL_Delay(10);
	ST7789_WriteCommand(0x29); // Display ON
	HAL_Delay(120);

    /* 화면 초기화 */
	ST7789_Fill_Color(BLACK);
}

void ST7789_Fill_Color(uint16_t color) {
	uint16_t i, j;
	uint8_t data[2] = {color >> 8, color & 0xFF};
	ST7789_SetAddressWindow(0, 0, ST7789_WIDTH_R - 1, ST7789_HEIGHT_R - 1);
	for (i = 0; i < ST7789_WIDTH_R; i++) {
		for (j = 0; j < ST7789_HEIGHT_R; j++) {
			ST7789_WriteData(data, 2);
		}
	}
}

void ST7789_DrawPixel(uint16_t x, uint16_t y, uint16_t color) {
	if ((x >= ST7789_WIDTH_R) || (y >= ST7789_HEIGHT_R)) return;
	uint8_t data[2] = {color >> 8, color & 0xFF};
	ST7789_SetAddressWindow(x, y, x, y);
	ST7789_WriteData(data, 2);
}

void ST7789_DrawFilledRectangle(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color) {
	if ((x >= ST7789_WIDTH_R) || (y >= ST7789_HEIGHT_R)) return;
	if ((x + w - 1) >= ST7789_WIDTH_R) w = ST7789_WIDTH_R - x;
	if ((y + h - 1) >= ST7789_HEIGHT_R) h = ST7789_HEIGHT_R - y;

	uint8_t data[2] = {color >> 8, color & 0xFF};
	ST7789_SetAddressWindow(x, y, x + w - 1, y + h - 1);
	for (uint32_t i = 0; i < (uint32_t)(w * h); i++) {
		ST7789_WriteData(data, 2);
	}
}

void ST7789_DrawImage(uint16_t x, uint16_t y, uint16_t w, uint16_t h, const uint16_t *data) {
	if ((x >= ST7789_WIDTH_R) || (y >= ST7789_HEIGHT_R)) return;
	ST7789_SetAddressWindow(x, y, x + w - 1, y + h - 1);
	ST7789_WriteData((uint8_t *)data, sizeof(uint16_t) * w * h);
}

void ST7789_WriteChar(uint16_t x, uint16_t y, char ch, FontDef font, uint16_t color, uint16_t bgcolor) {
	uint32_t i, b, j;
	ST7789_SetAddressWindow(x, y, x + font.width - 1, y + font.height - 1);
	for (i = 0; i < font.height; i++) {
		b = font.data[(ch - 32) * font.height + i];
		for (j = 0; j < font.width; j++) {
			if ((b << j) & 0x8000) {
				uint8_t data[2] = {color >> 8, color & 0xFF};
				ST7789_WriteData(data, 2);
			} else {
				uint8_t data[2] = {bgcolor >> 8, bgcolor & 0xFF};
				ST7789_WriteData(data, 2);
			}
		}
	}
}

void ST7789_WriteString(uint16_t x, uint16_t y, const char *str, FontDef font, uint16_t color, uint16_t bgcolor) {
	while (*str) {
		if (x + font.width >= ST7789_WIDTH_R) {
			x = 0;
			y += font.height;
			if (y + font.height >= ST7789_HEIGHT_R) break;
			if (*str == ' ') { str++; continue; }
		}
		ST7789_WriteChar(x, y, *str, font, color, bgcolor);
		x += font.width;
		str++;
	}
}
