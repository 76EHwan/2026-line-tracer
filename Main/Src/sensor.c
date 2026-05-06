/*
 * sensor.c
 *
 *  Created on: 2026. 5. 2.
 *      Author: kth59
 */
#include "sensor.h"

#define SENSOR_TRIG_TIM &htim

#define SENSOR_IR_EN	HAL_GPIO_WritePin(SENSOR_IR_EN_GPIO_Port, SENSOR_IR_EN_Pin, GPIO_PIN_SET);
#define SENSOR_IR_DIS	HAL_GPIO_WritePin(SENSOR_IR_EN_GPIO_Port, SENSOR_IR_EN_Pin, GPIO_PIN_RESET);

typedef struct {
	GPIO_TypeDef *GPIO_Port;
	uint32_t GPIO_Pin;
} Sensor_Mux_Pin_t;

typedef enum {
	MUX0 = 0, MUX1 = 1, MUX2 = 2, MUX3 = 3
} MUX_CHANNEL;

Sensor_Mux_Pin_t Sensor_Mux_Pin[] = { { .GPIO_Port = SENSOR_MUX0_GPIO_Port,
		.GPIO_Pin = SENSOR_MUX0_Pin }, { .GPIO_Port = SENSOR_MUX1_GPIO_Port,
		.GPIO_Pin = SENSOR_MUX1_Pin }, { .GPIO_Port = SENSOR_MUX2_GPIO_Port,
		.GPIO_Pin = SENSOR_MUX2_Pin }, { .GPIO_Port = SENSOR_MUX3_GPIO_Port,
		.GPIO_Pin = SENSOR_MUX3_Pin }, };

SensorDataTypeDef ir_sensor = { .idx = 0, .raw = { 0 }, .blackmax = { 0 },
		.whitemax = { 0 }, .normalized = { 0 }, .state = 0, .threshold = 100 };

uint16_t adc3_buf[3] = { 0 };

void Sensor_Start() {
	ir_sensor.idx = 0;
	HAL_ADC_Start_DMA(&hadc3, (uint32_t*) adc3_buf, 3);
}

void Sensor_Stop() {
	HAL_ADC_Stop_DMA(&hadc3);

}

__STATIC_INLINE void Set_Mux_Channel(SensorDataTypeDef *sensor) {
	uint8_t index = sensor->idx;
	HAL_GPIO_WritePin(Sensor_Mux_Pin[MUX0].GPIO_Port,
			Sensor_Mux_Pin[MUX0].GPIO_Pin, index & (0x01 << MUX0));
	HAL_GPIO_WritePin(Sensor_Mux_Pin[MUX1].GPIO_Port,
			Sensor_Mux_Pin[MUX1].GPIO_Pin, index & (0x01 << MUX0));
	HAL_GPIO_WritePin(Sensor_Mux_Pin[MUX2].GPIO_Port,
			Sensor_Mux_Pin[MUX2].GPIO_Pin, index & (0x01 << MUX0));
	HAL_GPIO_WritePin(Sensor_Mux_Pin[MUX3].GPIO_Port,
			Sensor_Mux_Pin[MUX3].GPIO_Pin, index & (0x01 << MUX0));
	SENSOR_IR_EN
	;
}

__STATIC_INLINE void Get_Data_from_DMA(SensorDataTypeDef *sensor,
		uint16_t *data) {
	uint8_t index = sensor->idx;
	sensor->raw[LEFT_MARK_SENSOR_INDEX] = *(data + 0);
	sensor->raw[RIGHT_MARK_SENSOR_INDEX] = *(data + 1);
	sensor->raw[index] = *(data + 2);
}

void TIM7_IRQ_Handler() {
	Set_Mux_Channel(&ir_sensor);
	__HAL_TIM_SET_COUNTER(&htim2, 0);
	HAL_TIM_Base_Start(&htim2);
}

void ADC3_IRQ_Handler() {
	SENSOR_IR_DIS
	;
	Get_Data_from_DMA(&ir_sensor, adc3_buf);

	ir_sensor.idx = (ir_sensor.idx + 1) & 0x0F;
}

void Sensor_Calibration() {
	uint8_t i = 0;
	while (1) {
		if (ir_sensor.whitemax[i] < ir_sensor.raw[i]) {
			ir_sensor.whitemax[i] = ir_sensor.raw[i];
		}
		i = (i + 1) % 18;
	}
	while (1) {
		if (ir_sensor.blackmax[i] < ir_sensor.raw[i]) {
			ir_sensor.blackmax[i] = ir_sensor.raw[i];
		}
		i = (i + 1) % 18;
	}
}
