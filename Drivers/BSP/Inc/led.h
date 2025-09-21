/*
 * led.h
 *
 *  Created on: Mar 21, 2025
 *      Author: QH
 */

#ifndef BSP_INC_LED_H_
#define BSP_INC_LED_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f1xx_hal.h"

/* 根据 CubeMX 的 User Label 定义 LED 引脚 */
#define LED0_PIN    GPIO_PIN_0      // PB0 (原 LED1)
#define LED0_GPIO   GPIOA

#define LED1_PIN    GPIO_PIN_1      // PB1 (原 LED2)
#define LED1_GPIO   GPIOA

#define LED2_PIN    GPIO_PIN_2      // PB5 (原 LED3)
#define LED2_GPIO   GPIOA

// #define LED0_Pin GPIO_PIN_0
// #define LED0_GPIO_Port GPIOA
//
// #define LED1_Pin GPIO_PIN_1
// #define LED1_GPIO_Port GPIOA
//
// #define LED2_Pin GPIO_PIN_2
// #define LED2_GPIO_Port GPIOA

/* 函数声明 */
void LED_Init(void);                // 初始化LED硬件
void LED_On(uint8_t led_num);       // 点亮指定LED（参数范围 0~2）
void LED_Off(uint8_t led_num);      // 关闭指定LED
void LED_Toggle(uint8_t led_num);   // 翻转LED状态

#ifdef __cplusplus
}
#endif

#endif /* BSP_INC_LED_H_ */
