#include "cloud_api_module.h"

EventGroupHandle_t cloud_api_event_group;
const int got_repeated_alarm    = BIT0;
const int got_current_time      = BIT1;

char *response_data = NULL;
char *response_data2 = NULL;
size_t response_len = 0;
size_t response_len2 = 0;

const char* noozle_time_api = "http://dev.noozle.nl:3000/time/current";
const char* noozle_api = "http://dev.noozle.nl:3000/scenes/getRepeatedAlarmsByOllieID"; 
const char* noozle_post_data = "{\"ollie_id\":\"2\"}";

esp_err_t cloud_api_create_event_group()
{
    cloud_api_event_group = xEventGroupCreate();
    if( cloud_api_event_group == NULL )
        return ESP_FAIL;

    return ESP_OK;
}

esp_err_t cloud_api_wait_repeated_alarm_event(){
    xEventGroupWaitBits(cloud_api_event_group, got_repeated_alarm, true, true, portMAX_DELAY);
    return ESP_OK;
}

esp_err_t cloud_api_wait_current_time_event(){
    xEventGroupWaitBits(cloud_api_event_group, got_current_time, true, true, portMAX_DELAY);
    return ESP_OK;
}

void cloud_api_repeated_alarm_task(void* user_data)
{
    ESP_LOGI("TAG","repeated!");
    esp_http_client_config_t config = {
        .url = noozle_api,
        .method = HTTP_METHOD_POST,
        .event_handler = alarm_http_event_handler,
        .user_data = user_data,
    };
    esp_http_client_handle_t client = esp_http_client_init(&config);
    esp_http_client_set_url(client, noozle_api);
    esp_http_client_set_method(client, HTTP_METHOD_POST);
    esp_http_client_set_header(client, "Content-Type", "application/json");
    esp_http_client_set_post_field(client, noozle_post_data, strlen(noozle_post_data));
    esp_http_client_perform(client);
    // cloud_api_check_http_response_status(client);
    int status_code = esp_http_client_get_status_code(client);
    ESP_LOGI("Status Code","%d",status_code);
    
    esp_http_client_cleanup(client);
    vTaskDelete(NULL);
}

void cloud_api_current_time_task(void *user_data)
{    
    esp_http_client_config_t config = {
        .url = noozle_api,
        .method = HTTP_METHOD_POST,
        .event_handler = time_http_event_handler,
        .user_data = user_data,
    };
    esp_http_client_handle_t client = esp_http_client_init(&config);
    esp_http_client_set_url(client, noozle_time_api);
    esp_http_client_set_method(client, HTTP_METHOD_GET);
    esp_http_client_set_header(client, "Content-Type", "application/json");
    esp_http_client_perform(client);
    int status_code = esp_http_client_get_status_code(client);
    ESP_LOGI("Status Code","%d",status_code);
    
    esp_http_client_cleanup(client);
    vTaskDelete(NULL);
}

esp_err_t alarm_http_event_handler(esp_http_client_event_t *evt)
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
            ESP_LOGI("Noozle-API", "Received data: %s", response_data);
            // cloud_api_parse_time_and_brightness(response_data, alarmTime);
            xEventGroupSetBits(cloud_api_event_group, got_repeated_alarm);
            break;
        default:
            break;
    }
    return ESP_OK;
}

esp_err_t time_http_event_handler(esp_http_client_event_t *evt)
{
    switch (evt->event_id) {
        case HTTP_EVENT_ON_DATA:
            // Resize the buffer to fit the new chunk of data
            response_data2 = realloc(response_data2, response_len2 + evt->data_len);
            memcpy(response_data2 + response_len2, evt->data, evt->data_len);
            response_len2 += evt->data_len;
            break;

        case HTTP_EVENT_ON_FINISH:
            uint64_t* time = (uint64_t*)evt->user_data;
            ESP_LOGI("Noozle-API", "Received data: %s", response_data2);
            if(ESP_OK == cloud_api_parse_current_time(response_data2, time))
                xEventGroupSetBits(cloud_api_event_group, got_current_time);
            free(response_data2);
            response_data2 = NULL;
            response_len2 = 0;
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
                printf("Scene: %s, time: %llu, brightness: %lf \n", name, (alarmTime+i)->epoch_time, (alarmTime+i)->brightness);
                }
        }
    }
    cJSON_Delete(root);
    free((void*)jsonString);
    jsonString = NULL;
}

esp_err_t cloud_api_parse_current_time(const char* jsonString, uint64_t* time)
{
    // Parse the JSON string
    cJSON* root = cJSON_Parse(jsonString);
    if (root == NULL) {
        printf("Error: Failed to parse JSON.\n");
        return ESP_FAIL;
    }

    // Get the value of the "time" key
    cJSON* time_value = cJSON_GetObjectItem(root, "time");
    if (time_value == NULL || !cJSON_IsNumber(time_value)) {
        printf("Error: Invalid JSON format.\n");
        cJSON_Delete(root);
        return ESP_FAIL;
    }

    // Convert the value to uint64_t
    *time = (uint64_t)time_value->valuedouble;

    // Clean up cJSON objects
    cJSON_Delete(root);
    return ESP_OK;
}