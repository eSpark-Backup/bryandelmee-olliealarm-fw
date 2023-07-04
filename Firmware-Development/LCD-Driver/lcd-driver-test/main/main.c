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
#include "st7701.h"
#include "lv_port_disp.h"

static const char *TAG = "MainAPP";

void setup_test_pin();

void app_main(void)
{
    setup_test_pin();
    vTaskDelay(1000/portTICK_PERIOD_MS);

    esp_lcd_setup();
    gpio_set_level(GPIO_NUM_41, 1);
    esp_lcd_draw_buf();
    gpio_set_level(GPIO_NUM_41, 0);
    esp_lcd_draw_buf();
    gpio_set_level(GPIO_NUM_41, 1);
    
    while (1)
    {
        vTaskDelay(1000/portTICK_PERIOD_MS);
    }
}

void setup_test_pin()
{
    gpio_config_t test_pin = {
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = 1ULL << GPIO_NUM_41
    };
    gpio_config(&test_pin);
    gpio_set_level(GPIO_NUM_41, 0);
}