#pragma once

#include <stdio.h>
#include <map>
#include <string>
#include <cJSON.h>
#include "esp_log.h"
#include "esp_event.h"
#include <esp_err.h>
#include "esp_netif.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/ledc.h"

// #include "esp_websocket_client.h"
#include "sio_client.h"

void eventHandlerSIO(void *arg, esp_event_base_t eventBase, int32_t eventID, void *eventData);

enum event_name {
    BRIGHTNESS_UPDATE, 
    ALARM_ADDED, 
    UPDATE_ALARM, 
    TESTING_MODE, 
    VOLUME_UPDATE, 
    NEW_NAP_TIME, 
    UPDATE_NAP_TIME, 
    DISCONNECT_DEVICE, 
    UNKNOWN
};

class plug{
    public:
        esp_err_t set_handler();
        esp_err_t configure();
};

