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
#include "usb_keyboard.h"
#include "matrix_keyboard.h"
#include "usbd_hid.h"
#include "OLED.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
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

/* USER CODE END Variables */
/* Definitions for keyboardTask */
osThreadId_t keyboardTaskHandle;
const osThreadAttr_t keyboardTask_attributes = {
  .name = "keyboardTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for guiTask */
osThreadId_t guiTaskHandle;
const osThreadAttr_t guiTask_attributes = {
  .name = "guiTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for encoderTask */
osThreadId_t encoderTaskHandle;
const osThreadAttr_t encoderTask_attributes = {
  .name = "encoderTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityLow,
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void StartkeyboardTask(void *argument);
void StartguiTask(void *argument);
void StartencoderTask(void *argument);

extern void MX_USB_DEVICE_Init(void);
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

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of keyboardTask */
  keyboardTaskHandle = osThreadNew(StartkeyboardTask, NULL, &keyboardTask_attributes);

  /* creation of guiTask */
  guiTaskHandle = osThreadNew(StartguiTask, NULL, &guiTask_attributes);

  /* creation of encoderTask */
  encoderTaskHandle = osThreadNew(StartencoderTask, NULL, &encoderTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_StartkeyboardTask */
/**
  * @brief  Function implementing the keyboardTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartkeyboardTask */
void StartkeyboardTask(void *argument)
{
  /* init code for USB_DEVICE */
  MX_USB_DEVICE_Init();
  /* USER CODE BEGIN StartkeyboardTask */
 
 
 Matrix_Keyboard_Init();
  MX_USB_DEVICE_Init();
 
  uint32_t tick = osKernelGetTickCount(); 
  // 定义周期为 10ms (假设你的配置是 1000Hz, 那么 10 ticks 就是 10ms)
  const uint32_t frequency = 10;
  /* Infinite loop */
  for(;;)
  {
    // 1. 执行矩阵扫描
    Matrix_Keyboard_Scan();

    // 2. 绝对延时：确保从上一次唤醒到下一次唤醒精确间隔 10ms
    // 这会自动扣除 Matrix_Keyboard_Scan() 运行所占用的时间
    tick += frequency;
    osDelayUntil(tick);
  }
  /* USER CODE END StartkeyboardTask */
}

/* USER CODE BEGIN Header_StartguiTask */
/**
* @brief Function implementing the guiTakk thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartguiTask */
void StartguiTask(void *argument)
{
  /* USER CODE BEGIN StartguiTask */
  OLED_Init(); // OLED初始化


   uint32_t count = 0; // 计数变量
  
  /* Infinite loop */
  for(;;)
  {
    OLED_Clear();

    // 3. 绘制内容
    // 显示静态文字
    OLED_ShowString(0, 0, "STM32 OLED Test", OLED_8X16);
    OLED_ShowString(0, 16, "Count:", OLED_8X16);
    
    // 显示动态数字
    OLED_ShowNum(48, 16, count, 5, OLED_8X16);
    
    // 画一个简单的图形测试（画一个矩形框）
    OLED_DrawRectangle(0, 34, 127, 10, OLED_UNFILLED);
    
    // 画一个动态的进度条（在矩形框内部）
    uint8_t width = (count % 100) * 1.25; // 简单的计算，让它循环变长
    OLED_DrawRectangle(2, 36, width, 6, OLED_FILLED);
    
    // 4. 将显存写入屏幕
    OLED_Update();

    // 5. 更新数据
    count++;
    
    // 6. 延时 (FreeRTOS 延时，让出CPU权)
    osDelay(50); 
  }
  /* USER CODE END StartguiTask */
}

/* USER CODE BEGIN Header_StartencoderTask */
/**
* @brief Function implementing the encoderTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartencoderTask */
void StartencoderTask(void *argument)
{
  /* USER CODE BEGIN StartencoderTask */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END StartencoderTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */
void Matrix_Key_Callback(uint8_t key_code, uint8_t pressed) {
  const char *key_names[] = {"1", "2", "3", "4", "5", "6", "7", "8", "9"};


  /* Send to USB HID */
  USB_Keyboard_HandleMatrixKey(key_code, pressed);
}
/* USER CODE END Application */

