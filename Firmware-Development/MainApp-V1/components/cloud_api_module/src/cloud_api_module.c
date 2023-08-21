#include "cloud_api_module.h"

EventGroupHandle_t cloud_api_event_group;
const int cloud_api_http_finish_event = BIT0;

char *cloud_buffer = NULL;
size_t cloud_buffer_len = 0;

const char* noozle_time_api = "http://dev.noozle.nl:3000/time/current";
const char* noozle_repeated_alarm_api = "http://dev.noozle.nl:3000/scenes/getRepeatedAlarmsByOllieID";

const char* noozle_device_id = "{\"ollie_id\":\"64d249aa500db82a5dad3827\"}";

esp_err_t noozle_cloud_service(noozle_api_t API)
{
    switch (API)
    {
        case NOOZLE_TIME_API:
            uint64_t time;
            xEventGroupClearBits(cloud_api_event_group, cloud_api_http_finish_event);
            while(ESP_OK != cloud_api_request(noozle_time_api, HTTP_METHOD_GET, NULL));
            cloud_api_wait_finish_event();
            cloud_api_parse_current_time(cloud_buffer, &time);
            ESP_LOGI("TAG", "CURRENT TIME : %llu", time);
            setenv("TZ", "GMT-5", 1);
            setEpochTime(time, 0);
            break;
        
        case NOOZLE_REPEATED_ALARM_API:
            xEventGroupClearBits(cloud_api_event_group, cloud_api_http_finish_event);
            while(ESP_OK != cloud_api_request(noozle_repeated_alarm_api, HTTP_METHOD_POST, NULL));
            cloud_api_wait_finish_event();
            break;

        default:
            break;
    }
    
    return ESP_OK;
}

esp_err_t cloud_api_create_event_group()
{
    cloud_api_event_group = xEventGroupCreate();
    if( cloud_api_event_group == NULL )
        return ESP_FAIL;

    return ESP_OK;
}

esp_err_t cloud_api_wait_finish_event()
{
    xEventGroupWaitBits(cloud_api_event_group, cloud_api_http_finish_event, true, true, portMAX_DELAY);
    return ESP_OK;
}

esp_err_t cloud_api_request(const char* api, esp_http_client_method_t http_method, void* user_data)
{
    esp_err_t ret = ESP_OK;

    esp_http_client_config_t config = { .url = api , .event_handler = cloud_http_event_handler };
    esp_http_client_handle_t client = esp_http_client_init(&config);
    ret |= esp_http_client_set_url(client, api);
    ret |= esp_http_client_set_method(client, http_method);
    ret |= esp_http_client_set_header(client, "Content-Type", "application/json");
    ret |= esp_http_client_set_user_data(client, user_data);
    if(http_method == HTTP_METHOD_POST)
        ret |= esp_http_client_set_post_field(client, noozle_device_id, strlen(noozle_device_id));
    ret |= esp_http_client_perform(client);
    int status_code = esp_http_client_get_status_code(client);
    ret |= esp_http_client_cleanup(client);

    ESP_LOGI("Status Code","%d",status_code);
    if(status_code != 200)
        ret = ESP_FAIL;
    return ret;
}

void cloud_free_buf()
{
    if(cloud_buffer != NULL)
    {
        free(cloud_buffer);
        cloud_buffer = NULL;
    }
}

void cloud_copy_response(esp_http_client_event_t *evt)
{
    cloud_buffer = realloc(cloud_buffer , evt->data_len);
    memcpy(cloud_buffer, evt->data, evt->data_len);
    cloud_buffer_len += evt->data_len;
    ESP_LOGI("Noozle-API", "cloud_buffer: %s", cloud_buffer);
}

esp_err_t cloud_http_event_handler(esp_http_client_event_t *evt)
{
    switch (evt->event_id) {
        case HTTP_EVENT_ON_DATA:
            ESP_LOGI("Noozle-API", "evt->data: %s", (char *)evt->data);
            cloud_free_buf();
            cloud_copy_response(evt);
            break;

        case HTTP_EVENT_ON_FINISH:
            xEventGroupSetBits(cloud_api_event_group, cloud_api_http_finish_event);
            ESP_LOGI("Noozle-API", "HTTP_EVENT_ON_FINISH");
            break;

        default:
            break;
    }
    return ESP_OK;
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