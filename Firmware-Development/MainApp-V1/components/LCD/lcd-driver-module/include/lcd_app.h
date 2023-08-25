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

typedef struct{
    int64_t start_time;
    int64_t end_time;
    int alarm_name;
}lcd_config_alarm_t;

esp_err_t lcd_setup();
void lcd_display();
esp_err_t lcd_create_task(void* param);
void lcd_app_run(void* parameter);

#endif