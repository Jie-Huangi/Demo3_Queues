/*
 * sensor.h
 *
 *  Created on: Mar 21, 2025
 *      Author: QH
 */

#ifndef BSP_INC_SENSOR_H_
#define BSP_INC_SENSOR_H_

#ifdef __cplusplus  // 兼容 C++ 编译器的条件编译
extern "C" {        // 禁用 C++ 名称修饰
#endif

/* 依赖的硬件库 */
#include "stm32f1xx_hal.h"
#include "adc.h"

/* 光敏传感器硬件定义 */
#define LIGHT_SENSOR_ADC_HANDLE    hadc1   // 使用的ADC句柄（需与CubeMX配置一致）
#define LIGHT_SENSOR_ADC_CHANNEL    ADC_CHANNEL_3  // PC2对应的ADC通道

/* 函数声明 */
void BSP_LightSensor_Init(void);            // 传感器初始化
uint16_t BSP_LightSensor_ReadRaw(void);     // 读取ADC原始值
uint16_t BSP_LightSensor_ReadFiltered(void); // 读取滤波后的ADC值


#ifdef __cplusplus  // 闭合 extern "C" 作用域
}
#endif

#endif /* BSP_INC_SENSOR_H_ */
