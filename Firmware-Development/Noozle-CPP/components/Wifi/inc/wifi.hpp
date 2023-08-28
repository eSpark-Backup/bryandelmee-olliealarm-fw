#pragma once 

#include "esp_wifi.h"
#include "freertos/event_groups.h"
#include "esp_bit_defs.h"

class WIFI
{
    private:
    wifi_config_t config;
    esp_netif_t *netif;
    EventGroupHandle_t event_group;
    const int WIFI_CONNECTED_BIT =  BIT0;
    const int WIFI_FAIL_BIT = BIT1;
    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;

    public:

    esp_err_t setup();
    esp_err_t run();
    esp_err_t init();
    esp_err_t netifCreateWifiSTA();
    esp_err_t getConfig();
    esp_err_t setConfig(const char *ssid, const char* password);
    esp_err_t connect();
    esp_err_t wait();
    void printCredentials();

    esp_err_t setupEventGroup();
    esp_err_t setupEventHandler();
    static void eventHandlerWIFI(void *arg, esp_event_base_t eventBase, int32_t eventID, void *eventData);
    static void eventHandlerIP(void *arg, esp_event_base_t eventBase, int32_t eventID, void *eventData);
};

