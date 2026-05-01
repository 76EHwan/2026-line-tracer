/*
 * user_init.c
 *
 *  Created on: 2026. 4. 16.
 *      Author: kth59
 */
#include "main.h"
#include "stm32h7xx_hal.h"
#include "usb_device.h"
#include "usbd_core.h"

#include "user_init.h"
#include "st7789_lcd.h"
#include "w25qxx.h"
#include "SDcard.h"
#include "button.h"

extern USBD_HandleTypeDef hUsbDeviceFS;

#define SYSTEM_MEMORY_BASE 0x1FF09800

void Button_init() {
	Button_Init_Internal(&btn_k, KEY_GPIO_Port, KEY_Pin, GPIO_PIN_SET);
	Button_Init_Internal(&btn_l, SWITCH_LEFT_GPIO_Port, SWITCH_LEFT_Pin,
			GPIO_PIN_SET);
	Button_Init_Internal(&btn_r, SWITCH_RIGHT_GPIO_Port, SWITCH_RIGHT_Pin,
			GPIO_PIN_SET);
	Button_Init_Internal(&btn_u, SWITCH_UP_GPIO_Port, SWITCH_UP_Pin,
			GPIO_PIN_SET);
	Button_Init_Internal(&btn_d, SWITCH_DOWN_GPIO_Port, SWITCH_DOWN_Pin,
			GPIO_PIN_SET);
}

void SDCard_Test(void) {
	FRESULT res;
	char writeData[] = "STM32 FATFS Write & Read Test Success!";
	char readBuffer[50];

	LCD7789_Printf(0, 0, "[SD Test]");

	// 마운트
	res = SDCard_Mount();
	if (res != FR_OK) {
		LCD7789_Printf(0, 1, "FAIL: Mount %d", res);
		return;
	}

	// 쓰기
	res = SDCard_Write("test.txt", writeData);
	if (res != FR_OK) {
		LCD7789_Printf(0, 1, "FAIL: Write %d", res);
		SDCard_Unmount();
		return;
	}

	// 읽기
	res = SDCard_Read("test.txt", readBuffer, sizeof(readBuffer));
	if (res != FR_OK) {
		LCD7789_Printf(0, 1, "FAIL: Read %d", res);
		SDCard_Unmount();
		return;
	}

	// 결과 비교
	if (strcmp(writeData, readBuffer) == 0) {
		LCD7789_Printf(0, 1, "SUCCESS!");
	} else {
		LCD7789_Printf(0, 1, "FAIL: Mismatch");
	}

	// 읽어온 내용 출력 (row=3부터 자동 줄바꿈)
	LCD7789_Printf(0, 2, readBuffer);

	SDCard_Unmount();
}

void W25QXX_Test(void) {
	W25Qx_Init();
	uint16_t id;
	W25Qx_Read_ID(&id);
	LCD7789_Printf(0, 0, "%04X", id);
}

#ifdef FOC_CONTROL
void Test_DRV8316C_Read_Status(DRV8316C_Handle_t *hdrv) {
	uint8_t status;
	// 디버깅용: 상태 레지스터 읽기
	DRV8316C_ReadRegister(hdrv, DRV_REG_IC_STATUS, &status);
	LCD7789_Printf(0, 4, "IC STATUS: %02X", status);

	DRV8316C_ReadRegister(hdrv, DRV_REG_STATUS_1, &status);
	LCD7789_Printf(0, 5, "STATUS 1: %02X", status);

	DRV8316C_ReadRegister(hdrv, DRV_REG_STATUS_2, &status);
	LCD7789_Printf(0, 6, "STATUS 2: %02X", status);

	DRV8316C_ReadRegister(hdrv, DRV_REG_CTRL_2, &status);
	LCD7789_Printf(0, 7, "CTRL 2: %02X", status);

}
#endif

void JumpToBootloader(void)
{
    void (*SysMemBootJump)(void);

    // 1. 작동 중인 USB 라이브러리 중지
    USBD_Stop(&hUsbDeviceFS);
    USBD_DeInit(&hUsbDeviceFS);

    // 2. PA12 핀(USB D+)을 Low로 강제하여 PC에 Disconnect 신호 보내기
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    __HAL_RCC_GPIOA_CLK_ENABLE();
    GPIO_InitStruct.Pin = GPIO_PIN_12;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_12, GPIO_PIN_RESET);

    // PC가 완전히 연결 해제를 인식할 수 있도록 200ms 대기 (여유 있게)
    HAL_Delay(200);

    // ★ [핵심 추가 1] PA12 핀을 다시 초기 상태로 되돌림
    // 부트로더가 이 핀을 다시 USB Alternate Function으로 설정할 수 있도록 방해하지 않음
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_12);

    // 3. 시스템 초기화 및 캐시 끄기
    __disable_irq(); // 잠시 글로벌 인터럽트를 끄고 초기화 진행

    SysTick->CTRL = 0;
    SysTick->LOAD = 0;
    SysTick->VAL  = 0;

    HAL_MPU_Disable();
    SCB_DisableICache();
    SCB_DisableDCache();

    HAL_DeInit();

    HAL_RCC_DeInit();
    RCC->CR |= RCC_CR_HSION;
    while((RCC->CR & RCC_CR_HSIRDY) == 0);
    RCC->CFGR = 0x00000000;

    // 개별 인터럽트 펜딩 레지스터 초기화
    for (int i = 0; i < 8; i++) {
        NVIC->ICER[i] = 0xFFFFFFFF;
        NVIC->ICPR[i] = 0xFFFFFFFF;
    }

    // 4. 점프 준비
    SCB->VTOR = 0;
    __DSB();
    __ISB();

    SysMemBootJump = (void (*)(void)) (*((uint32_t *)(SYSTEM_MEMORY_BASE + 4)));
    __set_MSP(*((uint32_t *)SYSTEM_MEMORY_BASE));

    // ★ [핵심 추가 2] 점프 직전 글로벌 인터럽트 마스크 해제
    // 이 코드가 있어야 부트로더가 USB 인터럽트를 정상적으로 처리합니다.
    __enable_irq();

    // 5. 부트로더로 점프
    SysMemBootJump();

    while(1);
}
