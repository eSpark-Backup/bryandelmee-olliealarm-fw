#include <stdio.h>
#include "esp_check.h"
#include "esp_timer.h"
#include "esp_sleep.h"
#include "esp_netif.h"
#include "freertos/FreeRTOS.h"
static const char* TAG = "MAIN";

#include "Noozle.hpp"

Noozle* app;

extern "C" void app_main(void)
{
    app = new Noozle();
    app->setup();
    
    esp_reset_reason_t rst_reason = esp_reset_reason();
    switch (rst_reason)
    {
        case ESP_RST_DEEPSLEEP:
        {
            esp_err_t ret = app->connectWifi();
            if(ret != ESP_OK)
                app->setProvStatus(1); /*1 : Device is not Provisioned */
            break;
        }
        default:
        {
            app->checkProvStatusAndRunBleOrWifi();
            break;
        }
    }

    while (1)
    {
        ESP_LOGI(TAG, "HELLO WORLD!");
        vTaskDelay( (3*1000) /portTICK_PERIOD_MS);
        esp_sleep_enable_timer_wakeup(10 * 1000000);
        esp_deep_sleep_start();
    }
}