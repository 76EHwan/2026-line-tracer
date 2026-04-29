/*
 * user_init.h
 *
 *  Created on: 2026. 4. 16.
 *      Author: kth59
 */

#ifndef USER_INIT_H_
#define USER_INIT_H_

#include "main.h"
#define LCD_ST7789

void SDCard_Test(void);
void W25QXX_Test(void);

#ifdef FOC_CONTROL
void Test_DRV8316C_Read_Status(DRV8316C_Handle_t *hdrv);
#endif

#endif /* USER_INIT_H_ */
