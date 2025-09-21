/*
 * sensor.c
 *
 *  Created on: Mar 21, 2025
 *      Author: QH
 */

// Drivers/BSP/Src/sensor.c
#include "sensor.h"
#include "main.h"  // 包含CubeMX生成的ADC和GPIO配置

/* 私有变量：滑动平均滤波 */
#define FILTER_SIZE  5
static uint16_t s_adc_buffer[FILTER_SIZE] = {0};
static uint8_t s_buffer_index = 0;

/**
  * @brief 初始化光敏传感器硬件（ADC）
  */
void BSP_LightSensor_Init(void) {
    /* 校准ADC（CubeMX可能已自动生成校准代码，若未生成需手动调用） */
    if (HAL_ADCEx_Calibration_Start(&LIGHT_SENSOR_ADC_HANDLE) != HAL_OK) {
        Error_Handler();  // 初始化失败处理（需在main.c中定义）
    }
}

/**
  * @brief 读取光敏传感器的原始ADC值
  * @retval ADC原始值（0~4095）
  */
uint16_t BSP_LightSensor_ReadRaw(void) {
    HAL_ADC_Start(&LIGHT_SENSOR_ADC_HANDLE);              // 启动ADC转换
    if (HAL_ADC_PollForConversion(&LIGHT_SENSOR_ADC_HANDLE, 10) == HAL_OK) {
        return HAL_ADC_GetValue(&LIGHT_SENSOR_ADC_HANDLE); // 读取转换结果
    }
    return 0;  // 转换超时或失败
}

/**
  * @brief 读取滤波后的光敏传感器值（滑动平均滤波）
  * @retval 滤波后的ADC值（0~4095）
  */
uint16_t BSP_LightSensor_ReadFiltered(void) {
    /* 更新缓冲区 */
    s_adc_buffer[s_buffer_index] = BSP_LightSensor_ReadRaw();
    s_buffer_index = (s_buffer_index + 1) % FILTER_SIZE;

    /* 计算平均值 */
    uint32_t sum = 0;
    for (uint8_t i = 0; i < FILTER_SIZE; i++) {
        sum += s_adc_buffer[i];
    }
    return (uint16_t)(sum / FILTER_SIZE);
}
