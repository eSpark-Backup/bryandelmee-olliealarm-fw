#include "prov_module.h"

static const char* TAG = "PROV-MODULE";
static EventGroupHandle_t ble_event_group;
static const int WIFI_CRED_RECV_BIT = BIT1;
static void prov_device_set_service_name(char *service_name, size_t max);
static void ble_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);

esp_err_t prov_mgr_init(){
        /* Configuration for the provisioning manager */
        wifi_prov_mgr_config_t config = {
            .scheme = wifi_prov_scheme_ble,
            .scheme_event_handler = WIFI_PROV_SCHEME_BLE_EVENT_HANDLER_FREE_BTDM
        };
    /* Initialize provisioning manager with the configuration parameters set above */
    return wifi_prov_mgr_init(config);
}

esp_err_t prov_create_event_group()
{
    ble_event_group = xEventGroupCreate();
    if(ble_event_group == NULL)
        return ESP_FAIL;
    
    return ESP_OK;
} 
esp_err_t prov_register_events()
{
    return esp_event_handler_register(WIFI_PROV_EVENT, ESP_EVENT_ANY_ID, &ble_event_handler, NULL);
}

esp_err_t prov_mgr_setup()
{
    prov_data_t prov_data;
    prov_mgr_set_data(&prov_data);

    wifi_prov_scheme_ble_set_service_uuid(prov_data.custom_service_uuid);
    wifi_prov_mgr_endpoint_create("custom-data");
    /* Start provisioning service */
    ESP_ERROR_CHECK(wifi_prov_mgr_start_provisioning(prov_data.security, prov_data.pop, prov_data.service_name, prov_data.service_key));
    wifi_prov_mgr_endpoint_register("custom-data", custom_prov_data_handler, NULL);

    return ESP_OK;
}

esp_err_t prov_mgr_set_data(prov_data_t* prov_data)
{
    prov_device_set_service_name(prov_data->service_name, sizeof(prov_data->service_name));
    prov_data->security = WIFI_PROV_SECURITY_1;
    prov_data->pop = "abcd1234";
    char uuid[16] = {
        /* LSB <---------------------------------------
         * ---------------------------------------> MSB */
        0xb4, 0xdf, 0x5a, 0x1c, 0x3f, 0x6b, 0xf4, 0xbf,
        0xea, 0x4a, 0x82, 0x03, 0x04, 0x90, 0x1a, 0x02,
    };
    memcpy(prov_data->custom_service_uuid, uuid, sizeof(uuid));
    prov_data->service_key = NULL;

    return ESP_OK;
}

esp_err_t prov_mgr_credential_recv_wait()
{
    xEventGroupWaitBits(ble_event_group, WIFI_CRED_RECV_BIT, false, true, portMAX_DELAY); 
    return ESP_OK;  
}
static void prov_device_set_service_name(char *service_name, size_t max)
{
    uint8_t eth_mac[6];
    const char *ssid_prefix = "PROV_";
    esp_wifi_get_mac(WIFI_IF_STA, eth_mac);
    snprintf(service_name, max, "%s%02X%02X%02X",
             ssid_prefix, eth_mac[3], eth_mac[4], eth_mac[5]);
}

esp_err_t custom_prov_data_handler(uint32_t session_id, const uint8_t *inbuf, ssize_t inlen,
                                          uint8_t **outbuf, ssize_t *outlen, void *priv_data)
{
    if (inbuf) {
        ESP_LOGI(TAG, "Received data: %.*s", inlen, (char *)inbuf);
    }
    char response[] = "SUCCESS";
    *outbuf = (uint8_t *)strdup(response);
    if (*outbuf == NULL) {
        ESP_LOGE(TAG, "System out of memory");
        return ESP_ERR_NO_MEM;
    }
    *outlen = strlen(response) + 1; /* +1 for NULL terminating byte */

    return ESP_OK;
}


static void ble_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
    static int retries = 0;
    if (event_base == WIFI_PROV_EVENT) {

        switch (event_id) {

            case WIFI_PROV_START:
                ESP_LOGI(TAG, "Provisioning started");
                break;

            case WIFI_PROV_CRED_RECV: {
                wifi_sta_config_t *wifi_sta_cfg = (wifi_sta_config_t *)event_data;
                wifi_save_credentials_nvs((const char *)wifi_sta_cfg->ssid, (const char *)wifi_sta_cfg->password, WIFI_KEY);
                ESP_LOGI(TAG, "Received Wi-Fi credentials"
                         "\n\tSSID     : %s\n\tPassword : %s",
                         (const char *) wifi_sta_cfg->ssid,
                         (const char *) wifi_sta_cfg->password);
                break;
            }

            case WIFI_PROV_CRED_FAIL: {
                wifi_prov_sta_fail_reason_t *reason = (wifi_prov_sta_fail_reason_t *)event_data;
                ESP_LOGE(TAG, "Provisioning failed!\n\tReason : %s"
                         "\n\tPlease reset to factory and retry provisioning",
                         (*reason == WIFI_PROV_STA_AUTH_ERROR) ?
                         "Wi-Fi station authentication failed" : "Wi-Fi access-point not found");
                retries++;
                if (retries >= 5) {
                    ESP_LOGI(TAG, "Failed to connect with provisioned AP, reseting provisioned credentials");
                    wifi_prov_mgr_reset_sm_state_on_failure();
                    retries = 0;
                }
                break;
            }

            case WIFI_PROV_CRED_SUCCESS:
                ESP_LOGI(TAG, "Provisioning successful");
                retries = 0;
                xEventGroupSetBits(ble_event_group, WIFI_CRED_RECV_BIT);
                break;
            
            case WIFI_PROV_END:
                /* De-initialize manager once provisioning is finished */
                wifi_prov_mgr_deinit();
                ESP_LOGE(TAG,"wifi_prov_mgr_deinit");
                break;
                
            default:
                break;
        }
    }
}