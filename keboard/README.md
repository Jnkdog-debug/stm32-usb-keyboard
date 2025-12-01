# 矩阵键盘驱动 - STM32F407 Demo

## 项目概述

这是一个STM32F407上的3x3矩阵键盘驱动程序，支持9个按键的扫描和检测。采用行驱动-列检测的扫描方式，内置防抖算法。

## 硬件连接

### GPIO引脚配置

```
行线 (Output - 行驱动):
  PA12 → 行0
  PA10 → 行1
  PA8  → 行2

列线 (Input - 列检测):
  PC6  → 列0
  PC7  → 列1
  PC8  → 列2
```

### 按键映射

```
        列0(PC6)  列1(PC7)  列2(PC8)
行0(PA12)  0       1        2
行1(PA10)  3       4        5
行2(PA8)   6       7        8
```

## 文件结构

```
Core/
├── Inc/
│   ├── main.h                    # 主程序头文件 (已修改)
│   └── matrix_keyboard.h         # 键盘驱动头文件 (新建)
└── Src/
    ├── main.c                    # 主程序 (已修改)
    ├── matrix_keyboard.c         # 键盘驱动实现 (新建)

keboard/
├── Matrix_Keyboard_USAGE.txt     # 使用说明文档
├── matrix_keyboard_advanced_example.c  # 高级示例 (参考)
└── README.md                     # 本文件
```

## 软件工作原理

### 扫描方法

1. **行激活**: 将一行设为低电平(LOW), 其他行设为高电平(HIGH)
2. **列读取**: 读取所有列的输入电平
3. **按键判断**: 列线为低 = 该位置的按键被按下
4. **去抖处理**: 状态改变后需要稳定20ms才确认状态变化
5. **周期重复**: 循环扫描所有行

### 特性

- ✅ **完整的去抖算法**: 20ms消抖时间(可配置)
- ✅ **低功耗设计**: 仅激活的行会消耗电流
- ✅ **事件回调**: 支持按键按下/释放事件
- ✅ **灵活的API**: 支持查询或回调两种模式
- ✅ **标准HAL库**: 易于集成到现有工程

## 快速开始

### 1. 初始化

在`main()`函数中:

```c
Matrix_Keyboard_Init();  // 初始化键盘
```

### 2. 扫描

在主循环中定期调用(建议10ms):

```c
while (1) {
    if ((HAL_GetTick() - scan_timer) >= 10) {
        scan_timer = HAL_GetTick();
        Matrix_Keyboard_Scan();
    }
}
```

### 3. 处理按键事件

实现回调函数(在main.c中已定义):

```c
void Matrix_Key_Callback(uint8_t key_code, uint8_t pressed)
{
    if (pressed) {
        printf("Key %d pressed\r\n", key_code);
    } else {
        printf("Key %d released\r\n", key_code);
    }
}
```

## API 参考

### 初始化函数

```c
void Matrix_Keyboard_Init(void);
```
初始化键盘硬件, 配置GPIO引脚和状态变量。

**参数**: 无

**返回值**: 无

**调用时机**: 在`main()`中, GPIO初始化之后

---

### 扫描函数

```c
void Matrix_Keyboard_Scan(void);
```
执行键盘扫描, 检测按键状态变化。应定期调用(建议10ms)。

**参数**: 无

**返回值**: 无

**调用时机**: 在主循环中, 定时调用

**注意**: 不要在中断中调用此函数

---

### 查询函数

```c
uint8_t Matrix_Get_Key_Status(uint8_t row, uint8_t col);
```
查询指定位置按键的当前状态。

**参数**:
- `row`: 行号 (0-2)
- `col`: 列号 (0-2)

**返回值**: 
- `1`: 按键被按下
- `0`: 按键未被按下

**示例**:
```c
if (Matrix_Get_Key_Status(0, 0) == 1) {
    printf("Key at position [0,0] is pressed\r\n");
}
```

---

### 回调函数

```c
void Matrix_Key_Callback(uint8_t key_code, uint8_t pressed);
```
按键事件回调函数, 当检测到状态变化时自动调用。

**参数**:
- `key_code`: 按键编码 (0-8)
- `pressed`: 按键状态
  - `1`: 按键被按下
  - `0`: 按键被释放

**实现示例**:
```c
void Matrix_Key_Callback(uint8_t key_code, uint8_t pressed)
{
    if (pressed) {
        switch (key_code) {
            case 0: printf("Zero pressed\r\n"); break;
            case 1: printf("One pressed\r\n"); break;
            // ... 其他按键处理
        }
    } else {
        printf("Key %d released\r\n", key_code);
    }
}
```

## 配置参数

在`matrix_keyboard.h`中可配置:

```c
#define KEYBOARD_ROWS       3          // 键盘行数
#define KEYBOARD_COLS       3          // 键盘列数
#define DEBOUNCE_TIME       20         // 消抖时间(ms)
```

## 应用示例

详见`matrix_keyboard_advanced_example.c`文件, 包含以下示例:

1. **基础示例**: 简单的按键检测
2. **计算器**: 使用按键进行数学运算
3. **游戏控制器**: 方向键和动作按键
4. **密码系统**: 密码输入和验证
5. **长按检测**: 区分短按和长按

## 性能指标

| 项目 | 数值 |
|-----|------|
| 扫描周期 | 10ms |
| 按键响应延迟 | ~30ms |
| 消抖时间 | 20ms |
| 支持按键数 | 9个 (3x3) |
| 电流消耗 | 低 |

## 故障排查

### 问题1: 某个按键无反应

**可能原因**:
- GPIO连接错误
- 该行或列的驱动/检测故障
- 按键本身故障

**解决方法**:
1. 检查GPIO连接
2. 用万用表测量引脚电压
3. 检查硬件焊接质量

---

### 问题2: 按键误触发

**可能原因**:
- 消抖时间不足
- 电路干扰
- 按键接触不良

**解决方法**:
1. 增加`DEBOUNCE_TIME`值
2. 检查PCB布线
3. 添加EMI滤波电容

---

### 问题3: 扫描速度缓慢

**可能原因**:
- 主循环中有其他耗时操作
- 扫描周期设置过长

**解决方法**:
1. 优化主循环代码
2. 减少扫描周期(最小建议5ms)
3. 使用中断处理其他任务

## 进阶用法

### 支持更多按键

若需要支持4x4或其他规格的键盘, 修改:
1. `matrix_keyboard.h`中的`KEYBOARD_ROWS`和`KEYBOARD_COLS`
2. 添加对应的GPIO引脚定义
3. 更新`matrix_keyboard.c`中的行列引脚数组

### 集成UART输出

若要通过UART打印按键信息:

```c
void Matrix_Key_Callback(uint8_t key_code, uint8_t pressed)
{
    printf("KEY%d_%s\r\n", key_code, pressed ? "DOWN" : "UP");
}
```

## 许可证

此代码为示例代码, 可自由使用和修改。

## 作者说明

- 硬件: STM32F407
- 编译工具: STM32CubeIDE / ARM Compiler
- HAL库: STM32F4xx_HAL
- 开发日期: 2025年

---

**更多问题或建议, 欢迎反馈!**
