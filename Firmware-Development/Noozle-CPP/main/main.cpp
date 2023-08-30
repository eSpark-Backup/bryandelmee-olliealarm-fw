#include <stdio.h>
#include "esp_check.h"
#include "esp_timer.h"
#include "esp_sleep.h"
#include "esp_netif.h"
#include "freertos/FreeRTOS.h"
static const char* TAG = "MAIN";

#include "NVS.hpp"
#include "wifi.hpp"
#include "BLEProv.hpp"
// #include "Noozle.hpp"

WIFI* wifi;
BLEProv* ble;
NVS* nvs;
// Noozle* app;

#define NAME_SPACE "Noozle"
#define BOOT_KEY "Boot"
#define PROV_STATUS_KEY "Prov_Status"

esp_err_t bootCountUpdate();
esp_err_t checkProvisioninigStatus();

extern "C" void app_main(void)
{
    nvs = new NVS();
    nvs->init();
    nvs->setupKeyU32(NAME_SPACE, BOOT_KEY);
    nvs->setupKeyU8(NAME_SPACE, PROV_STATUS_KEY);
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    
    wifi = new WIFI();
    ble = new BLEProv();

    wifi->setup();
    ble->setup();

    esp_reset_reason_t rst_reason = esp_reset_reason();

    if(rst_reason == ESP_RST_DEEPSLEEP)
    { 
        wifi->run();
        if(ESP_FAIL == wifi->wait())
        {
            uint8_t prov_status = 0;
            nvs->writeKeyU8(NAME_SPACE, PROV_STATUS_KEY, prov_status);
        }
    }
    else
    {
        bootCountUpdate();
        if( ESP_FAIL == checkProvisioninigStatus() )
        {
            ble->run();
            ble->wait();
            uint8_t prov_status = 1;
            nvs->writeKeyU8(NAME_SPACE, PROV_STATUS_KEY, prov_status);
        }
        else
        {
            wifi->run();
            if(ESP_FAIL == wifi->wait())
            {
                uint8_t prov_status = 0;
                nvs->writeKeyU8(NAME_SPACE, PROV_STATUS_KEY, prov_status);
            }
        }
    }
            
    while (1)
    {
        wifi->printCredentials();
        vTaskDelay( (10*1000) /portTICK_PERIOD_MS);
        esp_sleep_enable_timer_wakeup(10 * 1000000);
        esp_deep_sleep_start();
    }
}

esp_err_t bootCountUpdate()
{
    uint32_t boot_count = 0;
    ESP_RETURN_ON_ERROR(nvs->readKeyU32(NAME_SPACE, BOOT_KEY, &boot_count), TAG, "bootCountUpdate: readKeyU32");
    boot_count++;
    ESP_RETURN_ON_ERROR(nvs->writeKeyU32(NAME_SPACE, BOOT_KEY, boot_count), TAG, "bootCountUpdate: writeKeyU32");
    ESP_LOGI(TAG, "boot count = %lu", boot_count);
    return ESP_OK;
}

esp_err_t checkProvisioninigStatus()
{
    uint8_t prov_status = 0;
    ESP_RETURN_ON_ERROR(nvs->readKeyU8(NAME_SPACE, PROV_STATUS_KEY, &prov_status), TAG, "checkProvisioninigStatus : readKeyU8");
    if(prov_status == 0)
        return ESP_FAIL;
    else
        return ESP_OK;
}