/*
 * sensor.c
 *
 *  Created on: 2026. 5. 2.
 *      Author: kth59
 */
#include "sensor.h"
#include "main.h"
#include "button.h"
#include "st7789_lcd.h"

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

volatile SensorDataTypeDef ir_sensor = { .idx = 0, .raw = { 0 }, .blackmax = { 0 },
		.whitemax = { 0 }, .normalized = { 0 }, .state = 0, .threshold = 100 };

uint16_t adc3_buf[3] = { 0 };

void Sensor_Printf(uint8_t idx, volatile uint16_t *sensor_data){
	LCD_Printf(8 * (idx & 0x1), idx / 2 + 1, "0x%03X", *(sensor_data + idx));
}

void Sensor_Start() {
	ir_sensor.idx = 0;
    HAL_StatusTypeDef ret = HAL_ADC_Start_DMA(&hadc3, (uint32_t*)adc3_buf, 3);
    if (ret != HAL_OK) {
        while(1) LED_TOGGLE;
    }
    HAL_TIM_Base_Start_IT(&htim7);
}

void Sensor_Stop() {
	HAL_TIM_Base_Stop(&htim2);
	HAL_TIM_Base_Stop_IT(&htim7);
	HAL_ADC_Stop_DMA(&hadc3);
}

__STATIC_INLINE void Set_Mux_Channel(volatile SensorDataTypeDef *sensor) {
	uint8_t index = sensor->idx;
	HAL_GPIO_WritePin((Sensor_Mux_Pin + MUX0)->GPIO_Port,
			(Sensor_Mux_Pin + MUX0)->GPIO_Pin, index & (0x01 << MUX0));
	HAL_GPIO_WritePin((Sensor_Mux_Pin + MUX1)->GPIO_Port,
			(Sensor_Mux_Pin + MUX1)->GPIO_Pin, index & (0x01 << MUX1));
	HAL_GPIO_WritePin((Sensor_Mux_Pin + MUX2)->GPIO_Port,
			(Sensor_Mux_Pin + MUX2)->GPIO_Pin, index & (0x01 << MUX2));
	HAL_GPIO_WritePin((Sensor_Mux_Pin + MUX3)->GPIO_Port,
			(Sensor_Mux_Pin + MUX3)->GPIO_Pin, index & (0x01 << MUX3));
	SENSOR_IR_EN
	;
}

__STATIC_INLINE void Get_Data_from_DMA(volatile SensorDataTypeDef *sensor,
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
	SENSOR_IR_DIS;
	Get_Data_from_DMA(&ir_sensor, adc3_buf);

	ir_sensor.idx = (ir_sensor.idx + 1) & 0x0F;
}

void Sensor_Calibration() {
	Sensor_Start();
	uint8_t i = 0;
	LCD_Printf(0, 0, "White Max");
	while (Button_Get_Input() != INPUT_CMD_K_HOLD) {
		LCD_Printf(0, 12, "%d", ir_sensor.idx);
		if (ir_sensor.whitemax[i] < ir_sensor.raw[i]) {
			ir_sensor.whitemax[i] = ir_sensor.raw[i];
		}
		Sensor_Printf(i, ir_sensor.whitemax);
		i = (i + 1) % 18;
	}
	LCD_Clear();
	Sensor_Stop();
	Button_Wait_Release(&btn_k);
	Sensor_Start();
	LCD_Printf(0, 0, "Black Max");

	while (Button_Get_Input() != INPUT_CMD_K_HOLD) {
		LCD_Printf(0, 12, "%d", ir_sensor.idx);

		if (ir_sensor.blackmax[i] < ir_sensor.raw[i]) {
			ir_sensor.blackmax[i] = ir_sensor.raw[i];
		}
		Sensor_Printf(i, ir_sensor.blackmax);
		i = (i + 1) % 18;
	}
	LCD_Clear();
	Button_Wait_Release(&btn_k);
	Sensor_Stop();
}

void Sensor_Raw_Printf() {
	Sensor_Start();

	uint8_t i = 0;
	LCD_Printf(0, 0, "Sensor Raw");

	while(Button_Get_Input() != INPUT_CMD_K_HOLD) {
		LCD_Printf(0, 12, "%d", ir_sensor.idx);
		Sensor_Printf(i, ir_sensor.raw);
		i = (i + 1) % 18;
	}
	LCD_Clear();
	Sensor_Stop();
	Button_Wait_Release(&btn_k);
}
