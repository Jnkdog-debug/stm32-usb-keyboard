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
#include "tim.h"
#include "u8g2.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
/* u8g2 实体定义 */
static u8g2_t u8g2;

// 声明你的移植回调函数 (在 u8g2_port.c 中实现)
extern uint8_t u8x8_byte_stm32_hw_i2c(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
extern uint8_t u8x8_stm32_gpio_and_delay(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
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
/* Definitions for guiEventQueue */
osMessageQueueId_t guiEventQueueHandle;
const osMessageQueueAttr_t guiEventQueue_attributes = {
  .name = "guiEventQueue"
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

  /* Create the queue(s) */
  /* creation of guiEventQueue */
  guiEventQueueHandle = osMessageQueueNew (16, sizeof(InputEvent_t), &guiEventQueue_attributes);

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
  
  /* ================= 1. 初始化阶段 ================= */
  
  // 初始化 u8g2 (SSD1306, 128x64, 硬件I2C, 全屏缓冲_f)
  // 如果内存不够(F103 RAM紧张)，可以将 _f 改为 _1 (页缓冲模式)，但绘图逻辑要改
  u8g2_Setup_ssd1306_i2c_128x64_noname_f(&u8g2, U8G2_R0, u8x8_byte_stm32_hw_i2c, u8x8_stm32_gpio_and_delay);
  
  // 设置设备地址 (通常是 0x78)
  u8x8_SetI2CAddress(&u8g2.u8x8, 0x78);
  
  // 启动显示
  u8g2_InitDisplay(&u8g2);
  u8g2_SetPowerSave(&u8g2, 0); // 唤醒屏幕

  /* ================= 2. 局部变量定义 ================= */
  
  InputEvent_t recv_evt;       // 接收到的队列消息
  UI_Page_t current_page = PAGE_HOME; // 当前页面状态
  
  int8_t menu_index = 0;       // 菜单光标位置
  const int8_t MENU_MAX = 3;   // 菜单项数量
  char *menu_items[] = {"Back", "Pomodoro", "Game"}; // 菜单文字

  uint32_t last_blink_time = 0; // 控制眨眼的时间戳
  uint8_t is_eye_closed = 0;    // 眼睛状态

  /* ================= 3. 任务主循环 ================= */
  for(;;)
  {
    // --- A. 接收输入 (带超时机制) ---
    // 这里设置 20ms 超时。意味着：
    // 1. 如果有按键，立即响应，无延迟。
    // 2. 如果没按键，20ms 后也会向下执行，保证屏幕能刷新动画(眨眼)。
    osStatus_t status = osMessageQueueGet(guiEventQueueHandle, &recv_evt, NULL, 20);
    
    // 如果超时没收到消息，重置事件为 NONE
    if (status != osOK) {
        recv_evt = EVENT_NONE;
    }

    // --- B. 逻辑处理 (状态机) ---
    switch (current_page) 
    {
        // ------------- 主页逻辑 -------------
        case PAGE_HOME:
            // 响应点击 -> 进菜单
            if (recv_evt == EVENT_ENCODER_CLICK) {
                current_page = PAGE_MENU;
                menu_index = 0; // 重置光标
            }
            break;

        // ------------- 菜单逻辑 -------------
        case PAGE_MENU:
            // 响应旋转 -> 移动光标
            if (recv_evt == EVENT_ENCODER_DOWN) menu_index++;
            if (recv_evt == EVENT_ENCODER_UP)   menu_index--;
            
            // 限制光标范围
            if (menu_index < 0) menu_index = 0;
            if (menu_index >= MENU_MAX) menu_index = MENU_MAX - 1;

            // 响应点击 -> 执行功能
            if (recv_evt == EVENT_ENCODER_CLICK) {
                if (menu_index == 0) current_page = PAGE_HOME;     // Back
                if (menu_index == 1) current_page = PAGE_POMODORO; // Pomodoro
                if (menu_index == 2) current_page = PAGE_GAME;     // Game
            }
            break;

        // ------------- 其他页面逻辑 -------------
        case PAGE_POMODORO:
        case PAGE_GAME:
            // 点击返回主菜单
            if (recv_evt == EVENT_ENCODER_CLICK) {
                current_page = PAGE_MENU;
            }
            break;
    }

    // --- C. 绘图处理 (u8g2) ---
    u8g2_ClearBuffer(&u8g2); // 清空缓冲区

    switch (current_page) 
    {
        // ============= 绘制主页 (动态脸) =============
        case PAGE_HOME:
            // 1. 设置字体画文字
            u8g2_SetFont(&u8g2, u8g2_font_ncenB08_tr);
            u8g2_DrawStr(&u8g2, 0, 10, "Speed: 0 WPM"); 
            
            // 2. 计算眨眼动画 (每3秒眨眼一次)
            uint32_t now = osKernelGetTickCount();
            if (now - last_blink_time > 3000) {
                is_eye_closed = 1; // 闭眼
                if (now - last_blink_time > 3200) { // 闭眼维持200ms
                    is_eye_closed = 0; // 睁眼
                    last_blink_time = now;
                }
            }

            // 3. 画脸
            u8g2_DrawCircle(&u8g2, 64, 40, 20, U8G2_DRAW_ALL); // 脸轮廓
            
            if (is_eye_closed) {
                // 闭眼：画两条横线
                u8g2_DrawLine(&u8g2, 54, 38, 60, 38); // 左眼
                u8g2_DrawLine(&u8g2, 68, 38, 74, 38); // 右眼
            } else {
                // 睁眼：画两个实心圆
                u8g2_DrawDisc(&u8g2, 57, 38, 3, U8G2_DRAW_ALL); // 左眼
                u8g2_DrawDisc(&u8g2, 71, 38, 3, U8G2_DRAW_ALL); // 右眼
            }
            
            // 画嘴巴 (画一个圆弧模拟微笑)
            // Center(64,40), Radius 12, Angle 45~135 degree
            // 注意：u8g2 只有 DrawCircle，没有简单的 DrawArc，这里用简单的线代替嘴巴
            u8g2_DrawLine(&u8g2, 58, 50, 70, 50); 
            break;

        // ============= 绘制菜单 =============
        case PAGE_MENU:
            u8g2_SetFont(&u8g2, u8g2_font_ncenB08_tr);
            u8g2_DrawStr(&u8g2, 40, 10, "- MENU -");
            
            // 循环绘制菜单项
            for (int i = 0; i < MENU_MAX; i++) {
                // 如果是当前选中项，画一个实心框作为背景（反色显示）
                if (i == menu_index) {
                    u8g2_SetDrawColor(&u8g2, 1); // 正常色
                    u8g2_DrawBox(&u8g2, 0, 16 + i*16, 128, 14); // 画框
                    u8g2_SetDrawColor(&u8g2, 0); // 设为背景色(黑色)，实现文字反白
                } else {
                    u8g2_SetDrawColor(&u8g2, 1);
                }
                
                // 绘制文字 (Y坐标需要微调以垂直居中)
                u8g2_DrawStr(&u8g2, 10, 27 + i*16, menu_items[i]);
            }
            // 恢复颜色设置，以免影响下次循环
            u8g2_SetDrawColor(&u8g2, 1); 
            break;

        // ============= 绘制番茄钟 =============
        case PAGE_POMODORO:
            u8g2_SetFont(&u8g2, u8g2_font_ncenB14_tr); // 大字体
            u8g2_DrawStr(&u8g2, 35, 40, "25:00");
            u8g2_SetFont(&u8g2, u8g2_font_ncenB08_tr);
            u8g2_DrawStr(&u8g2, 30, 60, "Click to Exit");
            break;
            
        // ============= 绘制游戏 =============
        case PAGE_GAME:
            u8g2_SetFont(&u8g2, u8g2_font_ncenB08_tr);
            u8g2_DrawStr(&u8g2, 20, 30, "Game Area...");
            u8g2_DrawFrame(&u8g2, 10, 10, 108, 44); // 画个框假装是游戏界面
            break;
    }

    // --- D. 发送显存到屏幕 ---
    u8g2_SendBuffer(&u8g2);
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
   HAL_TIM_Encoder_Start(&htim2, TIM_CHANNEL_ALL);
  int16_t last_counter = 0;


  /* Infinite loop */
  for(;;)
  {
     // 1. 处理旋转
    int16_t current_counter = (int16_t)__HAL_TIM_GET_COUNTER(&htim2);
    int16_t diff = current_counter - last_counter;
    
    if (abs(diff) >= 2) { // 假设转一格计数变2
        InputEvent_t evt = (diff > 0) ? EVENT_ENCODER_DOWN : EVENT_ENCODER_UP;
        osMessageQueuePut(guiEventQueueHandle, &evt, 0, 0); // 发送到队列
        last_counter = current_counter;
    }
    
    // 2. 处理按键 (此处省略消抖逻辑)
    if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_5) == GPIO_PIN_RESET) {
        InputEvent_t evt = EVENT_ENCODER_CLICK;
        osMessageQueuePut(guiEventQueueHandle, &evt, 0, 0);
        // 等待松开，防止重复触发
        while(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_5) == GPIO_PIN_RESET) osDelay(20);
    }
    
    osDelay(20);

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

