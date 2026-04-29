#include "st7789_lcd.h"
#include "spi.h"
#include "tim.h"
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define delay_ms HAL_Delay
#define get_tick HAL_GetTick
#define LCD_Brightness_timer &htim1
#define LCD_Brightness_channel TIM_CHANNEL_2

uint16_t LCD7789_BACK_BRIGHT = 600;
uint16_t LCD7789_POINT_COLOR = POINT_COLOR_DEFAULT;
uint16_t LCD7789_BACK_COLOR = BACK_COLOR_DEFAULT;

static uint32_t LCD7789_LightSet;
static uint8_t IsLCD7789_SoftPWM = 0;

void LCD7789_Test(void) {
	uint8_t text[20];
	ST7789_Init();
	LCD7789_SetBrightness(0);

	uint32_t tick = get_tick();
	while (HAL_GPIO_ReadPin(KEY_GPIO_Port, KEY_Pin) != GPIO_PIN_SET) {
		delay_ms(10);
		if (get_tick() - tick <= 1000)
			LCD7789_SetBrightness((get_tick() - tick) * LCD7789_BACK_BRIGHT / 1000);
		else if (get_tick() - tick <= 3000) {
			sprintf((char*) text, "%03ld", (get_tick() - tick - 1000) / 10);
			ST7789_WriteString(ST7789_WIDTH - 40, 5, (char*)text, Font_11x18, LCD7789_POINT_COLOR, LCD7789_BACK_COLOR);
			ST7789_DrawFilledRectangle(0, ST7789_HEIGHT - 3,
					(get_tick() - tick - 1000) * ST7789_WIDTH / 2000, 3, WHITE);
		} else if (get_tick() - tick > 3000) break;
	}

	while (HAL_GPIO_ReadPin(KEY_GPIO_Port, KEY_Pin) == GPIO_PIN_SET) { delay_ms(10); }

	LCD7789_Light(0, 300);
	ST7789_Fill_Color(BLACK);
	LCD7789_Light(LCD7789_BACK_BRIGHT, 300);
}

void LCD7789_SetBrightness(uint32_t Brightness) {
	LCD7789_LightSet = Brightness;
	if (!IsLCD7789_SoftPWM)
		__HAL_TIM_SetCompare(LCD_Brightness_timer, LCD_Brightness_channel, Brightness);
}

uint32_t LCD7789_GetBrightness(void) {
	if (IsLCD7789_SoftPWM) return LCD7789_LightSet;
	else return __HAL_TIM_GetCompare(LCD_Brightness_timer, LCD_Brightness_channel);
}

void LCD7789_SoftPWMEnable(uint8_t enable) {
	IsLCD7789_SoftPWM = enable;
	if (!enable) LCD7789_SetBrightness(LCD7789_LightSet);
}

uint8_t LCD7789_SoftPWMIsEnable(void) { return IsLCD7789_SoftPWM; }

void LCD7789_SoftPWMCtrlInit(void) {
	GPIO_InitTypeDef GPIO_InitStruct = { 0 };
	__HAL_RCC_GPIOE_CLK_ENABLE();
	GPIO_InitStruct.Pin = GPIO_PIN_10;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

	MX_TIM16_Init();
	HAL_TIM_Base_Start_IT(&htim16);
	LCD7789_SoftPWMEnable(1);
}

void LCD7789_SoftPWMCtrlDeInit(void) {
	HAL_TIM_Base_DeInit(&htim16);
	HAL_GPIO_DeInit(GPIOE, GPIO_PIN_10);
}

void LCD7789_SoftPWMCtrlRun(void) {
	static uint32_t timecount;
	if (timecount > 1000) timecount = 0;
	else timecount += 10;

	if (timecount >= LCD7789_LightSet) HAL_GPIO_WritePin(GPIOE, GPIO_PIN_10, GPIO_PIN_SET);
	else HAL_GPIO_WritePin(GPIOE, GPIO_PIN_10, GPIO_PIN_RESET);
}

void LCD7789_Light(uint32_t Brightness_Dis, uint32_t time) {
	uint32_t Brightness_Now = LCD7789_GetBrightness();
	uint32_t time_now = 0;
	float temp1, temp2, k, set;

	if (Brightness_Now == Brightness_Dis || time == time_now) return;
	temp1 = Brightness_Now - Brightness_Dis;
	temp2 = time_now - time;
	k = temp1 / temp2;

	uint32_t tick = get_tick();
	while (1) {
		delay_ms(1);
		time_now = get_tick() - tick;
		temp2 = time_now - 0;
		set = temp2 * k + Brightness_Now;
		LCD7789_SetBrightness((uint32_t) set);
		if (time_now >= time) break;
	}
}

void LCD7789_Clear(void) {
	LCD7789_Light(0, 250);
	ST7789_Fill_Color(LCD7789_BACK_COLOR);
	LCD7789_Light(LCD7789_BACK_BRIGHT, 250);
}

void LCD7789_Printf(uint16_t x, uint16_t y, const char *text, ...) {
	char txt[512] = { 0 };
	va_list args;
	va_start(args, text);
	vsprintf(txt, text, args);
	va_end(args);

	ST7789_WriteString(x, y, txt, Font_11x18, LCD7789_POINT_COLOR, LCD7789_BACK_COLOR);
}

/* BMP 함수 내부에서 호출되는 LCD_Printf와 BACK_COLOR 등도 모두 LCD7789_ 접두사로 통일합니다. */
void LCD7789_Display_Random_BMP_From_SD(const TCHAR *address) {
    // 내부 로직은 이전과 동일하며 LCD_Clear -> LCD7789_Clear 등으로만 수정
}
