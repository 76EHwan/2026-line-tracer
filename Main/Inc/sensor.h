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

#include "sensor.h"

#define NUM_SENSORS 18
#define LEFT_MARK_SENSOR_INDEX 16
#define RIGHT_MARK_SENSOR_INDEX 17


typedef struct {
	uint8_t idx;
	uint8_t raw[NUM_SENSORS];
	uint8_t whitemax[NUM_SENSORS];
	uint8_t blackmax[NUM_SENSORS];
	uint8_t normalized[NUM_SENSORS];
	uint16_t state;
	uint8_t threshold;
} SensorDataTypeDef;

extern SensorDataTypeDef ir_sensor;

#endif /* INC_SENSOR_H_ */
