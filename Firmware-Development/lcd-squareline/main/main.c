#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <inttypes.h>
#include "sdkconfig.h"
#include "esp_timer.h"
#include "esp_sleep.h"
#include "esp_log.h"
#include "esp_err.h"
#include "soc/soc_caps.h"
#include "driver/rtc_io.h"
#include "soc/rtc.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "sdkconfig.h"

/*LCD*/
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_rgb.h"
#include "driver/gpio.h"
#include "lvgl.h"
#include "lv_examples.h"
#include "st7701.h"
#include "lv_port_disp.h"

#include "lv_demos.h"
#include "ui.h"

static const char *TAG = "MainAPP";
int64_t time_since_boot = 0;

void lcd_init(void* para)
{
    lv_port_disp_init();
    vTaskDelete(NULL);
}

void app_main(void)
{
    
    lv_port_disp_init();
    // lv_port_tick_init();
    // xTaskCreatePinnedToCore(lcd_init, "lcd_init_task", 2048*4, NULL, 10, NULL, 1);
    lv_port_tick_init();
    ui_init();
    time_since_boot = esp_timer_get_time();
    while (1)
    {
        lv_timer_handler();
        vTaskDelay(10/portTICK_PERIOD_MS);
    }
}