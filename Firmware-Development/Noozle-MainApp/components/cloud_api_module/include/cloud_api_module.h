#ifndef CLOUD_API_MOUDLE_H
#define CLOUD_API_MOUDLE_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
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

void cloud_api_task(void *alarmTimeBuf);
void cloud_api_set_config(esp_http_client_config_t* config, void* alarmTimeBuf);
esp_err_t cloud_api_set_http_post_request(esp_http_client_handle_t client);
esp_err_t cloud_api_check_http_response_status(esp_http_client_handle_t* client);
esp_err_t cloud_api_http_event_handler(esp_http_client_event_t *evt);
void cloud_api_parse_time_and_brightness(const char* jsonString, alarmtime_t* alarmTime);

#endif