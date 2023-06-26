#ifndef _ST7701_H_
#define _ST7701_H_

#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"
#include "esp_check.h"

#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "st7701_pins.h"


#define LCD_CS_Clr()    gpio_set_level(GPIO_LCD_CS, 0)
#define LCD_CS_Set()    gpio_set_level(GPIO_LCD_CS, 1)
#define LCD_SCK_Clr()   gpio_set_level(GPIO_LCD_SCK, 0)
#define LCD_SCK_Set()   gpio_set_level(GPIO_LCD_SCK, 1)
#define LCD_SDA_Clr()   gpio_set_level(GPIO_LCD_SDA, 0)
#define LCD_SDA_Set()   gpio_set_level(GPIO_LCD_SDA, 1)

#define ST7701_CND2BKxSEL   0xFF  ///< Command2 BKx Selection
#define ST7701_BK0_PORCTRL  0xC1  ///< Porch control
#define ST7701_BK0_LNESET   0xC0  ///< Display Line setting
#define ST7701_BK0_INVSET   0xC2  ///< Inversion selection & Frame Rate Control
#define ST7701_MADCTL       0x36  ///< Display data access control


void st7701_reg_init(void);
void st7701_set_brightness_level(uint8_t brightness);

#endif

