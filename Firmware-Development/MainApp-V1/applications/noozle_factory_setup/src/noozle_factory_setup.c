#include "noozle_factory_setup.h"

static const char* TAG = "NOOZLE_FACTORY_SETUP";

esp_err_t noozle_device_initialization()
{
    nvs_init();
    noozle_device_create_event_groups();
    esp_netif_init();   /*Initialize TCP/IP STACk*/
    esp_event_loop_create_default();
    noozle_wifi_initialize();
    return ESP_OK;
}

esp_err_t noozle_device_create_event_groups()
{
     /*Create a Event Groups*/
     wifi_create_event_group();
     return ESP_OK;
}

esp_err_t noozle_factory_setup()
{
    noozle_register_status dev_reg_satus = noozle_get_registeration_status();
    
    switch (dev_reg_satus)
    {
        case DEVICE_UNREGISTERED:
            noozle_factory_registeration_run();
            nvs_open_namespace_rw("storage");
            nvs_write_key_value_u8("noozle_register", DEVICE_REGISTERED);
            nvs_close_handle();
            break;

        case DEVICE_REGISTERED:
            noozle_wifi_service_run();
            break;

        default:
            break;
    }

    return ESP_OK;
}
/**
 * @brief   Open NVS namespace "storage", checks the status of noozle_register.
 * @return  noozle_register_status : { 0 = DEVICE_UNREGISTERED, 1 = DEVICE_REGISTERED }
*/
noozle_register_status noozle_get_registeration_status()
{
    esp_err_t ret;
    uint8_t noozle_register = DEVICE_UNREGISTERED;  /* factory_setup : 0 represents that the device is not registered yet. */

    nvs_open_namespace_rw("storage");
    ret = nvs_read_key_value_u8("noozle_register", &noozle_register);

    /* It represents the Device First BootUp and noozle_register is not defined in NVS yet neither device is registered. */
    if(ret == ESP_ERR_NVS_NOT_FOUND)
          nvs_write_key_value_u8("noozle_register", noozle_register);

    nvs_close_handle();
    return noozle_register;
}

esp_err_t noozle_factory_registeration_run()
{
    return noozle_provisioning_service();
}

char* noozle_get_device_name() 
{
    uint8_t eth_mac[6];
    const char *ssid_prefix = "PROV_";
    esp_wifi_get_mac(WIFI_IF_STA, eth_mac);

    char* device_name = (char*)malloc(12 * sizeof(char));
    snprintf(device_name, 12, "%s%02X%02X%02X",
             ssid_prefix, eth_mac[3], eth_mac[4], eth_mac[5]);

    return device_name;
}