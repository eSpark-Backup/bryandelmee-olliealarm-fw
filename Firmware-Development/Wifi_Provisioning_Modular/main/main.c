#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "lwip/err.h"
#include "lwip/sys.h"
#include "esp_event_loop.h"
#include <wifi_provisioning/manager.h>
#include <wifi_provisioning/scheme_ble.h>

static const char* TAG = "MainAPP";

#include "nvs_module.h"
extern nvs_handle_t nvsHandle;

void app_main()
{
    int var = 5;
    int temp;
    size_t size = sizeof(temp);
    esp_err_t ret;

    ret = nvs_init();
    ESP_LOGI(TAG,"%s", esp_err_to_name(ret));

    ret = nvs_open_namespace_rw("wificonf");
    ESP_LOGI(TAG,"%s", esp_err_to_name(ret));

    ret = nvs_write_key_value("config", &var, size);
    ESP_LOGI(TAG,"%s", esp_err_to_name(ret));

    nvs_close_handle();

    ret = nvs_read_key_value("config", &var);
    ESP_LOGI(TAG,"%s", esp_err_to_name(ret));


    while(1)
    {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}