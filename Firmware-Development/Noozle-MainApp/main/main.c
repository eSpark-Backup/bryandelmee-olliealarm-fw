#include <stdio.h>
#include <string.h>
#include "esp_system.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "wifi_prov_app.h"

static const char *TAG = "MainAPP";

void app_main()
{
    wifi_prov_app_setup();
    while (1)
    {
        ESP_LOGI(TAG, "HEHE : MAIN LOOP WORKING");
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
    
}