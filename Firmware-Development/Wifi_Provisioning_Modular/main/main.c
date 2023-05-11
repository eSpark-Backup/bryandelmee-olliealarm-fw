#include <stdio.h>
#include <string.h>
#include "esp_system.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "wifi_module.h"
#include "nvs_module.h"
#include "prov_module.h"

static const char *TAG = "MainAPP";

void app_main()
{
    /* Wifi prov manager relies on other components of IDF, namely NVS, TCP/IP, Event Loop and Wi-Fi*/
    nvs_init();
    esp_netif_init(); // tcpip_adapter_init(); 
    esp_event_loop_create_default(); /* Creates a default Event Loop to handle Wifi/IP Events */
    wifi_create_event_group();
    prov_create_event_group(); 
    wifi_register_events();
    prov_register_events();

    /* Initialize Wi-Fi including netif with default config */
    esp_netif_create_default_wifi_sta();
    wifi_init();

    prov_mgr_init();
    prov_mgr_setup();
    ESP_LOGE(TAG,"Wait for the Wi-Fi Credentials to be recieved");
    prov_mgr_credential_recv_wait();
    ESP_LOGE(TAG,"Wi-Fi Credentials Recieved (xEventGroupWaitBits)");

    ESP_ERROR_CHECK(esp_wifi_start());
    wifi_connction_wait();

    while (1)
    {
        ESP_LOGI(TAG,"Main Loop");
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
    
}