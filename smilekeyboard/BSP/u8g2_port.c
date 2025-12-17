#include "main.h"
#include "u8g2.h" 
#include "stm32f1xx_hal.h" // 确保引用了 F1 的 HAL 库

// 定义 OLED 的 I2C 地址 (通常是 0x3C 或 0x78，u8g2 内部会自动处理读写位)
// 这里填 0x78 是因为 HAL 库通常使用 8位地址 (0x3C << 1)
#define OLED_I2C_ADDRESS  0x78 

// 引用 CubeMX 生成的 I2C 句柄
extern I2C_HandleTypeDef hi2c1; 

// =============================================
// 回调函数 1: 硬件 I2C 通信
// =============================================
uint8_t u8x8_byte_stm32_hw_i2c(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
    static uint8_t buffer[32]; // u8g2 发送缓冲区
    static uint8_t buf_idx;
    uint8_t *data;

    switch(msg)
    {
    case U8X8_MSG_BYTE_SEND:
        // 收集要发送的数据
        data = (uint8_t *)arg_ptr;
        while( arg_int > 0 )
        {
            buffer[buf_idx++] = *data;
            data++;
            arg_int--;
        }
        break;
        
    case U8X8_MSG_BYTE_INIT:
        /* I2C 已经在 main.c 里初始化了，这里留空 */
        break;
        
    case U8X8_MSG_BYTE_SET_DC:
        /* I2C 不需要 DC 引脚，留空 */
        break;
        
    case U8X8_MSG_BYTE_START_TRANSFER:
        buf_idx = 0; // 开始传输，重置缓冲区索引
        break;
        
    case U8X8_MSG_BYTE_END_TRANSFER:
        // 结束传输，真正调用 HAL 库发送数据
        // 注意：如果你用的是 FreeRTOS，这里最好加一个超时时间，或者改用 DMA
        if(HAL_I2C_Master_Transmit(&hi2c1, OLED_I2C_ADDRESS, buffer, buf_idx, 100) != HAL_OK) 
            return 0;
        break;
        
    default:
        return 0;
    }
    return 1;
}

// =============================================
// 回调函数 2: GPIO 和 延时
// =============================================
uint8_t u8x8_stm32_gpio_and_delay(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
    switch(msg)
    {
    case U8X8_MSG_GPIO_AND_DELAY_INIT:
        // GPIO 初始化已在 main.c 完成
        break;

    case U8X8_MSG_DELAY_MILLI:
        // 毫秒延时
        // 如果在 FreeRTOS 任务中，建议用 osDelay(arg_int); 以释放 CPU
        // 如果在初始化阶段（任务开始前），只能用 HAL_Delay(arg_int);
        HAL_Delay(arg_int); 
        break;

    case U8X8_MSG_GPIO_I2C_CLOCK:
    case U8X8_MSG_GPIO_I2C_DATA:
        // 硬件 I2C 不需要手动控制 SCL/SDA 电平
        break;
        
    case U8X8_MSG_GPIO_RESET:
        // 如果你的 OLED 有 RES 引脚，在这里操作 GPIO
        // 如果是 4针 OLED (VCC,GND,SCL,SDA)，这里留空即可
        break;
    }
    return 1;
}