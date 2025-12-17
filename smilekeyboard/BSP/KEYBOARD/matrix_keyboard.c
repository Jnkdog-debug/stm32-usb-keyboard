/**
  ******************************************************************************
  * @file           : matrix_keyboard.c
  * @brief          : Matrix keyboard driver implementation
  * 
  * Scanning method: Drive rows one by one, read columns
  * Matrix layout:
  *        COL0(C6)  COL1(C7)  COL2(C8)
  * ROW0(A12)  0       1        2
  * ROW1(A10)  3       4        5
  * ROW2(A8)   6       7        8
  ******************************************************************************
  */

#include "matrix_keyboard.h"

/* Global variables for keyboard state */
static uint8_t key_state[KEYBOARD_ROWS][KEYBOARD_COLS] = {0};
static uint8_t key_state_last[KEYBOARD_ROWS][KEYBOARD_COLS] = {0};
static uint32_t debounce_timer[KEYBOARD_ROWS][KEYBOARD_COLS] = {0};

/* Row and Column pin definitions */
static const uint16_t row_pins[KEYBOARD_ROWS] = {ROW_PIN_0, ROW_PIN_1, ROW_PIN_2};
static const uint16_t col_pins[KEYBOARD_COLS] = {COL_PIN_0, COL_PIN_1, COL_PIN_2};

/* Key mapping table for easier reference */
static const uint8_t key_map[KEYBOARD_ROWS][KEYBOARD_COLS] = {
    {0, 1, 2},
    {3, 4, 5},
    {6, 7, 8}
};

/**
  * @brief Initialize matrix keyboard
  * @retval None
  */
void Matrix_Keyboard_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    /* Configure rows as output (push-pull, low) */
    GPIO_InitStruct.Pin = ROW_PIN_0 | ROW_PIN_1 | ROW_PIN_2;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(ROW_PORT, &GPIO_InitStruct);
    
    /* Configure columns as input (with pull-up for stable detection) */
    GPIO_InitStruct.Pin = COL_PIN_0 | COL_PIN_1 | COL_PIN_2;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(COL_PORT, &GPIO_InitStruct);
    
    /* Initialize all rows to inactive state (HIGH) */
    HAL_GPIO_WritePin(ROW_PORT, ROW_PIN_0, GPIO_PIN_SET);
    HAL_GPIO_WritePin(ROW_PORT, ROW_PIN_1, GPIO_PIN_SET);
    HAL_GPIO_WritePin(ROW_PORT, ROW_PIN_2, GPIO_PIN_SET);
    
    /* Clear state arrays */
    for (uint8_t i = 0; i < KEYBOARD_ROWS; i++) {
        for (uint8_t j = 0; j < KEYBOARD_COLS; j++) {
            key_state[i][j] = 0;
            key_state_last[i][j] = 0;
            debounce_timer[i][j] = 0;
        }
    }
}

/**
  * @brief Scan the matrix keyboard
  * This function should be called periodically (e.g., every 5-10ms)
  * @retval None
  */
void Matrix_Keyboard_Scan(void)
{
    uint8_t col;
    GPIO_PinState pin_state;
    uint32_t current_time = HAL_GetTick();
    
    /* Scan each row */
    for (uint8_t row = 0; row < KEYBOARD_ROWS; row++) {
        /* Set current row to LOW (activate it), others to HIGH */
        for (uint8_t i = 0; i < KEYBOARD_ROWS; i++) {
            if (i == row) {
                HAL_GPIO_WritePin(ROW_PORT, row_pins[i], GPIO_PIN_RESET);  // LOW = active
            } else {
                HAL_GPIO_WritePin(ROW_PORT, row_pins[i], GPIO_PIN_SET);    // HIGH = inactive
            }
        }
        
        /* Small delay for signal stabilization */
        for (volatile uint32_t i = 0; i < 100; i++);
        
        /* Read each column */
        for (col = 0; col < KEYBOARD_COLS; col++) {
            pin_state = HAL_GPIO_ReadPin(COL_PORT, col_pins[col]);
            
            /* With pull-up: GPIO_PIN_RESET (0) = pressed, GPIO_PIN_SET (1) = not pressed */
            uint8_t current_state = (pin_state == GPIO_PIN_RESET) ? 1 : 0;
            
            /* Debounce logic */
            if (current_state != key_state[row][col]) {
                /* State changed, start debounce timer */
                if (debounce_timer[row][col] == 0) {
                    debounce_timer[row][col] = current_time;
                }
                
                /* Check if debounce time has passed */
                if ((current_time - debounce_timer[row][col]) >= DEBOUNCE_TIME) {
                    /* State is stable, update key state */
                    key_state_last[row][col] = key_state[row][col];
                    key_state[row][col] = current_state;
                    debounce_timer[row][col] = 0;
                    
                    /* Call callback if state changed */
                    if (key_state[row][col] != key_state_last[row][col]) {
                        uint8_t key_code = key_map[row][col];
                        Matrix_Key_Callback(key_code, key_state[row][col]);
                    }
                }
            } else {
                /* State stable, reset debounce timer */
                debounce_timer[row][col] = 0;
            }
        }
    }
    
    /* Set all rows back to HIGH when done */
    for (uint8_t i = 0; i < KEYBOARD_ROWS; i++) {
        HAL_GPIO_WritePin(ROW_PORT, row_pins[i], GPIO_PIN_SET);
    }
}

/**
  * @brief Get the status of a specific key
  * @param row: Row index (0-2)
  * @param col: Column index (0-2)
  * @retval Key status: 1 = pressed, 0 = not pressed
  */
uint8_t Matrix_Get_Key_Status(uint8_t row, uint8_t col)
{
    if (row < KEYBOARD_ROWS && col < KEYBOARD_COLS) {
        return key_state[row][col];
    }
    return 0;
}

/**
  * @brief Callback function for key press/release events
  * This function should be overridden by user application
  * @param key_code: Key code (0-8)
  * @param pressed: 1 = key pressed, 0 = key released
  * @retval None
  */
__weak void Matrix_Key_Callback(uint8_t key_code, uint8_t pressed)
{
    /* This is a weak function, user can override it in application code */
    /* Example implementation:
     * if (pressed) {
     *     printf("Key %d pressed\r\n", key_code);
     * } else {
     *     printf("Key %d released\r\n", key_code);
     * }
     */
}
