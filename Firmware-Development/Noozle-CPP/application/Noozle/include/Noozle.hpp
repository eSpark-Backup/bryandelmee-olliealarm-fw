#pragma once

#include "NVS.hpp"
#include "wifi.hpp"
#include "BLEProv.hpp"

enum prov_status_t : uint8_t { PROVISIONING_STATUS_FAIL, PROVISIONING_STATUS_OK };

class Noozle
{
    public:
        Noozle();
        esp_err_t setup();
        esp_err_t setupNVS();
        esp_err_t connectWifi();
        esp_err_t connectBLE();
        esp_err_t setProvStatus(uint8_t status);
        uint8_t getProvStatus();
        esp_err_t bootCountUpdate();
        void checkProvStatusAndRunBleOrWifi();
        
    private:
        NVS* nvs;
        WIFI* wifi;
        BLEProv* ble;
};