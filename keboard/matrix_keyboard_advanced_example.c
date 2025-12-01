/**
  ******************************************************************************
  * @file           : matrix_keyboard_advanced_example.c
  * @brief          : Advanced matrix keyboard example with UART output
  * 
  * 这是一个高级示例，展示如何在实际应用中使用矩阵键盘
  * 注意: 这个文件不需要编译到项目中, 仅作为参考示例
  ******************************************************************************
  */

#include "main.h"
#include "matrix_keyboard.h"
#include <stdio.h>
#include <string.h>

/* ============================================================================
 * 方式1: 使用回调函数处理按键事件 (推荐)
 * ============================================================================
 */

/**
 * 用户自定义回调函数 - 处理按键事件
 * 在 main.c 中替换原有的 Matrix_Key_Callback() 实现
 */
void Matrix_Key_Callback_Example1(uint8_t key_code, uint8_t pressed)
{
    const char *key_names[] = {
        "0", "1", "2",
        "3", "4", "5",
        "6", "7", "8"
    };
    
    if (pressed) {
        /* 按键被按下 */
        printf("Key %s pressed\r\n", key_names[key_code]);
        
        /* 根据不同按键执行不同操作 */
        switch (key_code) {
            case 0:  /* 按键0 */
                printf("Executing action for key 0\r\n");
                // LED_ON();
                break;
            case 1:  /* 按键1 */
                printf("Executing action for key 1\r\n");
                // Motor_Start();
                break;
            case 4:  /* 按键4 - 中间按键 */
                printf("Center key pressed - This could be enter/confirm\r\n");
                break;
            default:
                break;
        }
    } else {
        /* 按键被释放 */
        printf("Key %s released\r\n", key_names[key_code]);
    }
}


/* ============================================================================
 * 方式2: 使用状态查询 (轮询模式)
 * ============================================================================
 */

/**
 * 在主循环中轮询按键状态 (不使用回调)
 * 将此代码添加到 main() 的 while(1) 中替换 Matrix_Keyboard_Scan()
 */
void Keyboard_Polling_Example(void)
{
    static uint8_t last_state[3][3] = {0};
    uint8_t current_state;
    
    /* 定期扫描 */
    static uint32_t scan_timer = 0;
    if ((HAL_GetTick() - scan_timer) >= 10) {
        scan_timer = HAL_GetTick();
        Matrix_Keyboard_Scan();
        
        /* 轮询检查所有按键状态 */
        for (uint8_t row = 0; row < 3; row++) {
            for (uint8_t col = 0; col < 3; col++) {
                current_state = Matrix_Get_Key_Status(row, col);
                
                /* 检测状态变化 */
                if (current_state != last_state[row][col]) {
                    if (current_state) {
                        printf("Key[%d][%d] pressed\r\n", row, col);
                    } else {
                        printf("Key[%d][%d] released\r\n", row, col);
                    }
                    last_state[row][col] = current_state;
                }
            }
        }
    }
}


/* ============================================================================
 * 方式3: 功能性键盘 - 计算器示例
 * ============================================================================
 */

typedef struct {
    int32_t result;
    int32_t operand;
    char operation;
    uint8_t new_number;
} Calculator;

static Calculator calc = {0, 0, '+', 1};

/**
 * 计算器回调函数实现
 */
void Matrix_Key_Callback_Calculator(uint8_t key_code, uint8_t pressed)
{
    if (!pressed) return;  /* 仅在按下时处理 */
    
    if (key_code >= 0 && key_code <= 8) {
        if (key_code >= 0 && key_code <= 2) {
            /* 数字键 0-2 */
            if (calc.new_number) {
                calc.operand = key_code;
                calc.new_number = 0;
            } else {
                calc.operand = calc.operand * 10 + key_code;
            }
            printf("Display: %d\r\n", calc.operand);
        }
        else if (key_code >= 3 && key_code <= 5) {
            /* 操作符: +, -, * */
            if (calc.operation == '+') calc.result += calc.operand;
            else if (calc.operation == '-') calc.result -= calc.operand;
            else if (calc.operation == '*') calc.result *= calc.operand;
            
            calc.operation = (key_code == 3) ? '+' : (key_code == 4) ? '-' : '*';
            calc.new_number = 1;
            printf("Operation: %c\r\n", calc.operation);
        }
        else if (key_code == 6) {
            /* 等号 - 显示结果 */
            if (calc.operation == '+') calc.result += calc.operand;
            else if (calc.operation == '-') calc.result -= calc.operand;
            else if (calc.operation == '*') calc.result *= calc.operand;
            printf("Result: %d\r\n", calc.result);
            calc.new_number = 1;
        }
    }
}


/* ============================================================================
 * 方式4: 游戏控制器模式 - 贪吃蛇/俄罗斯方块
 * ============================================================================
 */

typedef enum {
    DIR_UP,
    DIR_DOWN,
    DIR_LEFT,
    DIR_RIGHT,
    ACTION_FIRE,
    ACTION_PAUSE,
    ACTION_MENU
} GameAction;

/**
 * 游戏控制器回调
 * 按键映射:
 * 1=UP, 2=DOWN, 3=LEFT, 4=RIGHT, 5=FIRE, 7=PAUSE, 8=MENU
 */
void Matrix_Key_Callback_GameController(uint8_t key_code, uint8_t pressed)
{
    if (!pressed) return;  /* 仅处理按键按下事件 */
    
    GameAction action;
    const char *action_names[] = {"UP", "DOWN", "LEFT", "RIGHT", "FIRE", "PAUSE", "MENU"};
    
    switch (key_code) {
        case 1:
            action = DIR_UP;
            printf("Action: %s\r\n", action_names[action]);
            // Move_Up();
            break;
        case 2:
            action = DIR_DOWN;
            printf("Action: %s\r\n", action_names[action]);
            // Move_Down();
            break;
        case 3:
            action = DIR_LEFT;
            printf("Action: %s\r\n", action_names[action]);
            // Move_Left();
            break;
        case 4:
            action = DIR_RIGHT;
            printf("Action: %s\r\n", action_names[action]);
            // Move_Right();
            break;
        case 5:
            action = ACTION_FIRE;
            printf("Action: %s\r\n", action_names[action]);
            // Fire();
            break;
        case 7:
            action = ACTION_PAUSE;
            printf("Action: %s\r\n", action_names[action]);
            // Pause_Game();
            break;
        case 8:
            action = ACTION_MENU;
            printf("Action: %s\r\n", action_names[action]);
            // Show_Menu();
            break;
        default:
            break;
    }
}


/* ============================================================================
 * 方式5: 密码输入系统
 * ============================================================================
 */

#define PASSWORD_MAX_LENGTH 4
#define PASSWORD "1234"

typedef struct {
    char input[PASSWORD_MAX_LENGTH + 1];
    uint8_t length;
    uint8_t attempt;
    uint8_t locked;
} PasswordSystem;

static PasswordSystem pwd_sys = {0, 0, 0, 0};

void Matrix_Key_Callback_Password(uint8_t key_code, uint8_t pressed)
{
    const char *digits[] = {"0", "1", "2", "3", "4", "5", "6", "7", "8"};
    
    if (!pressed) return;
    
    if (pwd_sys.locked) {
        printf("System locked! Too many failed attempts.\r\n");
        return;
    }
    
    if (key_code >= 0 && key_code <= 8) {
        /* 数字键 */
        if (pwd_sys.length < PASSWORD_MAX_LENGTH) {
            pwd_sys.input[pwd_sys.length] = '0' + key_code;
            pwd_sys.length++;
            printf("*");  /* 显示星号而不是数字 */
            
            if (pwd_sys.length == PASSWORD_MAX_LENGTH) {
                pwd_sys.input[pwd_sys.length] = '\0';
                if (strcmp(pwd_sys.input, PASSWORD) == 0) {
                    printf("\r\nPassword correct! Access granted.\r\n");
                    // Open_Door();
                } else {
                    printf("\r\nPassword incorrect! Access denied.\r\n");
                    pwd_sys.attempt++;
                    if (pwd_sys.attempt >= 3) {
                        pwd_sys.locked = 1;
                        printf("System locked due to too many failed attempts!\r\n");
                    }
                }
                pwd_sys.length = 0;
                memset(pwd_sys.input, 0, sizeof(pwd_sys.input));
            }
        }
    }
}


/* ============================================================================
 * 方式6: 按键长按/短按检测
 * ============================================================================
 */

typedef struct {
    uint32_t press_time;
    uint8_t is_pressed;
} KeyPressInfo;

static KeyPressInfo key_info[9] = {0};

/**
 * 需要在扫描函数中集成长按检测逻辑
 * 这里是回调函数部分
 */
void Matrix_Key_Callback_LongPress(uint8_t key_code, uint8_t pressed)
{
    if (pressed) {
        key_info[key_code].is_pressed = 1;
        key_info[key_code].press_time = HAL_GetTick();
        printf("Key %d pressed\r\n", key_code);
    } else {
        uint32_t hold_time = HAL_GetTick() - key_info[key_code].press_time;
        
        if (hold_time > 1000) {
            /* 长按 (>1秒) */
            printf("Key %d long pressed (%dms)\r\n", key_code, hold_time);
        } else {
            /* 短按 */
            printf("Key %d short pressed (%dms)\r\n", key_code, hold_time);
        }
        
        key_info[key_code].is_pressed = 0;
    }
}


/* ============================================================================
 * 使用说明:
 * ============================================================================
 * 
 * 选择上述某种方式, 将对应的回调函数实现复制到 main.c 中的
 * Matrix_Key_Callback() 函数中即可.
 * 
 * 例如, 要使用计算器示例:
 * 1. 将 Matrix_Key_Callback_Calculator() 的实现复制到 main.c
 * 2. 修改 main.c 中的 Matrix_Key_Callback() 如下:
 * 
 *    void Matrix_Key_Callback(uint8_t key_code, uint8_t pressed)
 *    {
 *        Matrix_Key_Callback_Calculator(key_code, pressed);
 *    }
 * 
 * 3. 重新编译并下载到STM32即可
 * 
 * ============================================================================
 */
