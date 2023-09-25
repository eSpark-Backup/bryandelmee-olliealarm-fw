#pragma once

#include "NVS.hpp"
#include "wifi.hpp"
#include "rtc.hpp"
#include "BLEProv.hpp"
#include <time.h>
#include "cloud_api_module.h"
#include "alarm_config.h"

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
        void setAlarmsFromNvs();
        void bits_to_bool(uint8_t bits);
        void updateTime();
        void setNearestAlarm();
        void updateBatteryStatus();
        void checkAlarm();
        
    private:
        NVS* nvs;
        WIFI* wifi;
        BLEProv* ble;
        RTC* rtc;
        alarm_config* wakeAlarm;
        alarm_config* sleepAlarm;
        alarm_config* nearestAlarm;
        uint8_t weekday_bits;
        bool weekday[7];
        
};
