#include "BLEProv.hpp"
#include <cstring>
#include "esp_log.h"
#include "esp_check.h"
#include "esp_wifi.h"

static const char* TAG = "BLEProv";

esp_err_t BLEProv::setup()
{
    ESP_RETURN_ON_ERROR(initProvisioningManager(), TAG, "setup: initProvisioningManager");
    ESP_RETURN_ON_ERROR(setServiceName(), TAG, "setup: setServiceName");
    ESP_RETURN_ON_ERROR(setServiceUUID(), TAG, "setup: setServiceUUID");
    setSecurity(WIFI_PROV_SECURITY_1);
    setPOP("abcd1234");

    return ESP_OK;
}

esp_err_t BLEProv::start()
{
    /* Start provisioning service */
    ESP_RETURN_ON_ERROR(wifi_prov_mgr_start_provisioning(security, pop, service_name, service_key), TAG, "start: wifi_prov_mgr_start_provisioning");
    // wifi_prov_mgr_endpoint_register("custom-data", custom_prov_data_handler, NULL);
    return ESP_OK;
}

/**
 * @brief Initialize provisioning manager with the configuration parameters set above.
*/
esp_err_t BLEProv::initProvisioningManager()
{
    /* Configuration for the provisioning manager */
    wifi_prov_mgr_config_t config = {
        .scheme = wifi_prov_scheme_ble,
        .scheme_event_handler = WIFI_PROV_SCHEME_BLE_EVENT_HANDLER_FREE_BT ,
        .app_event_handler{
            .event_cb = NULL,
            .user_data = NULL
        }
    };
    return wifi_prov_mgr_init(config);
}

/**
 * @brief Sets the Bluetooth Service/Device name. eg: "PROV_xxxxxxx"
*/
esp_err_t BLEProv::setServiceName()
{
    esp_err_t ret;

    uint8_t eth_mac[6];
    const char *ssid_prefix = "PROV_";
    ret = esp_wifi_get_mac(WIFI_IF_STA, eth_mac);   /* esp_wifi_init() must be called before, else it'll return an error.  */

    /* Make a string from ssid_prefix & MAC-Address index 3,4,5 & sets it in the service name */
    snprintf(service_name, sizeof(service_name), "%s%02X%02X%02X",
             ssid_prefix, eth_mac[3], eth_mac[4], eth_mac[5]);

    return ret;
}

void BLEProv::printServiceName()
{
    ESP_LOGI(TAG,"%s", this->service_name);
}

/**
 * @brief Sets a custom provisioning service uuid.
*/
esp_err_t BLEProv::setServiceUUID()
{
    char uuid[16] = {
        /* LSB <---------------------------------------
         * ---------------------------------------> MSB */
        0xb4, 0xdf, 0x5a, 0x1c, 0x3f, 0x6b, 0xf4, 0xbf,
        0xea, 0x4a, 0x82, 0x03, 0x04, 0x90, 0x1a, 0x02,
    };
    memcpy(custom_service_uuid, uuid, sizeof(uuid));
    return wifi_prov_scheme_ble_set_service_uuid(custom_service_uuid);
}
/**
 * @brief Sets the security mode of the Provisioning.
 *          WIFI_PROV_SECURITY_0 [No security]
 *          WIFI_PROV_SECURITY_1 [X25519 key exchange]
 *          WIFI_PROV_SECURITY_2 [SRP6a based authentication and key exchange] 
*/
void BLEProv::setSecurity(wifi_prov_security security)
{
    this->security = security;

    #if DEBUG 
    ESP_LOGI(TAG,"Security Level = %d", this->security);
    #endif
}

/**
 * @brief Sets the Proof-Of-Possesion, used in WIFI_PROV_SECURITY_1 where authentication is done using a pop.
*/
void BLEProv::setPOP(const char* pop)
{
    this->pop = pop;

    #if DEBUG 
    ESP_LOGI(TAG,"POP = %s", this->pop);
    #endif
}