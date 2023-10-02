#ifndef CLOUD_API_MOUDLE_H
#define CLOUD_API_MOUDLE_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/event_groups.h"
#include "nvs_flash.h"
#include "esp_check.h"
/* Cloud-API */
#include "esp_http_client.h"
#include <cJSON.h>

/* RTC MOUDLE*/
#include "rtc_module.h"
#define MAX_HTTP_OUTPUT_BUFFER 2048

// #include "noozle_factory_setup.h"

#define ALARM_NAMESPACE "ALARM"
#define ALARM_KEY "alarm_epoch"
#define ALARM_NAME "alarm_name"

#ifdef __cplusplus
extern "C" {
#endif



typedef enum{
    NOOZLE_TIME_API = 0,
    NOOZLE_GET_ID_API,
    NOOZLE_REPEATED_ALARM_API,
    NOOZLE_BATTERY_UPDATE_API
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

typedef enum{
    ALARM_WAKE = 0,
    ALARM_SLEEP,
    ALARM_NONE
}alarm_name_t;

esp_err_t noozle_cloud_service(noozle_api_t API);
esp_err_t cloud_api_request(const char* api, esp_http_client_method_t http_method, void* user_data);
esp_err_t cloud_http_event_handler(esp_http_client_event_t *evt);

/*cloud_api_json_parsing*/
esp_err_t cloud_api_parse_current_time(const char* jsonString, uint64_t* time);
esp_err_t cloud_api_parse_ollie_id(const char* jsonString, char** noozle_dev_id, char** noozle_batt_id);
esp_err_t cloud_api_parse_repeated_alarms(const char* jsonString);
esp_err_t cloud_api_success_status(cJSON* root);
esp_err_t cloud_api_parse_repeated_alarms_epoch(cJSON* root, int64_t* alarm_epoch);
esp_err_t cloud_api_create_battery_status_json(char** noozle_dev_id, char** noozle_batt_json);
esp_err_t save_alarms_in_nvs(int64_t* alarm_epoch, size_t size);

void cloud_free_buf();
void cloud_copy_response(esp_http_client_event_t *evt);

#ifdef __cplusplus
}
#endif

#endif