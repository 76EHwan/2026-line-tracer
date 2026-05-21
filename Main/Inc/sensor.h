/*
 * sensor.h
 *
 *  Created on: 2026. 5. 2.
 *      Author: kth59
 */

#ifndef INC_SENSOR_H_
#define INC_SENSOR_H_

#include "main.h"
#include "adc.h"
#include "tim.h"

#define NUM_SENSORS 18
#define LEFT_MARK_SENSOR_INDEX 16
#define RIGHT_MARK_SENSOR_INDEX 17


typedef struct {
	volatile uint8_t idx;
	uint16_t raw[NUM_SENSORS];
	uint16_t whitemax[NUM_SENSORS];
	uint16_t blackmax[NUM_SENSORS];
	uint16_t normalized[NUM_SENSORS];
	uint16_t state;
	uint16_t threshold;
} SensorDataTypeDef;

extern volatile SensorDataTypeDef ir_sensor;

void Sensor_Calibration();
void Sensor_Raw_Printf();

void TIM7_IRQ_Handler(void);
void ADC3_IRQ_Handler(void);

#endif /* INC_SENSOR_H_ */
