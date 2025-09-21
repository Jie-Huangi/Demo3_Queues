/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>

#include "oled.h"
#include "led.h"
#include "sensor.h"
#include "queue.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
typedef StaticTask_t osStaticThreadDef_t;
typedef StaticQueue_t osStaticMessageQDef_t;
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */
/* 显示数据结构体 */
typedef struct {
  uint16_t light_value;
  uint8_t led_status;
} DisplayData_t;

// 在 Process_Task 中定义校准值
const uint16_t ADC_MAX = 3800;  // 全暗时的ADC值
const uint16_t ADC_MIN = 200;   // 全亮时的ADC值

/* USER CODE END Variables */
/* Definitions for Sensor_Task */
osThreadId_t Sensor_TaskHandle;
uint32_t Sensor_TaskBuffer[ 128 ];
osStaticThreadDef_t Sensor_TaskControlBlock;
const osThreadAttr_t Sensor_Task_attributes = {
  .name = "Sensor_Task",
  .cb_mem = &Sensor_TaskControlBlock,
  .cb_size = sizeof(Sensor_TaskControlBlock),
  .stack_mem = &Sensor_TaskBuffer[0],
  .stack_size = sizeof(Sensor_TaskBuffer),
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for Process_Task */
osThreadId_t Process_TaskHandle;
uint32_t Process_TaskBuffer[ 128 ];
osStaticThreadDef_t Process_TaskControlBlock;
const osThreadAttr_t Process_Task_attributes = {
  .name = "Process_Task",
  .cb_mem = &Process_TaskControlBlock,
  .cb_size = sizeof(Process_TaskControlBlock),
  .stack_mem = &Process_TaskBuffer[0],
  .stack_size = sizeof(Process_TaskBuffer),
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for Display_Task */
osThreadId_t Display_TaskHandle;
uint32_t Display_TaskBuffer[ 128 ];
osStaticThreadDef_t Display_TaskControlBlock;
const osThreadAttr_t Display_Task_attributes = {
  .name = "Display_Task",
  .cb_mem = &Display_TaskControlBlock,
  .cb_size = sizeof(Display_TaskControlBlock),
  .stack_mem = &Display_TaskBuffer[0],
  .stack_size = sizeof(Display_TaskBuffer),
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for xLightQueue */
osMessageQueueId_t xLightQueueHandle;
uint8_t xLightQueueBuffer[ 10 * sizeof( uint16_t ) ];
osStaticMessageQDef_t xLightQueueControlBlock;
const osMessageQueueAttr_t xLightQueue_attributes = {
  .name = "xLightQueue",
  .cb_mem = &xLightQueueControlBlock,
  .cb_size = sizeof(xLightQueueControlBlock),
  .mq_mem = &xLightQueueBuffer,
  .mq_size = sizeof(xLightQueueBuffer)
};
/* Definitions for xDisplayQueue */
osMessageQueueId_t xDisplayQueueHandle;
uint8_t xDisplayQueueBuffer[ 5 * sizeof( DisplayData_t ) ];
osStaticMessageQDef_t xDisplayQueueControlBlock;
const osMessageQueueAttr_t xDisplayQueue_attributes = {
  .name = "xDisplayQueue",
  .cb_mem = &xDisplayQueueControlBlock,
  .cb_size = sizeof(xDisplayQueueControlBlock),
  .mq_mem = &xDisplayQueueBuffer,
  .mq_size = sizeof(xDisplayQueueBuffer)
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
static uint8_t CalculateDarknessPercent(uint16_t adc_value);

/* USER CODE END FunctionPrototypes */

void StarSensorTask(void *argument);
void StartProcessTask(void *argument);
void StartDisplayTask(void *argument);
static void DrawCoordinateGrid(uint8_t start_y, uint8_t height);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* Create the queue(s) */
  /* creation of xLightQueue */
  xLightQueueHandle = osMessageQueueNew (10, sizeof(uint16_t), &xLightQueue_attributes);

  /* creation of xDisplayQueue */
  xDisplayQueueHandle = osMessageQueueNew (5, sizeof(DisplayData_t), &xDisplayQueue_attributes);

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of Sensor_Task */
  Sensor_TaskHandle = osThreadNew(StarSensorTask, NULL, &Sensor_Task_attributes);

  /* creation of Process_Task */
  Process_TaskHandle = osThreadNew(StartProcessTask, NULL, &Process_Task_attributes);

  /* creation of Display_Task */
  Display_TaskHandle = osThreadNew(StartDisplayTask, NULL, &Display_Task_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_StarSensorTask */
/**
  * @brief  Function implementing the Sensor_Task thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StarSensorTask */
void StarSensorTask(void *argument)
{
  /* USER CODE BEGIN StarSensorTask */
  uint16_t adc_value;
  /* Infinite loop */
  for (;;) {
    // 读取滤波后的光强数据
    adc_value = BSP_LightSensor_ReadFiltered();
    // 发送到队列（非阻塞，超时100ms）
    if (xQueueSend(xLightQueueHandle, &adc_value, pdMS_TO_TICKS(100)) != pdPASS) {
      // 处理队列满错误（如LED闪烁报警）
    }
    osDelay(pdMS_TO_TICKS(100)); //100ms采样周期
  }
  /* USER CODE END StarSensorTask */
}

/* USER CODE BEGIN Header_StartProcessTask */
/**
* @brief Function implementing the Process_Task thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartProcessTask */
void StartProcessTask(void *argument)
{
  /* USER CODE BEGIN StartProcessTask */
  DisplayData_t display_data;
  uint16_t adc_value;
  /* Infinite loop */
  for (;;) {
    if (xQueueReceive(xLightQueueHandle, &adc_value, portMAX_DELAY) == pdPASS) {
      // 关闭所有LED
      LED_Off(0);
      LED_Off(1);
      LED_Off(2);

      // 计算光照强度百分比（值越大，光照越弱），带饱和与下限保护
      uint8_t darkness_percent = CalculateDarknessPercent(adc_value);
      // uint8_t darkness_percent = (adc_value - ADC_MIN) * 100 / (ADC_MAX - ADC_MIN);

      // 根据黑暗百分比点亮LED
      if (darkness_percent >= 75) {      // 全暗：三灯全亮
        LED_On(0);
        LED_On(1);
        LED_On(2);
        display_data.led_status = 0x07; // 二进制 00000111
      } else if (darkness_percent >= 50) { // 较暗：亮两灯
        LED_On(0);
        LED_On(1);
        display_data.led_status = 0x03;  // 00000011
      } else if (darkness_percent >= 25) { // 中等：亮一灯
        LED_On(0);
        display_data.led_status = 0x01;  // 00000001
      } else {                            // 全亮：全灭
        display_data.led_status = 0x00;  // 00000000
      }

      // 发送数据到显示队列
      display_data.light_value = adc_value;
      xQueueSend(xDisplayQueueHandle, &display_data, 0);
    }
  }
  /* USER CODE END StartProcessTask */
}


/* USER CODE END PrivateFunctions */

/* USER CODE BEGIN Header_StartProcessTask */
/**
* @brief Function implementing the Process_Task thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartProcessTask */

/* USER CODE BEGIN Header_StartDisplayTask */
/**
* @brief Function implementing the Display_Task thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartDisplayTask */
void StartDisplayTask(void *argument)
{
  /* USER CODE BEGIN StartDisplayTask */
  // 定义显示相关的变量
  DisplayData_t display_data;         // 显示任务需要的数据（光强 + LED 状态）
  char str_buf[16];                   // 百分比字符串缓冲区
  static uint8_t history_index = 0;   // 光强曲线的写入索引
  static uint8_t light_history[128] = {0}; // 存储最近 128 次采样的映射值
  const ASCIIFont *small_font = &afont8x6; // 使用 6x8 字体显示顶部文字

  // 曲线显示区域参数定义
  const uint8_t graph_start_y = 16;   // 曲线区域起始y坐标
  const uint8_t graph_height = 48;    // 曲线区域高度
  const uint8_t graph_end_y = graph_start_y + graph_height; // 曲线区域结束y坐标

  // 初始化OLED显示
  OLED_NewFrame();                    // 创建新的显示帧
  OLED_PrintASCIIString(0, 0, "Dark:", small_font, OLED_COLOR_NORMAL); // 显示固定标签"Dark:"
  
  // 绘制初始坐标网格
  // DrawCoordinateGrid(graph_start_y, graph_height);
  
  OLED_ShowFrame();                   // 将帧内容显示到OLED屏幕上

  // 任务主循环
  for (;;) {
    // 从显示队列接收数据，无限等待直到有数据
    if (xQueueReceive(xDisplayQueueHandle, &display_data, portMAX_DELAY) == pdPASS) {
      // 1. 计算光强对应的黑暗百分比
      uint8_t darkness = CalculateDarknessPercent(display_data.light_value);
      // 将百分比转换为字符串，格式化为三位数加百分号
      snprintf(str_buf, sizeof(str_buf), "%3d%%", darkness);

      // 计算显示位置参数
      const uint8_t label_width = 5 * small_font->w;      // "Dark:"标签的宽度
      const uint8_t value_x = label_width;                // 百分比数值的起始x坐标
      const uint8_t value_width = 4 * small_font->w;      // 百分比数值的宽度

      // 2. 刷新"百分比"显示区域
      // 先用反色矩形清除原来的显示区域
      OLED_DrawFilledRectangle(value_x, 0, value_width - 1, small_font->h, OLED_COLOR_REVERSED);
      // 显示新的百分比数值
      OLED_PrintASCIIString(value_x, 0, str_buf, small_font, OLED_COLOR_NORMAL);

      // 3. 刷新三个小圆形LED指示灯的显示
      const uint8_t led_radius = 2;                       // LED圆形半径
      const uint8_t led_spacing = 10;                     // LED之间的间距
      const uint8_t led_start_x = value_x + value_width + 20; // 第一个LED的x坐标
      const uint8_t led_y = 2;                            // LED的y坐标

      const uint8_t leftmost_x = led_start_x - led_radius;    // LED区域最左侧坐标
      const uint8_t rightmost_x = led_start_x + 2 * (2 * led_radius + led_spacing) + led_radius; // LED区域最右侧坐标
      const uint8_t led_area_w = rightmost_x - leftmost_x + 1; // LED显示区域的宽度
      const uint8_t led_area_h = (uint8_t)(2 * led_radius + 1); // LED显示区域的高度

      // 清除LED显示区域
      OLED_DrawFilledRectangle(leftmost_x, led_y - led_radius, led_area_w - 1, led_area_h, OLED_COLOR_REVERSED);

      // 绘制三个LED指示灯
      for (uint8_t i = 0; i < 3; i++) {
        uint8_t cx = led_start_x + i * (2 * led_radius + led_spacing); // 计算每个LED的x中心坐标
        if (display_data.led_status & (1 << i)) {
          // 如果对应位为1，显示点亮的LED（实心圆）
          OLED_DrawFilledCircle(cx, led_y, led_radius, OLED_COLOR_NORMAL);
        } else {
          // 如果对应位为0，显示熄灭的LED（空心圆）
          OLED_DrawCircle(cx, led_y, led_radius, OLED_COLOR_NORMAL);
        }
      }

      // 4. 更新光强曲线缓冲并重新绘制
      // 将ADC数值(0-4095)映射到显示高度(0-graph_height)
      int32_t scaled = (int32_t)display_data.light_value * graph_height / 4095;
      // 反转y轴方向，使光强越强，曲线越低（符合直觉）
      int32_t curve_value = graph_height - scaled;
      // 限制曲线值在有效范围内
      if (curve_value < 0) {
        curve_value = 0;
      } else if (curve_value > graph_height) {
        curve_value = graph_height;
      }
      
      // 对当前采样点进行平滑处理
      // 如果历史数组中已有数据，计算与前一个值的平均值以平滑过渡
      if (history_index > 0 || light_history[127] > 0) {
        uint8_t prev_idx = (history_index == 0) ? 127 : (history_index - 1);
        uint8_t prev_value = light_history[prev_idx];
        // 限制变化率，使曲线平滑
        int16_t diff = (int16_t)curve_value - prev_value;
        if (diff > 3) curve_value = prev_value + 3;
        else if (diff < -3) curve_value = prev_value - 3;
      }
      
      // 存储当前采样点到历史数组
      light_history[history_index] = (uint8_t)curve_value;
      // 循环更新索引，形成环形缓冲区
      history_index = (history_index + 1) % 128;

      // 清除曲线显示区域，保留网格线
      OLED_DrawFilledRectangle(0, graph_start_y, 127, graph_height, OLED_COLOR_REVERSED);
      
      // 重新绘制坐标网格
      // DrawCoordinateGrid(graph_start_y, graph_height);
      
      // 绘制光强曲线，使用线段连接点，而不仅仅是点
      uint8_t prev_x = 0;
      uint8_t prev_y = graph_start_y + light_history[(history_index) % 128];
      
      for (uint8_t x = 1; x < 128; x++) {
        // 计算对应的历史数据索引
        uint8_t data_idx = (history_index + x) % 128;
        // 计算y坐标（曲线上的点）
        uint8_t y = graph_start_y + light_history[data_idx];
        
        // 确保y在有效范围内并绘制线段
        if (y < 64 && prev_y < 64) {
          // 绘制从上一点到当前点的线段
          OLED_DrawLine(prev_x, prev_y, x, y, OLED_COLOR_NORMAL);
        }
        
        // 更新上一点的坐标
        prev_x = x;
        prev_y = y;
      }

      // 5. 将缓冲区一次性输出到屏幕，避免闪烁
      OLED_ShowFrame();
    }
  }
  /* USER CODE END StartDisplayTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

static uint8_t CalculateDarknessPercent(uint16_t adc_value)
{
  /* USER CODE BEGIN StarSensorTask */
  int32_t clamped = (int32_t)adc_value;
  const int32_t min_val = (int32_t)ADC_MIN;
  const int32_t max_val = (int32_t)ADC_MAX;
  int32_t range = max_val - min_val;
  if (range <= 0) {
    return 0;
  }
  if (clamped < min_val) {
    clamped = min_val;
  } else if (clamped > max_val) {
    clamped = max_val;
  }
  return (uint8_t)(((clamped - min_val) * 100) / range);
  /* USER CODE END StarSensorTask */
}

// 绘制坐标网格函数
static void DrawCoordinateGrid(uint8_t start_y, uint8_t height) {
  // 绘制水平网格线（每隔12个像素）
  for (uint8_t i = 0; i <= height; i += 12) {
    // 虚线效果
    for (uint8_t x = 0; x < 128; x += 4) {
      OLED_SetPixel(x, start_y + i, OLED_COLOR_NORMAL);
      OLED_SetPixel(x+1, start_y + i, OLED_COLOR_NORMAL);
    }
  }
  
  // 绘制垂直网格线（每隔16个像素）
  for (uint8_t i = 0; i < 128; i += 16) {
    // 虚线效果
    for (uint8_t y = start_y; y < start_y + height; y += 4) {
      OLED_SetPixel(i, y, OLED_COLOR_NORMAL);
      OLED_SetPixel(i, y+1, OLED_COLOR_NORMAL);
    }
  }
  
  // 绘制外框
  OLED_DrawRectangle(0, start_y, 127, start_y + height - 1, OLED_COLOR_NORMAL);
}
/* USER CODE END Application */

