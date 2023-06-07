#ifndef CLOUD_API_MOUDLE_H
#define CLOUD_API_MOUDLE_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include "esp_log.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

/* Cloud-API */
#include "esp_http_client.h"
#include <cJSON.h>

typedef struct{
    uint32_t epoch_time;
    float brightness;
} alarmtime_t;

typedef enum {
    ALMOST_SLEEP,
    TO_SLEEP,
    ALMOST_GETTING_UP,
    TO_GET_UP
}alarmtime_enum;

void cloud_api_current_time_task(void *user_data);
esp_err_t time_http_event_handler(esp_http_client_event_t *evt);
void cloud_api_parse_current_time(const char* jsonString, uint64_t* time);
#endif