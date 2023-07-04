#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
// #include "lvgl.h"

#include "esp_err.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_rgb.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_panel_ops.h"

#include "st7701.h"
#include "esp_timer.h"

#define EXAMPLE_LCD_PIXEL_CLOCK_HZ     (17.91036 * 1000 * 1000)
#define EXAMPLE_LCD_BK_LIGHT_ON_LEVEL  1
#define EXAMPLE_LCD_BK_LIGHT_OFF_LEVEL !EXAMPLE_LCD_BK_LIGHT_ON_LEVEL
// The pixel number in horizontal and vertical
#define EXAMPLE_LCD_H_RES              480
#define EXAMPLE_LCD_V_RES              480
#define EXAMPLE_LVGL_TICK_PERIOD_MS    1

void esp_lcd_setup();
void esp_lcd_backlight_init();
void esp_lcd_init();
void esp_lcd_draw_buf();
void *esp_lcd_memset(void *s, int c,  unsigned int len);


#ifdef __cplusplus
} /*extern "C"*/
#endif
