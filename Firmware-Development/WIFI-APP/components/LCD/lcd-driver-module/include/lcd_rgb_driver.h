#ifndef _LCD_RGB_DRIVER_H_
#define _LCD_RGB_DRIVER_H_

#include "esp_err.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_rgb.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_panel_ops.h"
#include "lcd_driver_pinouts.h"

esp_err_t lcd_rgb_driver_int();
esp_err_t lcd_backlight_setup();
esp_err_t lcd_backlight_on();
esp_err_t lcd_backlight_off();

#endif