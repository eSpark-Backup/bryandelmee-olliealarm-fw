#ifndef NOOZLE_FACTORY_SETUP_H
#define NOOZLE_FACTORY_SETUP_H

#include "nvs_module.h"
#include "wifi_module.h"
#include "prov_module.h"
#include "cloud_api_module.h"

typedef enum
{
    DEVICE_UNREGISTERED = 0,
    DEVICE_REGISTERED   = 1 
}noozle_register_status;

esp_err_t noozle_device_initialization();
esp_err_t noozle_device_create_event_groups();

esp_err_t noozle_factory_setup();
noozle_register_status noozle_get_registeration_status();
esp_err_t noozle_factory_registeration_run();

char* noozle_get_device_name();

#endif