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
typedef enum {
    FACE_NORMAL,
    FACE_BLINK,
    FACE_DIZZY // 预留晕的状态
} FaceState_t;

void StartguiTask(void *argument)
{
  // 1. 初始化 (保持不变)
  u8g2_Setup_ssd1306_i2c_128x64_noname_f(&u8g2, U8G2_R0, u8x8_byte_stm32_hw_i2c, u8x8_stm32_gpio_and_delay);
  u8x8_SetI2CAddress(&u8g2.u8x8, 0x78);
  u8g2_InitDisplay(&u8g2);
  u8g2_SetPowerSave(&u8g2, 0);

  // 2. 变量定义
  InputEvent_t recv_evt;
  UI_Page_t current_page = PAGE_HOME;
  
  int8_t menu_index = 0;
  const int8_t MENU_MAX = 3;
  char *menu_items[] = {"Return", "Pomodoro", "Settings"};

  // === 动画变量 ===
  uint32_t last_blink_time = 0;
  FaceState_t current_face = FACE_NORMAL;
  
  // === 【关键修改】热度与时间戳 ===
  int16_t typing_heat = 0;      // 热度值
  uint32_t last_key_time = 0;   // 最后一次按键的时间戳

  for(;;)
  {
    // 20ms 刷新率
    osStatus_t status = osMessageQueueGet(guiEventQueueHandle, &recv_evt, NULL, 20);
    if (status != osOK) recv_evt = EVENT_NONE;

    uint32_t now = osKernelGetTickCount(); // 获取当前系统时间

    // ================= 逻辑处理 =================
    
    // --- 1. 捕捉按键，更新时间戳 ---
    if (recv_evt == EVENT_KEY_HIT) {
        last_key_time = now; // 更新最后按键时间
        typing_heat += 20;   // 升温：数值越大越容易晕
    }

    // --- 2. 热度自然衰减 (用于触发眩晕) ---
    // 只有在 3秒内 有输入时，才计算热度逻辑
    if (now - last_key_time < 3000) {
        typing_heat -= 1; // 缓慢降温
    } else {
        // --- 3. 【核心逻辑】超时 3秒 强制恢复 ---
        typing_heat = 0;            // 热度归零
        current_face = FACE_NORMAL; // 强制恢复正常脸
    }
    
    // 限制热度范围
    if (typing_heat > 100) typing_heat = 100;
    if (typing_heat < 0) typing_heat = 0;

    // --- 4. 状态切换判定 ---
    // 只有没超时的时候，才允许变晕
    if (now - last_key_time < 3000) {
        if (typing_heat > 80) {
            current_face = FACE_DIZZY; // 热度超过60变晕
        }
        // 注意：这里去掉了“降到40恢复”的逻辑，完全交给 3秒倒计时来恢复
        // 这样只要你一旦晕了，就会晕满 3秒 (或者直到你停止输入3秒)
    }

    // --- 5. 页面跳转逻辑 (保持不变) ---
    switch (current_page) 
    {
        case PAGE_HOME:
            if (recv_evt == EVENT_ENCODER_UP || 
                recv_evt == EVENT_ENCODER_DOWN || 
                recv_evt == EVENT_ENCODER_CLICK) 
            {
                current_page = PAGE_MENU;
                menu_index = 0;
            }
            break;
        // ... (其他页面逻辑不变) ...
        case PAGE_MENU:
            if (recv_evt == EVENT_ENCODER_DOWN) menu_index++;
            if (recv_evt == EVENT_ENCODER_UP)   menu_index--;
            if (menu_index < 0) menu_index = MENU_MAX - 1;
            if (menu_index >= MENU_MAX) menu_index = 0;
            if (recv_evt == EVENT_ENCODER_CLICK) {
                if (menu_index == 0) current_page = PAGE_HOME;
            }
            break;
        case PAGE_POMODORO:
        case PAGE_GAME:
             if (recv_evt == EVENT_ENCODER_CLICK) current_page = PAGE_MENU;
             break;
    }

    // ================= 绘图处理 (保持不变) =================
    u8g2_ClearBuffer(&u8g2);

    switch (current_page) 
    {
        case PAGE_HOME:
            if (current_face == FACE_DIZZY) {
                // 晕脸绘制 (X 眼睛)
                u8g2_DrawLine(&u8g2, 22, 15, 42, 35);
                u8g2_DrawLine(&u8g2, 42, 15, 22, 35);
                u8g2_DrawLine(&u8g2, 86, 15, 106, 35);
                u8g2_DrawLine(&u8g2, 106, 15, 86, 35);
                u8g2_DrawCircle(&u8g2, 64, 52, 6, U8G2_DRAW_ALL);
                // 蚊香圈
                u8g2_DrawLine(&u8g2, 58, 5, 70, 5);
                u8g2_DrawLine(&u8g2, 60, 2, 68, 2);
            }
            else { // FACE_NORMAL
                // 正常脸绘制 (眨眼动画)
                int is_blink = 0;
                // 只有不是晕的时候才眨眼
                if (now - last_blink_time > 3500) {
                    is_blink = 1;
                    if (now - last_blink_time > 3650) {
                        is_blink = 0;
                        last_blink_time = now;
                    }
                }
                if (is_blink) {
                    u8g2_DrawBox(&u8g2, 18, 23, 28, 4);
                    u8g2_DrawBox(&u8g2, 82, 23, 28, 4);
                } else {
                    u8g2_DrawFilledEllipse(&u8g2, 32, 25, 12, 18, U8G2_DRAW_ALL);
                    u8g2_DrawFilledEllipse(&u8g2, 96, 25, 12, 18, U8G2_DRAW_ALL);
                    u8g2_SetDrawColor(&u8g2, 0); 
                    u8g2_DrawDisc(&u8g2, 36, 20, 3, U8G2_DRAW_ALL);
                    u8g2_DrawDisc(&u8g2, 100, 20, 3, U8G2_DRAW_ALL);
                    u8g2_SetDrawColor(&u8g2, 1);
                }
                u8g2_DrawRBox(&u8g2, 44, 50, 40, 8, 3);
            }
            break;

        case PAGE_MENU:
            // 菜单绘制保持不变...
            u8g2_SetFont(&u8g2, u8g2_font_ncenB10_tr);
            for (int i = 0; i < MENU_MAX; i++) {
                int y_pos = 20 + i * 20;
                if (i == menu_index) {
                    u8g2_DrawRBox(&u8g2, 10, y_pos - 14, 108, 19, 4);
                    u8g2_SetDrawColor(&u8g2, 0);
                } else {
                    u8g2_SetDrawColor(&u8g2, 1);
                }
                int str_w = u8g2_GetStrWidth(&u8g2, menu_items[i]);
                u8g2_DrawStr(&u8g2, (128 - str_w) / 2, y_pos, menu_items[i]);
            }
            u8g2_SetDrawColor(&u8g2, 1);
            break;
    }

    u8g2_SendBuffer(&u8g2);
  }
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
    
    if (abs(diff) >= 4) { // 假设转一格计数变2
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
  // 1. 原有的 USB 发送
  USB_Keyboard_HandleMatrixKey(key_code, pressed);
  
  // 2. 新增：如果是按下动作，通知 GUI 增加热度
  if (pressed) {
      InputEvent_t evt = EVENT_KEY_HIT;
      // timeout=0 表示如果不阻塞，队列满了就丢弃，不影响打字
      osMessageQueuePut(guiEventQueueHandle, &evt, 0, 0); 
  }
}

/* USER CODE END Application */

