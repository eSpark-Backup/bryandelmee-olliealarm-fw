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

#include <wifi_provisioning/manager.h>
#include <wifi_provisioning/scheme_ble.h>

#define PROV_QR_VERSION         "v1"
#define PROV_TRANSPORT_SOFTAP   "softap"
#define PROV_TRANSPORT_BLE      "ble"
#define NVS_NAMESPACE "wifi_config"
nvs_handle_t nvsHandle;
wifi_config_t wifi_config;

static const char *TAG = "MainAPP";

/* FreeRTOS event group to signal when we are connected*/
static EventGroupHandle_t wifi_event_group;
static const int WIFI_CONNECTED_BIT = BIT0;
static const int WIFI_CRED_RECV_BIT = BIT1;

esp_err_t wifi_set_credentials(const char * ssid, const char * password );
static void wifi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);

void wifi_init_sta()
{
    wifi_event_group = xEventGroupCreate();

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
    ESP_ERROR_CHECK(nvs_open(NVS_NAMESPACE, NVS_READWRITE, &nvsHandle));

    // wifi_config_t wifi_config;
    size_t size = sizeof(wifi_config);

    /*
        nvs_get_blob:
            - will return ESP_OK, if the wifi_config key is already present.
            - will return ESP_ERR_NVS_NOT_FOUND, if the wifi_config key is not present 
                (means startup condition, the user haven't input any wifi-credentials yet).
            
    */
    esp_err_t err = nvs_get_blob(nvsHandle, "wifi_config", &wifi_config, &size);
    if(err == ESP_ERR_NVS_NOT_FOUND){
        ESP_LOGI(TAG, "BLE SERVICE START");

        // Wait for the Wi-Fi Credentials to be recieved
        xEventGroupWaitBits(wifi_event_group, WIFI_CRED_RECV_BIT, true, true, portMAX_DELAY);        
    }

    
    else if (err == ESP_OK) {
        ESP_LOGI(TAG, "Wifi Cred Found!");
        ESP_LOGI(TAG, "SSID = %s , PASS = %s", wifi_config.sta.ssid , wifi_config.sta.password);
    }

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    // Wait for the Wi-Fi to be connected
    xEventGroupWaitBits(wifi_event_group, WIFI_CONNECTED_BIT, false, true, portMAX_DELAY);
    nvs_close(nvsHandle);
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

esp_err_t wifi_set_credentials(const char * ssid, const char * password )
{
    strcpy((char*)wifi_config.sta.ssid, ssid);
    strcpy((char*)wifi_config.sta.password, password);
    wifi_config.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;
    wifi_config.sta.pmf_cfg.capable = true;
    wifi_config.sta.pmf_cfg.required = false;

    esp_err_t err = nvs_set_blob(nvsHandle, "wifi_config", &wifi_config, sizeof(wifi_config));
    err = nvs_commit(nvsHandle);

    return err;
}

static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data)
{
    static int retries;
    if (event_base == WIFI_PROV_EVENT) {
        switch (event_id) {
            case WIFI_PROV_START:
                ESP_LOGI(TAG, "Provisioning started");
                break;

            case WIFI_PROV_CRED_RECV: {
                wifi_sta_config_t *wifi_sta_cfg = (wifi_sta_config_t *)event_data;
                wifi_set_credentials((const char *)wifi_sta_cfg->ssid, (const char *)wifi_sta_cfg->password);
                ESP_LOGI(TAG, "Received Wi-Fi credentials"
                         "\n\tSSID     : %s\n\tPassword : %s",
                         (const char *) wifi_sta_cfg->ssid,
                         (const char *) wifi_sta_cfg->password);
                xEventGroupSetBits(wifi_event_group, WIFI_CRED_RECV_BIT);
                break;
            }
            case WIFI_PROV_CRED_FAIL: {
                wifi_prov_sta_fail_reason_t *reason = (wifi_prov_sta_fail_reason_t *)event_data;
                ESP_LOGE(TAG, "Provisioning failed!\n\tReason : %s"
                         "\n\tPlease reset to factory and retry provisioning",
                         (*reason == WIFI_PROV_STA_AUTH_ERROR) ?
                         "Wi-Fi station authentication failed" : "Wi-Fi access-point not found");
                retries++;
                if (retries >= CONFIG_EXAMPLE_PROV_MGR_MAX_RETRY_CNT) {
                    ESP_LOGI(TAG, "Failed to connect with provisioned AP, reseting provisioned credentials");
                    wifi_prov_mgr_reset_sm_state_on_failure();
                    retries = 0;
                }
                break;
            }
            case WIFI_PROV_CRED_SUCCESS:
                ESP_LOGI(TAG, "Provisioning successful");
                retries = 0;
                break;
            
            case WIFI_PROV_END:
                /* De-initialize manager once provisioning is finished */
                wifi_prov_mgr_deinit();
                break;
                
            default:
                break;
        }
        
    }
    if(event_base == WIFI_EVENT){
        switch (event_id)
        {
            case WIFI_EVENT_STA_START:
                esp_wifi_connect();
                break;

            case WIFI_EVENT_STA_DISCONNECTED:
                ESP_LOGI(TAG, "Wi-Fi disconnected");
                xEventGroupClearBits(wifi_event_group, WIFI_CONNECTED_BIT);
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
            xEventGroupSetBits(wifi_event_group, WIFI_CONNECTED_BIT);
            ESP_LOGI(TAG, "Wi-Fi connected");
            break;
        
        default:
            break;
        }
    }
}


/*
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
                xEventGroupClearBits(wifi_event_group, WIFI_CONNECTED_BIT);
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
            xEventGroupSetBits(wifi_event_group, WIFI_CONNECTED_BIT);
            ESP_LOGI(TAG, "Wi-Fi connected");
            break;
        
        default:
            break;
        }
    }
}
*/
