#include <stdio.h>
#include <string.h>
#include <stdlib.h>
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
    wifi_prov_app_setup();
    xTaskCreate(&cloud_api_task, "cloud_api_task", 8192, (void*)alarmTimeData, 6, NULL);
    while (1)
        {
        ESP_LOGI(TAG, "HEHE : MAIN LOOP WORKING");
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
    
}

