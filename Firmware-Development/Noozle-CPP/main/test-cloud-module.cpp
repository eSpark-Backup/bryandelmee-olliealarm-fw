#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/param.h>
#include "driver/gpio.h"
#include "driver/rtc_io.h"
#include "esp_check.h"
#include "esp_timer.h"
#include "esp_sleep.h"
#include "esp_netif.h"
#include "esp_http_client.h"
#include "esp_event.h"
#include "cloud_api_module.h"
#include "freertos/FreeRTOS.h"

static const char* TAG = "MAIN";

#include "Noozle.hpp"
Noozle* app;


extern "C" void app_main(void)
{
    app = new Noozle();
    app->setup();
    app->connectWifi();

    noozle_cloud_service(NOOZLE_REPEATED_ALARM_API);


    while (1)
    {
        ESP_LOGI(TAG, "HELLO WORLD!");
        vTaskDelay(1000/portTICK_PERIOD_MS);
    }
}