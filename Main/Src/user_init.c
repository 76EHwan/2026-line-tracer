/*
 * user_init.c
 *
 *  Created on: 2026. 4. 16.
 *      Author: kth59
 */


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

void W25QXX_Test(void){
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


