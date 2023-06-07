#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include "esp_log.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

/* WiFi / BLE-Provisioning */
#include "wifi_prov_app.h"

/* Cloud-API */
#include "cloud_api_module.h"

static const char *TAG = "MainAPP";

void app_main()
{
    alarmtime_t alarmTimeData[4];   /* Variable to hold Alarm Data's: [ (Almost Sleep)-(To Sleep) & (Almost Getting Up) - (To Get Up)] */
    uint64_t current_time = 0;

    wifi_prov_app_setup();

    cloud_api_create_event_group();
    xTaskCreate(cloud_api_current_time_task, "cloud_api_current_time_task", 8192, (void*)&current_time, 6, NULL);
    cloud_api_wait_current_time_event();
    
    while (1)
        {
        ESP_LOGI(TAG, "%lld", current_time);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        }
    
}

