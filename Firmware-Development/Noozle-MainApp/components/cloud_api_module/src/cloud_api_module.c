#include "cloud_api_module.h"

#define CLOUD_RESPONSE_BUFFER_SIZE 1024
char *response_data = NULL;
size_t response_len = 0;
bool all_chunks_received = false;

static const char* noozle_api = "http://dev.noozle.nl:3000/scenes/getRepeatedAlarmsByOllieID?ollie_id=64688d9f53f1cb253ffb67a2";
static const char* noozle_post_data = "{\"ollie_id\":\"64688d9f53f1cb253ffb67a2\"}";

void cloud_api_set_config(esp_http_client_config_t* config, void* alarmTimeBuf)
{
    config->user_data = alarmTimeBuf;
    config->event_handler = cloud_api_http_event_handler;
}

esp_err_t cloud_api_set_http_post_request(esp_http_client_handle_t client)
{
    esp_err_t err;
    err = esp_http_client_set_url(client, noozle_api);
    err = esp_http_client_set_method(client, HTTP_METHOD_POST);
    err = esp_http_client_set_header(client, "Content-Type", "application/json");
    err = esp_http_client_set_post_field(client, noozle_post_data, strlen(noozle_post_data));
    err = esp_http_client_perform(client);
    return err;
}

esp_err_t cloud_api_check_http_response_status(esp_http_client_handle_t* client)
{
    int status_code = esp_http_client_get_status_code(client);
    if (status_code == 200)
        return ESP_OK;
    
    return ESP_FAIL;
}

void cloud_api_task(void *alarmTimeBuf)
{    
    esp_http_client_config_t config;
    cloud_api_set_config(&config, alarmTimeBuf);
    esp_http_client_handle_t client; = esp_http_client_init(&config);
    cloud_api_set_http_post_request(client);
    cloud_api_check_http_response_status(client);

    esp_http_client_cleanup(client);
    vTaskDelete(NULL);
}

esp_err_t cloud_api_http_event_handler(esp_http_client_event_t *evt)
{
    switch (evt->event_id) {
        case HTTP_EVENT_ON_DATA:
            // Resize the buffer to fit the new chunk of data
            response_data = realloc(response_data, response_len + evt->data_len);
            memcpy(response_data + response_len, evt->data, evt->data_len);
            response_len += evt->data_len;
            break;
        case HTTP_EVENT_ON_FINISH:
            // alarmtime_t* alarmTime = (alarmtime_t*)evt->user_data;
            all_chunks_received = true;
            ESP_LOGI("Noozle-API", "Received data: %s", response_data);
            // cloud_api_parse_time_and_brightness(response_data, alarmTime);
            break;
        default:
            break;
    }
    return ESP_OK;
}

void cloud_api_parse_time_and_brightness(const char* jsonString, alarmtime_t* alarmTime)
{
    cJSON* root = cJSON_Parse(jsonString);
    if (root == NULL) {
        cJSON_Delete(root);
        return;
    }

    cJSON* data = cJSON_GetObjectItemCaseSensitive(root, "data");
    if (cJSON_IsArray(data)) {
        cJSON* scene = cJSON_GetArrayItem(data, 0);
        cJSON* sceneSleepTimes = cJSON_GetObjectItemCaseSensitive(scene, "scene_sleep_time");
        if (cJSON_IsArray(sceneSleepTimes)) {
            int sceneSleepTimeCount = cJSON_GetArraySize(sceneSleepTimes);
            for (int i = 0; i < sceneSleepTimeCount; i++) {
                cJSON* sceneSleepTime = cJSON_GetArrayItem(sceneSleepTimes, i);
                const char* name = cJSON_GetObjectItemCaseSensitive(sceneSleepTime, "scene_sleep_time_name")->valuestring;
                char* time = cJSON_GetObjectItemCaseSensitive(sceneSleepTime, "time")->valuestring;
                time = "1686046904";
                (alarmTime+i)->epoch_time = atoi(time);
                const char* str_brightness = cJSON_GetObjectItemCaseSensitive(sceneSleepTime, "brightness")->valuestring;
                (alarmTime+i)->brightness = atof(str_brightness);
                printf("Scene: %s, time: %ld, brightness: %lf \n", name, (alarmTime+i)->epoch_time, (alarmTime+i)->brightness);
                }
        }
    }
    cJSON_Delete(root);
    free((void*)jsonString);
    jsonString = NULL;
}