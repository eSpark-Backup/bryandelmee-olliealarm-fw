#include <stdio.h>
#include <cstring>
#include "esp_log.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "nvs_flash.h"
#include "freertos/FreeRTOS.h"
#include "BLEProv.hpp"

static const char* TAG = "Main";

static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data);

extern "C" void app_main(void)
{
    nvs_flash_init();
    esp_netif_init();   /*Initialize TCP/IP STACk*/
    esp_event_loop_create_default();

    esp_netif_create_default_wifi_sta();
    esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL);
    esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL);
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);


    /*BLEProv*/
    BLEProv ble_prov = BLEProv();
    ble_prov.setup();
    ble_prov.printServiceName();
    ble_prov.start();
    /************/

    while (1)
    {
        ESP_LOGI(TAG, "HELLO WORLD!");
        vTaskDelay(1000/portTICK_PERIOD_MS);
    }
    
}

static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data)
{
    static int retry = 0; 
    if(event_base == WIFI_EVENT){
        switch (event_id)
        {
            case WIFI_EVENT_STA_START:
                ESP_LOGI(TAG,"esp_wifi_connect");
                esp_wifi_connect();
                break;

            case WIFI_EVENT_STA_DISCONNECTED:
                ESP_LOGI(TAG, "Wi-Fi disconnected, Retry %d", retry);
                esp_wifi_connect();
                if(retry == 30)
                {  
                    retry=0;
                    ESP_LOGI(TAG,"Retries Completed, Failed to Connect Wi-Fi.");
                    break;
                }
                retry++;
                break;

            default:
                break;
        }
    }

    if(event_base == IP_EVENT){
        switch (event_id)
        {
        case IP_EVENT_STA_GOT_IP:
            ESP_LOGI(TAG, "Wi-Fi connected");
            retry=0;
            break;
        
        default:
            break;
        }
    }
}