#ifndef _LCD_DRIVER_PINOUTS_H_
#define _LCD_DRIVER_PINOUTS_H_

#include "driver/gpio.h"

#define LCD_WIDTH       (480)
#define LCD_HEIGHT      (480)

/* LCD Driver ST7701 : SPI Interface*/
#define LCD_SPI_HOST    SPI2_HOST
#define LCD_GPIO_RST    (GPIO_NUM_NC)
#define LCD_GPIO_CS     (GPIO_NUM_39)
#define LCD_GPIO_SDA    (GPIO_NUM_47)
#define LCD_GPIO_SCK    (GPIO_NUM_48)

/* LCD Driver ST7701 : RGB Interface*/
#define LCD_PIXEL_CLOCK_HZ (20 * 1000 * 1000)
#define LCD_GPIO_PCLK   (GPIO_NUM_21)
#define LCD_GPIO_VSYNC  (GPIO_NUM_17)
#define LCD_GPIO_HSYNC  (GPIO_NUM_16)
#define LCD_GPIO_DE     (GPIO_NUM_18)
#define LCD_GPIO_DISP   (GPIO_NUM_NC)
#define LCD_GPIO_BL     (GPIO_NUM_38)

#define LCD_RGB_HBP     24
#define LCD_RGB_HFP     26
#define LCD_RGB_HPW     4
#define LCD_RGB_VBP     15
#define LCD_RGB_VFP     60
#define LCD_RGB_VPW     4

/* RGB Interface : Data Pins 0 - 15 [BGR] */
#define LCD_GPIO_B0    (GPIO_NUM_15)
#define LCD_GPIO_B1    (GPIO_NUM_14)
#define LCD_GPIO_B2    (GPIO_NUM_13)
#define LCD_GPIO_B3    (GPIO_NUM_12)
#define LCD_GPIO_B4    (GPIO_NUM_11)

#define LCD_GPIO_G0    (GPIO_NUM_10)
#define LCD_GPIO_G1    (GPIO_NUM_9)
#define LCD_GPIO_G2    (GPIO_NUM_8)
#define LCD_GPIO_G3    (GPIO_NUM_7)
#define LCD_GPIO_G4    (GPIO_NUM_6)
#define LCD_GPIO_G5    (GPIO_NUM_5)

#define LCD_GPIO_R0    (GPIO_NUM_4)
#define LCD_GPIO_R1    (GPIO_NUM_3)
#define LCD_GPIO_R2    (GPIO_NUM_2)
#define LCD_GPIO_R3    (GPIO_NUM_1)
#define LCD_GPIO_R4    (GPIO_NUM_0)

#endif