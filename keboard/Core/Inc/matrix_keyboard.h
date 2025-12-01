/**
  ******************************************************************************
  * @file           : matrix_keyboard.h
  * @brief          : Matrix keyboard driver header file
  * 
  * Matrix Configuration: 3x3
  * Rows (Output): PA12, PA10, PA8
  * Columns (Input): PC6, PC7, PC8
  ******************************************************************************
  */

#ifndef __MATRIX_KEYBOARD_H
#define __MATRIX_KEYBOARD_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f4xx_hal.h"

/* Keyboard Configuration */
#define KEYBOARD_ROWS    3
#define KEYBOARD_COLS    3
#define TOTAL_KEYS       9

/* Row GPIO Configuration (Output/Drive pins) */
#define ROW_PORT         GPIOD
#define ROW_PIN_0        GPIO_PIN_14// Row 0
#define ROW_PIN_1        GPIO_PIN_12  // Row 1
#define ROW_PIN_2        GPIO_PIN_10   // Row 2

/* Column GPIO Configuration (Input/Sense pins) */
#define COL_PORT         GPIOC
#define COL_PIN_0        GPIO_PIN_6   // Column 0
#define COL_PIN_1        GPIO_PIN_7   // Column 1
#define COL_PIN_2        GPIO_PIN_8   // Column 2

/* Debounce delay in milliseconds */
#define DEBOUNCE_TIME    20

/* Function Prototypes */
void Matrix_Keyboard_Init(void);
void Matrix_Keyboard_Scan(void);
uint8_t Matrix_Get_Key_Status(uint8_t row, uint8_t col);
void Matrix_Key_Callback(uint8_t key_code, uint8_t pressed);

#ifdef __cplusplus
}
#endif

#endif /* __MATRIX_KEYBOARD_H */
