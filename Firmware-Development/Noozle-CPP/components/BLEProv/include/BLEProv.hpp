#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "esp_bit_defs.h"
#include <wifi_provisioning/scheme_ble.h>

class BLEProv
{
    private:
        char service_name[12];
        uint8_t custom_service_uuid[16];
        const char *service_key = NULL;
        const char *pop = NULL;
        wifi_prov_security_t security;
        EventGroupHandle_t event_group;
        const int BLEPROV_SUCCESS_BIT =  BIT0;

    public:
        BLEProv(/* args */){}
        ~BLEProv(){}
        esp_err_t setup();
        esp_err_t run();
        esp_err_t restart();
        void stop();
        void wait();
        esp_err_t initProvisioningManager();
        esp_err_t setServiceName();
        esp_err_t setServiceUUID();
        void setSecurity(wifi_prov_security security);
        void setPOP(const char* pop);
        
        esp_err_t setupEventGroup();
        esp_err_t setupEventHandler();
        static void eventHandlerBLE(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);

        void printServiceName();
        
};