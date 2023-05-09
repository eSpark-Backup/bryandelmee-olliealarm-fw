#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "lwip/err.h"
#include "lwip/sys.h"
#include "esp_event_loop.h"

#define NVS_NAMESPACE "wifi_config"

static const char *TAG = "MainAPP";

/* FreeRTOS event group to signal when we are connected*/
static EventGroupHandle_t s_wifi_event_group;
static const int WIFI_CONNECTED_BIT = BIT0;

static void wifi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);

void wifi_init_sta()
{
    s_wifi_event_group = xEventGroupCreate();

    tcpip_adapter_init();

    ESP_ERROR_CHECK(esp_event_loop_create_default());
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL));

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    /*NVS PART*/

    /* nvs_open 
        - check if there is any namespace, if not, then create it.
        - if there is a namespace already present, it'll open it.
    */
    nvs_handle_t nvs_handle;
    ESP_ERROR_CHECK(nvs_open(NVS_NAMESPACE, NVS_READWRITE, &nvs_handle));

    wifi_config_t wifi_config;
    size_t size = sizeof(wifi_config);

    /*
        nvs_get_blob:
            - will return ESP_OK, if the wifi_config key is already present.
            - will return ESP_ERR_NVS_NOT_FOUND, if the wifi_config key is not present 
                (means startup condition, the user haven't input any wifi-credentials yet).
            
    */
    esp_err_t err = nvs_get_blob(nvs_handle, "wifi_config", &wifi_config, &size);
    if(err == ESP_ERR_NVS_NOT_FOUND){
        ESP_LOGI(TAG, "JUMP TO BLE TO SET WIFI CRED");
        memset(&wifi_config, 0, sizeof(wifi_config));
        strcpy((char*)wifi_config.sta.ssid, "HEHE");
        strcpy((char*)wifi_config.sta.password, "vision1999");
        wifi_config.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;
        wifi_config.sta.pmf_cfg.capable = true;
        wifi_config.sta.pmf_cfg.required = false;
        nvs_set_blob(nvs_handle, "wifi_config", &wifi_config, sizeof(wifi_config));
        nvs_commit(nvs_handle);
    }

    
    else if (err == ESP_OK) {
        ESP_LOGI(TAG, "Wifi Cred Found!");
        ESP_LOGI(TAG, "SSID = %s , PASS = %s", wifi_config.sta.ssid , wifi_config.sta.password);
    }

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    // Wait for the Wi-Fi to be connected
    xEventGroupWaitBits(s_wifi_event_group, WIFI_CONNECTED_BIT, false, true, portMAX_DELAY);
    nvs_close(nvs_handle);
}

void app_main()
{
    ESP_LOGI(TAG, "Initializing Wi-Fi");
    nvs_flash_init();
    wifi_init_sta();
    ESP_LOGI(TAG, "Wi-Fi connected");

    while (1)
    {
        ESP_LOGI(TAG, "HEHE : MAIN LOOP WORKING");
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
    
}

static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data)
{
    if(event_base == WIFI_EVENT){
        switch (event_id)
        {
            case WIFI_EVENT_STA_START:
                esp_wifi_connect();
                break;

            case WIFI_EVENT_STA_DISCONNECTED:
                ESP_LOGI(TAG, "Wi-Fi disconnected");
                xEventGroupClearBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
                esp_wifi_connect();
                break;

            default:
                break;
        }
    }

    else if(event_base == IP_EVENT){
        switch (event_id)
        {
        case IP_EVENT_STA_GOT_IP:
            xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
            ESP_LOGI(TAG, "Wi-Fi connected");
            break;
        
        default:
            break;
        }
    }
}
