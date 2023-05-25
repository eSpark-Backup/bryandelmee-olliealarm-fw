#pragma once

#include "driver/gpio.h"

/**************** Please update the following configuration according to your LCD spec ****************/
#define EXAMPLE_PIN_NUM_BK_LIGHT       GPIO_NUM_38
#define EXAMPLE_PIN_NUM_HSYNC          GPIO_NUM_16
#define EXAMPLE_PIN_NUM_VSYNC          GPIO_NUM_17
#define EXAMPLE_PIN_NUM_DE             GPIO_NUM_18
#define EXAMPLE_PIN_NUM_PCLK           GPIO_NUM_21
#define EXAMPLE_PIN_NUM_DATA0          GPIO_NUM_15 // B0
#define EXAMPLE_PIN_NUM_DATA1          GPIO_NUM_14 // B1
#define EXAMPLE_PIN_NUM_DATA2          GPIO_NUM_13 // B2
#define EXAMPLE_PIN_NUM_DATA3          GPIO_NUM_12 // B3
#define EXAMPLE_PIN_NUM_DATA4          GPIO_NUM_11 // B4
#define EXAMPLE_PIN_NUM_DATA5          GPIO_NUM_10 // G0
#define EXAMPLE_PIN_NUM_DATA6          GPIO_NUM_9 // G1
#define EXAMPLE_PIN_NUM_DATA7          GPIO_NUM_8 // G2
#define EXAMPLE_PIN_NUM_DATA8          GPIO_NUM_7 // G3
#define EXAMPLE_PIN_NUM_DATA9          GPIO_NUM_6 // G4
#define EXAMPLE_PIN_NUM_DATA10         GPIO_NUM_5 // G5
#define EXAMPLE_PIN_NUM_DATA11         GPIO_NUM_4  // R0
#define EXAMPLE_PIN_NUM_DATA12         GPIO_NUM_3  // R1
#define EXAMPLE_PIN_NUM_DATA13         GPIO_NUM_2 // R2
#define EXAMPLE_PIN_NUM_DATA14         GPIO_NUM_1 // R3
#define EXAMPLE_PIN_NUM_DATA15         GPIO_NUM_0 // R4
#define EXAMPLE_PIN_NUM_DISP_EN        -1

/**
 * @brief ESP32-S3 LCD GPIO defination and config
 * 
 */
#define FUNC_LCD_EN     (1)
#define LCD_WIDTH       (480)
#define LCD_HEIGHT      (480)

#define GPIO_LCD_BL     (GPIO_NUM_38)
#define GPIO_LCD_RST    (GPIO_NUM_NC)
#define GPIO_LCD_CS     (GPIO_NUM_39)
#define GPIO_LCD_SDA    (GPIO_NUM_47)
#define GPIO_LCD_SCK    (GPIO_NUM_48)

#define GPIO_LCD_DE     (GPIO_NUM_18)
#define GPIO_LCD_VSYNC  (GPIO_NUM_17)
#define GPIO_LCD_HSYNC  (GPIO_NUM_16)
#define GPIO_LCD_PCLK   (GPIO_NUM_21)

#define GPIO_LCD_R0    (GPIO_NUM_4)
#define GPIO_LCD_R1    (GPIO_NUM_3)
#define GPIO_LCD_R2    (GPIO_NUM_2)
#define GPIO_LCD_R3    (GPIO_NUM_1)
#define GPIO_LCD_R4    (GPIO_NUM_0)

#define GPIO_LCD_G0    (GPIO_NUM_10)
#define GPIO_LCD_G1    (GPIO_NUM_9)
#define GPIO_LCD_G2    (GPIO_NUM_8)
#define GPIO_LCD_G3    (GPIO_NUM_7)
#define GPIO_LCD_G4    (GPIO_NUM_6)
#define GPIO_LCD_G5    (GPIO_NUM_5)

#define GPIO_LCD_B0    (GPIO_NUM_15)
#define GPIO_LCD_B1    (GPIO_NUM_14)
#define GPIO_LCD_B2    (GPIO_NUM_13)
#define GPIO_LCD_B3    (GPIO_NUM_12)
#define GPIO_LCD_B4    (GPIO_NUM_11)