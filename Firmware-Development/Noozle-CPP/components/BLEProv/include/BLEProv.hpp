#pragma once

#include <wifi_provisioning/scheme_ble.h>

class BLEProv
{
    private:
        char service_name[12];
        uint8_t custom_service_uuid[16];
        const char *service_key = NULL;
        const char *pop = NULL;
        wifi_prov_security_t security;

    public:
        BLEProv(/* args */){}
        ~BLEProv(){}
        esp_err_t setup();
        esp_err_t start();
        esp_err_t restart();
        void stop();
        esp_err_t initProvisioningManager();
        esp_err_t setServiceName();
        esp_err_t setServiceUUID();
        void setSecurity(wifi_prov_security security);
        void setPOP(const char* pop);
        
        esp_err_t registerEventHandler();
        static void eventHandler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);

        void printServiceName();
        
};