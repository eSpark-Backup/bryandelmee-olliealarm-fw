#include "wifi.hpp"

#include <string.h>
#include "esp_log.h"
#include "esp_err.h"
#include "esp_check.h"
#include "esp_event.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"

static const char* TAG = "Wifi-Moudle";

esp_err_t WIFI::setup()
{
    ESP_RETURN_ON_ERROR(setupEventGroup(), TAG, "setup: Setting up WiFi event group");
    ESP_RETURN_ON_ERROR(setupEventHandler(), TAG, "setup: Setting up WiFi callbacks");
    ESP_RETURN_ON_ERROR(netifCreateWifiSTA(), TAG, "setup: Setting up default WiFi STA");
    ESP_RETURN_ON_ERROR(init(), TAG, "setup: Initialize WiFi");
    ESP_RETURN_ON_ERROR(esp_wifi_set_mode(WIFI_MODE_STA), TAG, "setup : Setting WiFi mode to STA");
    ESP_RETURN_ON_ERROR(getConfig(), TAG, "setup: Fetching saved WiFi configurations");

    return ESP_OK;
}

esp_err_t WIFI::run()
{
    ESP_RETURN_ON_ERROR(esp_wifi_start(), TAG, "run: Starts WiFi");
    return ESP_OK;
}

/**
 * @brief Custom function to create default WIFI STA. 
 *          Official API : esp_netif_create_default_wifi_sta();
 *          To avoid any kindof 'abort' from Official API, this custom function returns an error.           
 * 
 * @return esp_err_t 
 */
esp_err_t WIFI::netifCreateWifiSTA()
{
    esp_netif_config_t cfg = ESP_NETIF_DEFAULT_WIFI_STA();
    netif = esp_netif_new(&cfg);
    if(netif == NULL)
        return ESP_FAIL;
    ESP_RETURN_ON_ERROR(esp_netif_attach_wifi_station(netif), TAG, "Attaches wifi station interface to supplied netif");
    ESP_RETURN_ON_ERROR(esp_wifi_set_default_wifi_sta_handlers(), TAG, "Sets default wifi event handlers for STA interface");
    return ESP_OK;
}

/**
 * @brief Intialize the WIFI Driver, this fucntion must be called before all other WiFi API can be called.
 * 
 * @return esp_err_t 
 */
esp_err_t WIFI::init()
{
    wifi_init_config_t init_config = WIFI_INIT_CONFIG_DEFAULT();
    return esp_wifi_init(&init_config);
}

/**
 * @brief Intialized the 'config' with already stored wifi configurations from NVS.
 * 
 * @return esp_err_t 
 */
esp_err_t WIFI::getConfig()
{
    return esp_wifi_get_config(WIFI_IF_STA, &config);
}

/**
 * @brief Stored the wifi credentials in NVS.
 * 
 * @param ssid 
 * @param password 
 * @return esp_err_t 
 */
esp_err_t WIFI::setConfig(const char *ssid, const char* password)
{
    /* Copy the SSID & Password into wifi 'config' */
    strcpy((char*)config.sta.ssid, ssid);
    strcpy((char*)config.sta.password, password);
    /* Update Wifi configurations in NVS */
    return esp_wifi_set_config(WIFI_IF_STA, &config);
}

esp_err_t WIFI::connect()
{
    return esp_wifi_connect();
}

void WIFI::printCredentials()
{
    ESP_LOGI(TAG,"SSID : %s, PASSWORD: %s", config.sta.ssid, config.sta.password);
}