/*
 * sensor.c
 *
 *  Created on: 2026. 5. 2.
 *      Author: kth59
 */

#include "main.h"
#include "adc.h"
#include "tim.h"

#include "sensor.h"

#define NUM_SENSORS 16

volatile uint8_t sensor_idx = 0;
volatile uint16_t sensor_raw[NUM_SENSORS];
float line_position = 0.0f;

// [참고] 외부에서 구현할 하드웨어 제어 함수들 (예시)
extern void Set_MUX_Channel(uint8_t channel);
extern void Set_IR_State(uint8_t state);
extern void Motor_PID_Control(float position);
