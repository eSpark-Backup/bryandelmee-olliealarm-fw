#ifndef _LCD_APP_H_
#define _LCD_APP_H_

/* lcd-driver-moudle*/
#include "lcd_spi_driver.h"
#include "lcd_rgb_driver.h"
#include "lcd_lvgl.h"

/* ui-moudle */
#include "ui.h"
#include "ui_helpers.h"

#include "rtc_module.h"

esp_err_t lcd_setup();
void lcd_display();
esp_err_t lcd_create_task();
void lcd_app_run(void* parameter);

#endif