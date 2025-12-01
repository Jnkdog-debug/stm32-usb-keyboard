/**
  ******************************************************************************
  * @file           : usb_keyboard.c
  * @brief          : USB Keyboard HID driver implementation
  ******************************************************************************
  */

#include "usb_keyboard.h"
#include "usbd_hid.h"
#include <string.h>

/* Keyboard report buffer */
static USB_KeyboardReport_t keyboard_report = {0};
static USB_KeyboardReport_t keyboard_report_last = {0};

/* USB device handle (external, from usb_device.c) */
extern USBD_HandleTypeDef hUsbDeviceFS;

/* Key state tracking */
static uint8_t pressed_keys[6] = {0};
static uint8_t key_count = 0;

/**
  * @brief Initialize USB keyboard
  * @retval None
  */
void USB_Keyboard_Init(void)
{
    memset(&keyboard_report, 0, sizeof(keyboard_report));
    memset(&keyboard_report_last, 0, sizeof(keyboard_report_last));
    memset(pressed_keys, 0, sizeof(pressed_keys));
    key_count = 0;
}

/**
  * @brief Add a key to the keyboard report
  * @param key_code: HID keyboard code
  * @retval None
  */
void USB_Keyboard_PressKey(uint8_t key_code)
{
    if (key_code == 0) return;
    
    /* Check if key already in report */
    for (uint8_t i = 0; i < key_count; i++) {
        if (pressed_keys[i] == key_code) {
            return;  /* Already pressed */
        }
    }
    
    /* Add key if there's space */
    if (key_count < 6) {
        pressed_keys[key_count] = key_code;
        keyboard_report.keycode[key_count] = key_code;
        key_count++;
    }
}

/**
  * @brief Remove a key from the keyboard report
  * @param key_code: HID keyboard code
  * @retval None
  */
void USB_Keyboard_ReleaseKey(uint8_t key_code)
{
    if (key_code == 0) return;
    
    /* Find and remove the key */
    for (uint8_t i = 0; i < key_count; i++) {
        if (pressed_keys[i] == key_code) {
            /* Shift remaining keys */
            for (uint8_t j = i; j < key_count - 1; j++) {
                pressed_keys[j] = pressed_keys[j + 1];
                keyboard_report.keycode[j] = keyboard_report.keycode[j + 1];
            }
            pressed_keys[key_count - 1] = 0;
            keyboard_report.keycode[key_count - 1] = 0;
            key_count--;
            return;
        }
    }
}

/**
  * @brief Release all keys
  * @retval None
  */
void USB_Keyboard_ReleaseAll(void)
{
    memset(pressed_keys, 0, sizeof(pressed_keys));
    memset(keyboard_report.keycode, 0, sizeof(keyboard_report.keycode));
    key_count = 0;
    keyboard_report.modifier = 0;
}

/**
  * @brief Set modifier keys
  * @param modifier: Modifier bits (Shift, Ctrl, Alt, Win)
  * @retval None
  */
void USB_Keyboard_SetModifier(uint8_t modifier)
{
    keyboard_report.modifier = modifier;
}

/**
  * @brief Clear modifier keys
  * @retval None
  */
void USB_Keyboard_ClearModifier(void)
{
    keyboard_report.modifier = 0;
}

/**
  * @brief Send keyboard report to USB host
  * Only sends if report changed from last time
  * @retval None
  */
void USB_Keyboard_SendReport(void)
{
    /* Check if report changed */
    if (memcmp(&keyboard_report, &keyboard_report_last, sizeof(keyboard_report)) == 0) {
        return;  /* No change, don't send */
    }
    
    /* Send report */
    USBD_HID_SendReport(&hUsbDeviceFS, (uint8_t*)&keyboard_report, sizeof(keyboard_report));
    
    /* Update last report */
    memcpy(&keyboard_report_last, &keyboard_report, sizeof(keyboard_report));
}

/**
  * @brief Matrix keyboard to USB HID code mapping
  * Map 3x3 matrix keys to USB HID keycodes
  * 
  * Matrix layout:
  *   0 1 2  ->  Keys: 1 2 3
  *   3 4 5  ->  Keys: 4 5 6
  *   6 7 8  ->  Keys: 7 8 9
  */
static const uint8_t matrix_to_usb_hid[9] = {
    KEY_1,      /* Matrix key 0 -> USB 1 */
    KEY_2,      /* Matrix key 1 -> USB 2 */
    KEY_3,      /* Matrix key 2 -> USB 3 */
    KEY_4,      /* Matrix key 3 -> USB 4 */
    KEY_5,      /* Matrix key 4 -> USB 5 */
    KEY_6,      /* Matrix key 5 -> USB 6 */
    KEY_7,      /* Matrix key 6 -> USB 7 */
    KEY_8,      /* Matrix key 7 -> USB 8 */
    KEY_9,      /* Matrix key 8 -> USB 9 */
};

/**
  * @brief Convert matrix keyboard code to USB HID code
  * This is called from Matrix_Key_Callback (override in main.c)
  * @param matrix_key: Matrix key code (0-8)
  * @param pressed: 1 if pressed, 0 if released
  * @retval None
  */
void USB_Keyboard_HandleMatrixKey(uint8_t matrix_key, uint8_t pressed)
{
    if (matrix_key >= 9) return;
    
    uint8_t usb_key = matrix_to_usb_hid[matrix_key];
    
    if (pressed) {
        USB_Keyboard_PressKey(usb_key);
    } else {
        USB_Keyboard_ReleaseKey(usb_key);
    }
    
    /* Send report */
    USB_Keyboard_SendReport();
}

/**
  * @brief Get USB keyboard report content
  * @param report: Pointer to buffer to receive report
  * @retval Length of report (8 bytes)
  */
uint8_t USB_Keyboard_GetReport(uint8_t *report)
{
    memcpy(report, &keyboard_report, sizeof(keyboard_report));
    return sizeof(keyboard_report);
}
