/*
 * led.c
 *
 *  Created on: Mar 21, 2025
 *      Author: QH
 */

// Drivers/BSP/Src/led.c
#include "led.h"
#include "main.h"  // 包含CubeMX生成的GPIO定义

/**
  * @brief 初始化LED引脚（低电平点亮）
  */
void LED_Init(void) {
    // 初始状态：所有LED关闭（高电平）
    LED_Off(0);
    LED_Off(1);
    LED_Off(2);
}

/**
  * @brief 点亮指定LED（参数范围：0、1、2）
  * @param led_num: LED编号（对应 CubeMX 的 User Label）
  */
void LED_On(uint8_t led_num) {
    switch (led_num) {
        case 0:  // LED0（PB0）
            HAL_GPIO_WritePin(LED0_GPIO, LED0_PIN, GPIO_PIN_RESET);
            break;
        case 1:  // LED1（PB1）
            HAL_GPIO_WritePin(LED1_GPIO, LED1_PIN, GPIO_PIN_RESET);
            break;
        case 2:  // LED2（PB5）
            HAL_GPIO_WritePin(LED2_GPIO, LED2_PIN, GPIO_PIN_RESET);
            break;
        default:
            break;
    }
}

/**
  * @brief 关闭指定LED（参数范围：0、1、2）
  * @param led_num: LED编号（对应 CubeMX 的 User Label）
  */
void LED_Off(uint8_t led_num) {
    switch (led_num) {
        case 0:
            HAL_GPIO_WritePin(LED0_GPIO, LED0_PIN, GPIO_PIN_SET);
            break;
        case 1:
            HAL_GPIO_WritePin(LED1_GPIO, LED1_PIN, GPIO_PIN_SET);
            break;
        case 2:
            HAL_GPIO_WritePin(LED2_GPIO, LED2_PIN, GPIO_PIN_SET);
            break;
        default:
            break;
    }
}

/**
  * @brief 翻转指定LED状态（参数范围：0、1、2）
  * @param led_num: LED编号（对应 CubeMX 的 User Label）
  */
void LED_Toggle(uint8_t led_num) {
    switch (led_num) {
        case 0:
            HAL_GPIO_TogglePin(LED0_GPIO, LED0_PIN);
            break;
        case 1:
            HAL_GPIO_TogglePin(LED1_GPIO, LED1_PIN);
            break;
        case 2:
            HAL_GPIO_TogglePin(LED2_GPIO, LED2_PIN);
            break;
        default:
            break;
    }
}
