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
#include "freertos/semphr.h"
#include "freertos/event_groups.h"

/* Cloud-API */
#include "esp_http_client.h"
#include <cJSON.h>

/* RTC MOUDLE*/
#include "rtc_module.h"

typedef enum{
    NOOZLE_TIME_API = 0,
    NOOZLE_REPEATED_ALARM_API
}noozle_api_t;

#define CLOUD_DEBUG 1

typedef struct{
    int64_t epoch_time;
    float brightness;
} alarmtime_t;

typedef enum {
    ALMOST_SLEEP,
    TO_SLEEP,
    ALMOST_GETTING_UP,
    TO_GET_UP
}alarmtime_enum;

esp_err_t noozle_cloud_service(noozle_api_t API);

esp_err_t cloud_api_create_event_group();
esp_err_t cloud_api_wait_finish_event();
esp_err_t cloud_api_request(const char* api, esp_http_client_method_t http_method, void* user_data);
esp_err_t cloud_http_event_handler(esp_http_client_event_t *evt);

esp_err_t cloud_api_parse_current_time(const char* jsonString, uint64_t* time);

void cloud_free_buf();
void cloud_copy_response(esp_http_client_event_t *evt);

#endif