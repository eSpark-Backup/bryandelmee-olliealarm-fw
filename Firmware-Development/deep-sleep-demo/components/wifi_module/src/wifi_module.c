#include "wifi_module.h"

static const char* TAG = "WIFI-MODULE"; 

/* FreeRTOS event group to signal when we are connected*/
static EventGroupHandle_t wifi_event_group;
static const int WIFI_CONNECTED_BIT = BIT0;
static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data);

esp_err_t wifi_init()
{
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    return esp_wifi_init(&cfg);
}

esp_err_t wifi_create_event_group()
{
    wifi_event_group = xEventGroupCreate();
    if(wifi_event_group == NULL)
        return ESP_FAIL;

    return ESP_OK;
}

esp_err_t wifi_register_events()
{
    esp_err_t ret = ESP_OK;
    ret |= esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL);
    ret |= esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL);

    return ret;
}
/* Function To Test Wifi Module Individually */
esp_err_t wifi_sta_setup(wifi_config_t* wifi_config)
{
    esp_err_t ret = ESP_OK;
    nvs_init();
    esp_netif_init();       /* It is depricated, however code isn't working with tcpip_adapter_init()*/
    ret |= esp_event_loop_create_default(); /* Creates a default Event Loop to handle Wifi/IP Events */
    ret |= wifi_create_event_group();  /* Create a Wifi Event Group To Wait For the Wifi Connection Before Executing MainApp*/
    ret |= wifi_register_events();
    ret |= wifi_init();                /* Initialize WiFi Allocate resource for WiFi driver with default settings */
    ret |= esp_wifi_set_mode(WIFI_MODE_STA);   
    ret |= esp_wifi_set_config(WIFI_IF_STA, wifi_config); /*Sets the Wifi Configurations retrieved from NVS*/
    ret |= esp_wifi_start();
    ret |= wifi_connction_wait();   

    return ret;
}

esp_err_t wifi_save_credentials_nvs(const char * ssid, const char * password, const char* key) //wifi_save_credentials_nvs
{
    wifi_config_t wifi_config;
    strcpy((char*)wifi_config.sta.ssid, ssid);
    strcpy((char*)wifi_config.sta.password, password);
    wifi_config.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;
    wifi_config.sta.pmf_cfg.capable = true;
    wifi_config.sta.pmf_cfg.required = false;

    size_t size = sizeof(wifi_config);
    esp_err_t err = nvs_write_key_value(key, &wifi_config, size);
    return err;
}

esp_err_t wifi_get_credetianls_nvs(wifi_config_t* wifi_config, const char *key)
{
    return nvs_read_key_value(key, wifi_config);
}

esp_err_t wifi_connction_wait()
{
    // Wait for the Wi-Fi to be connected
    xEventGroupWaitBits(wifi_event_group, WIFI_CONNECTED_BIT, false, true, portMAX_DELAY);
    return ESP_OK;
}

void prov_reinit(void* args)
{
    prov_mgr_init();
    prov_mgr_setup();
    prov_mgr_credential_recv_wait();
    vTaskDelete(NULL);
}

static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data)
{
    static int retry = 0; 
    if(event_base == WIFI_EVENT){
        switch (event_id)
        {
            case WIFI_EVENT_STA_START:
                esp_wifi_connect();
                break;

            case WIFI_EVENT_STA_DISCONNECTED:
                ESP_LOGI(TAG, "Wi-Fi disconnected, Retry %d", retry);
                xEventGroupClearBits(wifi_event_group, WIFI_CONNECTED_BIT);
                esp_wifi_connect();
                if(retry == 5){  
                    xTaskCreate(prov_reinit,"prov_reinit_task", 2*2048, NULL, 10, NULL);
                    retry=0;
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
            xEventGroupSetBits(wifi_event_group, WIFI_CONNECTED_BIT);
            ESP_LOGI(TAG, "Wi-Fi connected");
            retry=0;
            break;
        
        default:
            break;
        }
    }
}