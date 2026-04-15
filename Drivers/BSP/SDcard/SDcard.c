/*
 * SDcard.c
 *
 *  Created on: 2026. 4. 15.
 *      Author: kth59
 */

#include "SDcard.h"

#include "fatfs.h" // FATFS 파일 시스템 헤더
#include <string.h>  // memset 등 문자열 함수용

#include "lcd.h"


void SDCard_Test(void) {
    FATFS fs;
    FIL file;
    FRESULT res;
    UINT bytesWritten, bytesRead;

    char writeData[] = "STM32 FATFS Write & Read Test Success!";
    char readBuffer[50];

    ST7735_LCD_Driver.FillRect(&st7735_pObj, 0, 0, ST7735Ctx.Width, ST7735Ctx.Height, BLACK);
    LCD_ShowString(0, 2, ST7735Ctx.Width, 12, 12, (uint8_t*)"[SD Test]");

    // 마운트
    res = f_mount(&fs, "", 1);
    if (res != FR_OK) {
        LCD_ShowString(0, 20, ST7735Ctx.Width, 12, 12, (uint8_t*)"FAIL: Mount");
        return;
    }

    // 쓰기
    res = f_open(&file, "test.txt", FA_CREATE_ALWAYS | FA_WRITE);
    if (res != FR_OK) {
        LCD_ShowString(0, 20, ST7735Ctx.Width, 12, 12, (uint8_t*)"FAIL: Open W");
        f_mount(NULL, "", 0);
        return;
    }
    res = f_write(&file, writeData, strlen(writeData), &bytesWritten);
    f_close(&file);

    if (res != FR_OK || bytesWritten == 0) {
        LCD_ShowString(0, 20, ST7735Ctx.Width, 12, 12, (uint8_t*)"FAIL: Write");
        f_mount(NULL, "", 0);
        return;
    }

    // 읽기
    res = f_open(&file, "test.txt", FA_READ);
    if (res != FR_OK) {
        LCD_ShowString(0, 20, ST7735Ctx.Width, 12, 12, (uint8_t*)"FAIL: Open R");
        f_mount(NULL, "", 0);
        return;
    }
    memset(readBuffer, 0, sizeof(readBuffer));
    res = f_read(&file, readBuffer, sizeof(readBuffer) - 1, &bytesRead);
    f_close(&file);

    if (res != FR_OK || bytesRead == 0) {
        LCD_ShowString(0, 20, ST7735Ctx.Width, 12, 12, (uint8_t*)"FAIL: Read");
        f_mount(NULL, "", 0);
        return;
    }

    // 쓴 내용과 읽은 내용 비교
    if (strcmp(writeData, readBuffer) == 0) {
        LCD_ShowString(0, 20, ST7735Ctx.Width, 12, 12, (uint8_t*)"SUCCESS!");
    } else {
        LCD_ShowString(0, 20, ST7735Ctx.Width, 12, 12, (uint8_t*)"FAIL: Mismatch");
    }

    // 읽어온 내용 LCD에 출력 (자동 줄바꿈)
    LCD_ShowString(0, 36, ST7735Ctx.Width, ST7735Ctx.Height - 36, 12, (uint8_t*)readBuffer);

    f_mount(NULL, "", 0);
}

