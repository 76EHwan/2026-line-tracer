/*
 * SDcard.c
 *
 *  Created on: 2026. 4. 15.
 *      Author: kth59
 */

#include "SDcard.h"
#include "fatfs.h"
#include <string.h>
#include "lcd.h"

static FATFS fs;

// =====================
// SD카드 마운트
// =====================
static FRESULT SDCard_Mount(void) {
    return f_mount(&fs, "", 1);
}

static void SDCard_Unmount(void) {
    f_mount(NULL, "", 0);
}

// =====================
// 파일 쓰기
// =====================
FRESULT SDCard_Write(const char* filename, const char* data) {
    FIL file;
    FRESULT res;
    UINT bytesWritten;

    res = f_open(&file, filename, FA_CREATE_ALWAYS | FA_WRITE);
    if (res != FR_OK) return res;

    res = f_write(&file, data, strlen(data), &bytesWritten);
    f_close(&file);

    if (res == FR_OK && bytesWritten == 0) return FR_DENIED;
    return res;
}

// =====================
// 파일 읽기
// =====================
FRESULT SDCard_Read(const char* filename, char* buffer, UINT bufSize) {
    FIL file;
    FRESULT res;
    UINT bytesRead;

    res = f_open(&file, filename, FA_READ);
    if (res != FR_OK) return res;

    memset(buffer, 0, bufSize);
    res = f_read(&file, buffer, bufSize - 1, &bytesRead);
    f_close(&file);

    if (res == FR_OK && bytesRead == 0) return FR_DENIED;
    return res;
}

// =====================
// 테스트
// =====================
void SDCard_Test(void) {
    FRESULT res;
    char writeData[] = "STM32 FATFS Write & Read Test Success!";
    char readBuffer[50];

    ST7735_LCD_Driver.FillRect(&st7735_pObj, 0, 0, ST7735Ctx.Width, ST7735Ctx.Height, BLACK);
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
    uint16_t y = 1 + 3 * 13;
    LCD_ShowString(1, y, ST7735Ctx.Width - 2, ST7735Ctx.Height - y - 1, 12, (uint8_t*)readBuffer);

    SDCard_Unmount();
}
