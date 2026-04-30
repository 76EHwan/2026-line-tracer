#include "st7789.h"
#include "st7789_lcd.h"
#include "spi.h"
#include "tim.h"

#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define TFT135x240  // 또는 TFT240x240 으로 변경하여 사용 가능

#define LCD_RST_SET
#define LCD_RST_RESET
#define LCD_RS_SET      HAL_GPIO_WritePin(LCD_WR_RS_GPIO_Port, LCD_WR_RS_Pin, GPIO_PIN_SET)
#define LCD_RS_RESET    HAL_GPIO_WritePin(LCD_WR_RS_GPIO_Port, LCD_WR_RS_Pin, GPIO_PIN_RESET)
#define LCD_CS_SET      HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_SET)
#define LCD_CS_RESET    HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_RESET)

#define SPI spi4
#define SPI_Drv (&hspi4)
#define delay_ms HAL_Delay
#define get_tick HAL_GetTick

#define LCD_Brightness_timer &htim1
#define LCD_Brightness_channel TIM_CHANNEL_2

static int32_t lcd7789_init(void);
static int32_t lcd7789_gettick(void);
static int32_t lcd7789_writereg(uint8_t reg, uint8_t *pdata, uint32_t length);
static int32_t lcd7789_readreg(uint8_t reg, uint8_t *pdata);
static int32_t lcd7789_senddata(uint8_t *pdata, uint32_t length);
static int32_t lcd7789_recvdata(uint8_t *pdata, uint32_t length);

uint16_t LCD7789_BACK_BRIGHT = 600;

ST7789_IO_t st7789_pIO = { lcd7789_init, 0, 0, lcd7789_writereg,
		lcd7789_readreg, lcd7789_senddata, lcd7789_recvdata, lcd7789_gettick };

ST7789_Object_t st7789_pObj;
void LCD7789_Test(void) {
	uint8_t text[20];

#if defined(TFT135x240)
	ST7789Ctx.Orientation = ST7789_ORIENTATION_PORTRAIT;
	ST7789Ctx.Type = ST7789_135x240_screen;
#elif defined(TFT240x240)
	ST7789Ctx.Orientation = ST7789_ORIENTATION_PORTRAIT;
	ST7789Ctx.Type = ST7789_240x240_screen;
#else
	#error "Unknown Screen"
#endif

	ST7789_RegisterBusIO(&st7789_pObj, &st7789_pIO);
	ST7789_LCD_Driver.Init(&st7789_pObj, ST7789_FORMAT_RBG565, &ST7789Ctx);

	ST7789_LCD_Driver.FillRect(&st7789_pObj, 0, 0, ST7789Ctx.Width,
			ST7789Ctx.Height, BLACK);

	LCD7789_SetBrightness(0);

	LCD7789_Display_Random_BMP_From_SD("/display");


	uint32_t tick = get_tick();
	while (HAL_GPIO_ReadPin(KEY_GPIO_Port, KEY_Pin) != GPIO_PIN_SET) {
		delay_ms(10);

		uint32_t elapsed = get_tick() - tick;

		if (elapsed <= 1000) {
			LCD7789_SetBrightness(elapsed * LCD7789_BACK_BRIGHT / 1000);
		} else if (elapsed <= 3000) {
			LCD7789_SetBrightness(LCD7789_BACK_BRIGHT);

			sprintf((char*) &text, "%03ld", (elapsed - 1000) / 10);
			LCD7789_ShowString(ST7789Ctx.Width - 40, 5, ST7789Ctx.Width, 18, 18,
					text);
			ST7789_LCD_Driver.FillRect(&st7789_pObj, 0, ST7789Ctx.Height - 3,
					(elapsed - 1000) * ST7789Ctx.Width / 2000, 3, 0xFFFF);
		} else if (elapsed > 3000) {
			break;
		}
	}

	while (HAL_GPIO_ReadPin(KEY_GPIO_Port, KEY_Pin) == GPIO_PIN_SET) {
		delay_ms(10);
	}

	LCD7789_Light(0, 300);
	ST7789_LCD_Driver.FillRect(&st7789_pObj, 0, 0, ST7789Ctx.Width,
			ST7789Ctx.Height, BLACK);
	LCD7789_Light(LCD7789_BACK_BRIGHT, 300);
}

static uint32_t LCD7789_LightSet;
static uint8_t IsLCD7789_SoftPWM = 0;

void LCD7789_SetBrightness(uint32_t Brightness) {
	LCD7789_LightSet = Brightness;
	if (!IsLCD7789_SoftPWM)
		__HAL_TIM_SetCompare(LCD_Brightness_timer, LCD_Brightness_channel, Brightness);
}

uint32_t LCD7789_GetBrightness(void) {
	if (IsLCD7789_SoftPWM)
		return LCD7789_LightSet;
	else
		return __HAL_TIM_GetCompare(LCD_Brightness_timer, LCD_Brightness_channel);
}

void LCD7789_SoftPWMEnable(uint8_t enable) {
	IsLCD7789_SoftPWM = enable;
	if (!enable)
		LCD7789_SetBrightness(LCD7789_LightSet);
}

uint8_t LCD7789_SoftPWMIsEnable(void) {
	return IsLCD7789_SoftPWM;
}

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
	if (timecount > 1000)
		timecount = 0;
	else
		timecount += 10;

	if (timecount >= LCD7789_LightSet)
		HAL_GPIO_WritePin(GPIOE, GPIO_PIN_10, GPIO_PIN_SET);
	else
		HAL_GPIO_WritePin(GPIOE, GPIO_PIN_10, GPIO_PIN_RESET);
}

void LCD7789_Light(uint32_t Brightness_Dis, uint32_t time) {
	uint32_t Brightness_Now;
	uint32_t time_now;
	float temp1, temp2;
	float k, set;

	Brightness_Now = LCD7789_GetBrightness();
	time_now = 0;
	if (Brightness_Now == Brightness_Dis)
		return;

	if (time == time_now)
		return;

	temp1 = Brightness_Now;
	temp1 = temp1 - Brightness_Dis;
	temp2 = time_now;
	temp2 = temp2 - time;

	k = temp1 / temp2;

	uint32_t tick = get_tick();
	while (1) {
		delay_ms(1);
		time_now = get_tick() - tick;
		temp2 = time_now - 0;
		set = temp2 * k + Brightness_Now;
		LCD7789_SetBrightness((uint32_t) set);
		if (time_now >= time)
			break;
	}
}

uint16_t LCD7789_POINT_COLOR = 0xFFFF;
uint16_t LCD7789_BACK_COLOR = BLACK;

void LCD7789_ShowChar(uint16_t x, uint16_t y, uint8_t num, uint8_t size,
		uint8_t mode) {
	uint32_t i, j;
	uint16_t b;
	uint16_t color = LCD7789_POINT_COLOR;
	uint16_t bgcolor = LCD7789_BACK_COLOR;

	FontDef *font;
	// size 파라미터에 따라 폰트 구조체를 내부에서 자동 매핑
	if (size == 18)
		font = (FontDef*) &Font_11x18;
	else if (size == 26)
		font = (FontDef*) &Font_16x26;
	else
		font = (FontDef*) &Font_7x10; // 기본 크기(10)

	uint8_t f_width = font->width;
	uint8_t f_height = font->height;

	uint16_t write_buf[f_height * f_width];

	if (num < ' ' || num > '~')
		return;

	if (!mode) {
		for (i = 0; i < f_height; i++) {
			b = font->data[(num - 32) * f_height + i];
			for (j = 0; j < f_width; j++) {
				if ((b << j) & 0x8000) {
					write_buf[i * f_width + j] = (color >> 8) | (color << 8);
				} else {
					write_buf[i * f_width + j] = (bgcolor >> 8)
							| (bgcolor << 8);
				}
			}
		}
		ST7789_LCD_Driver.FillRGBRect(&st7789_pObj, x, y, (uint8_t*) write_buf,
				f_width, f_height);
	} else {
		for (i = 0; i < f_height; i++) {
			b = font->data[(num - 32) * f_height + i];
			for (j = 0; j < f_width; j++) {
				if ((b << j) & 0x8000) {
					ST7789_LCD_Driver.SetPixel(&st7789_pObj, x + j, y + i,
							color);
				}
			}
		}
	}
}

void LCD7789_ShowString(uint16_t x, uint16_t y, uint16_t width, uint16_t height,
		uint8_t size, uint8_t *p) {
	uint16_t x0 = x;
	width += x;
	height += y;

	FontDef *font;
	if (size == 18)
		font = (FontDef*) &Font_11x18;
	else if (size == 26)
		font = (FontDef*) &Font_16x26;
	else
		font = (FontDef*) &Font_7x10;

	while ((*p <= '~') && (*p >= ' ')) {
		if (x + font->width > width) {
			x = x0;
			y += font->height;
		}
		if (y >= height)
			break;

		LCD7789_ShowChar(x, y, *p, size, 0);
		x += font->width;
		p++;
	}
}

void LCD7789_Printf(uint16_t x, uint16_t y, const char *text, ...) {
	char txt[512] = { 0 };
	va_list args;
	va_start(args, text);
	vsprintf(txt, text, args);
	va_end(args);

	// 고정 크기 지정 (ST7735 시절의 고정 렌더링 스타일. 필요시 18이나 26으로 변경 가능)
	uint8_t fixed_size = 18;

	LCD7789_ShowString(x, y, ST7789Ctx.Width - x, ST7789Ctx.Height - y,
			fixed_size, (uint8_t*) txt);
}

static int32_t lcd7789_init(void) {
	HAL_TIMEx_PWMN_Start(LCD_Brightness_timer, LCD_Brightness_channel);
	return ST7789_OK;
}

static int32_t lcd7789_gettick(void) {
	return HAL_GetTick();
}

static int32_t lcd7789_writereg(uint8_t reg, uint8_t *pdata, uint32_t length) {
    int32_t result;
    LCD_CS_RESET;
    LCD_RS_RESET;
    // 레지스터 주소(1바이트)는 폴링으로 전송하는 것이 효율적입니다.
    result = HAL_SPI_Transmit(SPI_Drv, &reg, 1, 100);
    LCD_RS_SET;

    if (length > 0) {
        uint32_t remaining = length;
        uint8_t *ptr = pdata;

        while (remaining > 0) {
            uint16_t chunk = (remaining > 65535) ? 65535 : (uint16_t)remaining;
            HAL_SPI_Transmit_DMA(SPI_Drv, ptr, chunk);

            // DMA 전송 완료 대기
            while (HAL_SPI_GetState(SPI_Drv) != HAL_SPI_STATE_READY) {}

            ptr += chunk;
            remaining -= chunk;
        }
    }
    LCD_CS_SET;
    return result > 0 ? -1 : 0;
}

static int32_t lcd7789_readreg(uint8_t reg, uint8_t *pdata) {
    int32_t result;
    LCD_CS_RESET;
    LCD_RS_RESET;
    result = HAL_SPI_Transmit(SPI_Drv, &reg, 1, 100);
    LCD_RS_SET;

    // 읽기는 보통 1바이트이므로 폴링 유지 또는 DMA 적용 가능
    result += HAL_SPI_Receive_DMA(SPI_Drv, pdata, 1);
    while (HAL_SPI_GetState(SPI_Drv) != HAL_SPI_STATE_READY) {}

    LCD_CS_SET;
    return result > 0 ? -1 : 0;
}

static int32_t lcd7789_senddata(uint8_t *pdata, uint32_t length) {
    LCD_CS_RESET;
    uint32_t remaining = length;
    uint8_t *ptr = pdata;

    while (remaining > 0) {
        uint16_t chunk = (remaining > 65535) ? 65535 : (uint16_t)remaining;
        HAL_SPI_Transmit_DMA(SPI_Drv, ptr, chunk);

        while (HAL_SPI_GetState(SPI_Drv) != HAL_SPI_STATE_READY) {}

        ptr += chunk;
        remaining -= chunk;
    }
    LCD_CS_SET;
    return 0;
}

static int32_t lcd7789_recvdata(uint8_t *pdata, uint32_t length) {
    LCD_CS_RESET;
    uint32_t remaining = length;
    uint8_t *ptr = pdata;

    while (remaining > 0) {
        uint16_t chunk = (remaining > 65535) ? 65535 : (uint16_t)remaining;
        HAL_SPI_Receive_DMA(SPI_Drv, ptr, chunk);

        while (HAL_SPI_GetState(SPI_Drv) != HAL_SPI_STATE_READY) {}

        ptr += chunk;
        remaining -= chunk;
    }
    LCD_CS_SET;
    return 0;
}

void LCD7789_Clear() {
	LCD7789_Light(0, 250);
	ST7789_LCD_Driver.FillRect(&st7789_pObj, 0, 0, ST7789Ctx.Width,
			ST7789Ctx.Height, BLACK);
	LCD7789_Light(900, 250);
}

void LCD7789_Display_Random_BMP_From_SD(const TCHAR *address) {
	FRESULT res;
	DIR dir;
	FILINFO fno;
	int bmp_count = 0;
	uint16_t y_pos = 5;

////	LCD7789_Clear();
////
////	// Printf의 내부 고정 크기(기본 10)를 사용하므로 font 인자가 빠짐
////	LCD7789_Printf(5, y_pos, "[BMP Load Test]");
//	y_pos += 15;

	if (SDCard_Mount() != FR_OK) {
		LCD7789_Printf(5, y_pos, "ERR: SD Mount");
		return;
	}

	res = f_opendir(&dir, address);
	if (res != FR_OK) {
		LCD7789_Printf(5, y_pos, "ERR: Open Dir %d", res);
		SDCard_Unmount();
		return;
	}

	while (1) {
		res = f_readdir(&dir, &fno);
		if (res != FR_OK || fno.fname[0] == 0)
			break;
		if (strstr(fno.fname, ".bmp") || strstr(fno.fname, ".BMP")) {
			bmp_count++;
		}
	}
	f_closedir(&dir);

//	LCD7789_Printf(5, y_pos, "BMP Count: %d", bmp_count);
//	y_pos += 15;

	if (bmp_count == 0) {
		LCD7789_Printf(5, y_pos, "ERR: No BMPs");
		SDCard_Unmount();
		return;
	}

	srand(HAL_GetTick());
	int target_idx = rand() % bmp_count;
	char target_filename[30] = "";

	res = f_opendir(&dir, address);
	if (res == FR_OK) {
		int current_idx = 0;
		while (1) {
			res = f_readdir(&dir, &fno);
			if (res != FR_OK || fno.fname[0] == 0)
				break;

			if (strstr(fno.fname, ".bmp") || strstr(fno.fname, ".BMP")) {
				if (current_idx == target_idx) {
					strcpy(target_filename, fno.fname);
					break;
				}
				current_idx++;
			}
		}
		f_closedir(&dir);
	}

	char full_path[64];
	sprintf(full_path, "%s/%s", address, target_filename);

//	char short_name[15] = { 0 };
//	strncpy(short_name, target_filename, 14);
//	LCD7789_Printf(5, y_pos, "File: %s", short_name);
	y_pos += 15;

	FIL file;
	UINT bytesRead;
	uint8_t header[54];

	res = f_open(&file, full_path, FA_READ);
	if (res != FR_OK) {
		LCD7789_Printf(5, y_pos, "ERR: File Open %d", res);
		SDCard_Unmount();
		return;
	}

	f_read(&file, header, 54, &bytesRead);

	uint32_t dataOffset = header[10] | (header[11] << 8) | (header[12] << 16)
			| (header[13] << 24);
	int32_t width = header[18] | (header[19] << 8) | (header[20] << 16)
			| (header[21] << 24);
	int32_t height = header[22] | (header[23] << 8) | (header[24] << 16)
			| (header[25] << 24);
	uint16_t bitDepth = header[28] | (header[29] << 8);

//	LCD7789_Printf(5, y_pos, "W:%d H:%d B:%d", (int) width, (int) height,
//			(int) bitDepth);
//	y_pos += 15;

	if (bitDepth != 24) {
		LCD7789_Printf(5, y_pos, "ERR: Not 24bit!");
		f_close(&file);
		SDCard_Unmount();
		return;
	}

//	LCD7789_Printf(5, y_pos, "Drawing...");
//	HAL_Delay(1000);
//	LCD7789_Clear();

	f_lseek(&file, dataOffset);
	uint8_t rowBuffer[240 * 3];
	uint16_t lcdBuffer[240];
	int padding = (4 - ((width * 3) % 4)) % 4;

	for (int y = height - 1; y >= 0; y--) {
		f_read(&file, rowBuffer, (width * 3) + padding, &bytesRead);
		for (int x = 0; x < width; x++) {
			uint8_t b = rowBuffer[x * 3];
			uint8_t g = rowBuffer[x * 3 + 1];
			uint8_t r = rowBuffer[x * 3 + 2];

			uint16_t color = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
			lcdBuffer[x] = (color >> 8) | (color << 8);
		}
		if (y < ST7789Ctx.Height && width <= ST7789Ctx.Width) {
			ST7789_LCD_Driver.FillRGBRect(&st7789_pObj, 0, y,
					(uint8_t*) lcdBuffer, width, 1);
		}
	}

	f_close(&file);
	SDCard_Unmount();
}
