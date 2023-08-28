#include <stdio.h>
#include "wifi.hpp"
#include "BLEProv.hpp"

/* Deep Sleep */
#include "esp_timer.h"
#include "esp_sleep.h"
/* ---------- */

/* Prerequites for WIFI*/
#include "esp_netif.h"
#include "nvs_flash.h"
/* ------ */

WIFI* wifi;
BLEProv* ble;

#include "freertos/FreeRTOS.h"

extern "C" void app_main(void)
{
    nvs_flash_init();
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    
    wifi = new WIFI();
    ble = new BLEProv();

    wifi->setup();
    ble->setup();

    esp_reset_reason_t rst_reason = esp_reset_reason();
    switch (rst_reason)
    {
        case ESP_RST_POWERON:
            ble->run();
            ble->wait();
            break;

        case ESP_RST_DEEPSLEEP:
            wifi->run();
            wifi->wait();
            break;

        default:
            break;
    }
    
    while (1)
    {
        wifi->printCredentials();
        vTaskDelay( (10*1000) /portTICK_PERIOD_MS);
        esp_sleep_enable_timer_wakeup(10 * 1000000);
        esp_deep_sleep_start();
    }
    
}
