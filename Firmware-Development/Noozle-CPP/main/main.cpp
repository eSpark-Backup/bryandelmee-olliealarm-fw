#include <stdio.h>
#include "driver/gpio.h"
#include "driver/rtc_io.h"
#include "esp_check.h"
#include "esp_timer.h"
#include "esp_sleep.h"
#include "esp_netif.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

TaskHandle_t lcd_task_handle = NULL;


static const char* TAG = "MAIN of LCD-CPP Task";

#include "display.hpp"

void lcd_app_run( void* parameter );

extern "C" void app_main(void)
{
    // mydisplay.update(DAY);
    xTaskCreatePinnedToCore(lcd_app_run, "lcd_task", 1024*5, NULL, 10, &lcd_task_handle, 1);
    while (1)
    {
        // mydisplay.operate();
        ESP_LOGI(TAG, "hello world");
        vTaskDelay(pdMS_TO_TICKS(1));
    }
}

void lcd_app_run( void* parameter )
{
    DISPLAY mydisplay;
    mydisplay.setup();
    mydisplay.timer(10);
    while (1)
    {
        mydisplay.operate();
        vTaskDelay(pdMS_TO_TICKS(1));
    }
}