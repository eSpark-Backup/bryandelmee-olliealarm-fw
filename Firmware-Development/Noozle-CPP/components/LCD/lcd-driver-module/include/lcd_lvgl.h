#ifndef _LCD_LVGL_H_
#define _LCD_LVGL_H_

#include "lvgl.h"
#include "esp_err.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_rgb.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_panel_ops.h"
#include "esp_timer.h"
#include "lcd_driver_pinouts.h"

#define LVGL_TICK_PERIOD_MS 1

#ifdef __cplusplus
extern "C" {
#endif

esp_err_t lcd_lvgl_init();
esp_err_t lcd_lvgl_tick_init();

static void lcd_increase_lvgl_tick(void *arg);
static void lcd_lvgl_flush_cb(lv_disp_drv_t *drv, const lv_area_t *area, lv_color_t *color_map);

#ifdef __cplusplus
}
#endif

#endif