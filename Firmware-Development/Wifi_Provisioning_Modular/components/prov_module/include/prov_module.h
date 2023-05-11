#ifndef PROV_MOUDLE_H
#define PROV_MOUDLE_H

#include "wifi_module.h"
#include "nvs_module.h"
#include <wifi_provisioning/manager.h>
#include <wifi_provisioning/scheme_ble.h>

#define WIFI_PROV_SECURITY_LEVEL  WIFI_PROV_SECURITY_1

typedef struct{
    char service_name[12];
    uint8_t custom_service_uuid[16];
    const char *service_key;
    const char *pop;
    wifi_prov_security_t security;
}prov_data_t;

esp_err_t prov_mgr_init();
esp_err_t prov_create_event_group(); 
esp_err_t prov_register_events();
esp_err_t prov_mgr_setup();
esp_err_t prov_mgr_set_data(prov_data_t* prov_data);
esp_err_t prov_mgr_credential_recv_wait();

esp_err_t custom_prov_data_handler(uint32_t session_id, const uint8_t *inbuf, ssize_t inlen,
                                          uint8_t **outbuf, ssize_t *outlen, void *priv_data);

#endif