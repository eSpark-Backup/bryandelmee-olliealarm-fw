#include <stdio.h>
#include <string.h>
#include "esp_system.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "wifi_module.h"
#include "nvs_module.h"
static const char *TAG = "MainAPP";

void app_main()
{
    nvs_init();
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = "eSpark Wifi",
            .password = "47356880",
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
            .pmf_cfg.capable = true,
            .pmf_cfg.required = false,
        },
    };
    wifi_sta_setup(&wifi_config);
    wifi_connction_wait();   

    while (1)
    {
        ESP_LOGI(TAG, "HEHE : MAIN LOOP WORKING");
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
    
}