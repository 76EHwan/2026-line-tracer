/*
 * user_init.c
 *
 *  Created on: 2026. 4. 16.
 *      Author: kth59
 */
#define TARGET_FOLDER "/display"

#include "user_init.h"
#include "main.h"
#include "lcd.h"
#include "w25qxx.h"
#include "SDcard.h"

void SDCard_Test(void) {
	FRESULT res;
	char writeData[] = "STM32 FATFS Write & Read Test Success!";
	char readBuffer[50];

	LCD_Printf(0, 0, "[SD Test]");

	// 마운트
	res = SDCard_Mount();
	if (res != FR_OK) {
		LCD_Printf(0, 1, "FAIL: Mount %d", res);
		return;
	}

	// 쓰기
	res = SDCard_Write("test.txt", writeData);
	if (res != FR_OK) {
		LCD_Printf(0, 1, "FAIL: Write %d", res);
		SDCard_Unmount();
		return;
	}

	// 읽기
	res = SDCard_Read("test.txt", readBuffer, sizeof(readBuffer));
	if (res != FR_OK) {
		LCD_Printf(0, 1, "FAIL: Read %d", res);
		SDCard_Unmount();
		return;
	}

	// 결과 비교
	if (strcmp(writeData, readBuffer) == 0) {
		LCD_Printf(0, 1, "SUCCESS!");
	} else {
		LCD_Printf(0, 1, "FAIL: Mismatch");
	}

	// 읽어온 내용 출력 (row=3부터 자동 줄바꿈)
	LCD_Printf(0, 2, readBuffer);

	SDCard_Unmount();
}

void W25QXX_Test(void) {
	W25Qx_Init();
	uint16_t id;
	W25Qx_Read_ID(&id);
	LCD_Printf(0, 0, "%04X", id);
}

#ifdef FOC_CONTROL
void Test_DRV8316C_Read_Status(DRV8316C_Handle_t *hdrv) {
	uint8_t status;
	// 디버깅용: 상태 레지스터 읽기
	DRV8316C_ReadRegister(hdrv, DRV_REG_IC_STATUS, &status);
	LCD_Printf(0, 4, "IC STATUS: %02X", status);

	DRV8316C_ReadRegister(hdrv, DRV_REG_STATUS_1, &status);
	LCD_Printf(0, 5, "STATUS 1: %02X", status);

	DRV8316C_ReadRegister(hdrv, DRV_REG_STATUS_2, &status);
	LCD_Printf(0, 6, "STATUS 2: %02X", status);

	DRV8316C_ReadRegister(hdrv, DRV_REG_CTRL_2, &status);
	LCD_Printf(0, 7, "CTRL 2: %02X", status);

}
#endif

void Display_Random_BMP_From_SD(void) {
	FRESULT res;
	DIR dir;
	FILINFO fno;
	int bmp_count = 0;

	LCD_Clear();
	LCD_Printf(0, 0, "[BMP Load Test]");

	// 1. SD 마운트 확인
	if (SDCard_Mount() != FR_OK) {
		LCD_Printf(0, 1, "ERR: SD Mount");
		return;
	}

	// 2. 디렉토리 열기 확인
	res = f_opendir(&dir, TARGET_FOLDER);
	if (res != FR_OK) {
		LCD_Printf(0, 1, "ERR: Open Dir %d", res);
		SDCard_Unmount();
		return;
	}

	// 3. 파일 개수 세기
	while (1) {
		res = f_readdir(&dir, &fno);
		if (res != FR_OK || fno.fname[0] == 0)
			break;
		if (strstr(fno.fname, ".bmp") || strstr(fno.fname, ".BMP")) {
			bmp_count++;
		}
	}
	f_closedir(&dir);

	LCD_Printf(0, 1, "BMP Count: %d", bmp_count);
	if (bmp_count == 0) {
		LCD_Printf(0, 2, "ERR: No BMPs");
		SDCard_Unmount();
		return;
	}

	// 4. 랜덤 파일 선택
	srand(HAL_GetTick());
	int target_idx = rand() % bmp_count;
	char target_filename[30] = "";

	res = f_opendir(&dir, TARGET_FOLDER);
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

	// 5. 파일 열기
	char full_path[64];
	sprintf(full_path, "%s/%s", TARGET_FOLDER, target_filename);

	// 파일명이 길면 짤릴 수 있으니 앞 10글자만 출력
	char short_name[15] = { 0 };
	strncpy(short_name, target_filename, 10);
	LCD_Printf(0, 2, "File: %s", short_name);

	FIL file;
	UINT bytesRead;
	uint8_t header[54];

	res = f_open(&file, full_path, FA_READ);
	if (res != FR_OK) {
		LCD_Printf(0, 3, "ERR: File Open %d", res);
		SDCard_Unmount();
		return;
	}

	f_read(&file, header, 54, &bytesRead);

	// 6. 헤더 정보 파싱 (안전한 비트 시프트 방식 적용)
	uint32_t dataOffset = header[10] | (header[11] << 8) | (header[12] << 16)
			| (header[13] << 24);
	int32_t width = header[18] | (header[19] << 8) | (header[20] << 16)
			| (header[21] << 24);
	int32_t height = header[22] | (header[23] << 8) | (header[24] << 16)
			| (header[25] << 24);
	uint16_t bitDepth = header[28] | (header[29] << 8);

	LCD_Printf(0, 3, "W:%d H:%d B:%d", (int) width, (int) height,
			(int) bitDepth);

	// 7. 24비트 포맷 검사
	if (bitDepth != 24) {
		LCD_Printf(0, 4, "ERR: Not 24bit!");
		f_close(&file);
		SDCard_Unmount();
		return;
	}

	LCD_Printf(0, 4, "Drawing...");

	// 8. 데이터 읽고 LCD에 그리기
	f_lseek(&file, dataOffset);
	uint8_t rowBuffer[160 * 3];
	uint16_t lcdBuffer[160];
	int padding = (4 - ((width * 3) % 4)) % 4;

	for (int y = height - 1; y >= 0; y--) {
		f_read(&file, rowBuffer, (width * 3) + padding, &bytesRead);
		for (int x = 0; x < width; x++) {
			uint8_t b = rowBuffer[x * 3];
			uint8_t g = rowBuffer[x * 3 + 1];
			uint8_t r = rowBuffer[x * 3 + 2];

			// r과 b의 위치를 반대로 적용
			uint16_t color = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
			// 바이트 스왑 적용
			lcdBuffer[x] = (color >> 8) | (color << 8);
		}
		// y좌표가 LCD 해상도(80 또는 160)를 넘어가면 무시
		if (y < 160 && width <= 160) {
			ST7735_FillRGBRect(&st7735_pObj, 0, y, (uint8_t*) lcdBuffer, width,
					1);
		}
	}

	f_close(&file);
	SDCard_Unmount();
}
