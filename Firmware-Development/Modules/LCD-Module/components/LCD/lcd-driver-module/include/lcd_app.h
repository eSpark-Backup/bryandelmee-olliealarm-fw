#ifndef LCD_APP_H
#define LCD_APP_H

#include "lcd_spi_driver.h"
#include "lcd_rgb_driver.h"
#include "lcd_lvgl.h"

#include "ui.h"
#include "ui_helpers.h"

#include "esp_timer.h"
#include "esp_sleep.h"

esp_err_t lcd_setup();
void lcd_display();
esp_err_t lcd_create_task();
void lcd_app_run(void* parameter);

#endif