#include "wifi_prov_app.h"

static const char* TAG = "WIFI_PROV_APP";

esp_err_t wifi_prov_app_setup()
{
    esp_err_t err;
    wifi_config_t wifi_config;
    nvs_init();
    esp_netif_init();
    esp_event_loop_create_default();
    wifi_create_event_group();
    prov_create_event_group();
    wifi_register_events();
    prov_register_events();
    esp_netif_create_default_wifi_sta();
    wifi_init();

    nvs_open_namespace_rw(WIFI_NAMESPACE);
    err = wifi_get_credetianls_nvs(&wifi_config, WIFI_KEY);
    wifi_config.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;
    wifi_config.sta.bssid_set = 0;
    
    if(err == ESP_ERR_NVS_NOT_FOUND)
    {
        prov_mgr_init();
        prov_mgr_setup();
        prov_mgr_credential_recv_wait();
    }
    esp_wifi_set_mode(WIFI_MODE_STA);   
    esp_wifi_set_config(WIFI_IF_STA, &wifi_config);
    esp_wifi_start();
    wifi_connction_wait();
    nvs_close_handle();

    return ESP_OK;
}