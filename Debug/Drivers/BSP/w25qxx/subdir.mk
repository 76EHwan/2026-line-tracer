################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Drivers/BSP/w25qxx/w25qxx.c 

OBJS += \
./Drivers/BSP/w25qxx/w25qxx.o 

C_DEPS += \
./Drivers/BSP/w25qxx/w25qxx.d 


# Each subdirectory must supply rules for building sources it contributes
Drivers/BSP/w25qxx/%.o Drivers/BSP/w25qxx/%.su Drivers/BSP/w25qxx/%.cyclo: ../Drivers/BSP/w25qxx/%.c Drivers/BSP/w25qxx/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -g3 -DDEBUG -DUSE_PWR_LDO_SUPPLY -DUSE_HAL_DRIVER -DSTM32H743xx -c -I../Core/Inc -I../FATFS/Target -I../FATFS/App -I../USB_DEVICE/App -I../USB_DEVICE/Target -I../Drivers/STM32H7xx_HAL_Driver/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../Middlewares/Third_Party/FatFs/src -I../Middlewares/ST/STM32_USB_Device_Library/Core/Inc -I../Middlewares/ST/STM32_USB_Device_Library/Class/CustomHID/Inc -I../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../Drivers/CMSIS/Include -I../Drivers/BSP/ST7735 -I../Drivers/BSP/SDcard -I../Drivers/BSP/w25qxx -I../Main/Inc -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Drivers-2f-BSP-2f-w25qxx

clean-Drivers-2f-BSP-2f-w25qxx:
	-$(RM) ./Drivers/BSP/w25qxx/w25qxx.cyclo ./Drivers/BSP/w25qxx/w25qxx.d ./Drivers/BSP/w25qxx/w25qxx.o ./Drivers/BSP/w25qxx/w25qxx.su

.PHONY: clean-Drivers-2f-BSP-2f-w25qxx

