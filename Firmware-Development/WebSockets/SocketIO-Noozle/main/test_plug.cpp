#include <stdio.h>
#include "driver/ledc.h"
#include "esp_log.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "nvs_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "wifi.hpp"
#include "plug.hpp"


#define TAG "Plug Test"


extern "C" void app_main(void)
{
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    WIFI wifi = WIFI();
    wifi.setup();
    wifi.setConfig("eSpark Wifi", "47356880");
    wifi.run();
    wifi.wait();
    
    plug myplug = plug();
    myplug.set_handler();
    myplug.configure();

    /*setup gpio for brightnes*/
    ledc_timer_config_t led_config = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .duty_resolution =  LEDC_TIMER_10_BIT,
        .timer_num = LEDC_TIMER_0,
        .freq_hz = 20000

    };
    ledc_timer_config(&led_config);
    ledc_channel_config_t led_chan_config = {
        .gpio_num = 38,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = LEDC_CHANNEL_0

    };
    ledc_channel_config(&led_chan_config);
    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, 512);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);

    while(true){
        vTaskDelay(1000/portTICK_PERIOD_MS);
    }

}
