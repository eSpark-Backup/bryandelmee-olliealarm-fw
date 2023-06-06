#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"

#include "esp_http_client.h"
#include "connect_wifi.h"
#include <cJSON.h>

static const char *TAG = "HTTP_CLIENT";

#define HTTP_RESPONSE_BUFFER_SIZE 1024
char *response_data = NULL;
size_t response_len = 0;
bool all_chunks_received = false;
const char* api = "http://dev.noozle.nl:3000/scenes/getRepeatedAlarmsByOllieID?ollie_id=64688d9f53f1cb253ffb67a2";

void parseJson(const char* jsonString) {

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
                const char* time = cJSON_GetObjectItemCaseSensitive(sceneSleepTime, "time")->valuestring;
                const char* str_brightness = cJSON_GetObjectItemCaseSensitive(sceneSleepTime, "brightness")->valuestring;
                double brightness = atof(str_brightness);
                printf("Scene: %s, time: %s, brightness: %lf \n", name, time, brightness);
            }
        }
    }

    cJSON_Delete(root);
    free(jsonString);
}


esp_err_t _http_event_handler(esp_http_client_event_t *evt)
{
    switch (evt->event_id) {
        case HTTP_EVENT_ON_DATA:
            // Resize the buffer to fit the new chunk of data
            response_data = realloc(response_data, response_len + evt->data_len);
            memcpy(response_data + response_len, evt->data, evt->data_len);
            response_len += evt->data_len;
            break;
        case HTTP_EVENT_ON_FINISH:
            all_chunks_received = true;
            ESP_LOGI("Noozle-API", "Received data: %s", response_data);
            parseJson(response_data);
            break;
        default:
            break;
    }
    return ESP_OK;
}

void noozle_api_http(void *pvParameters)
{
    esp_http_client_config_t config = {
        .url = api,
        .method = HTTP_METHOD_POST,
        .event_handler = _http_event_handler,
    };

    esp_http_client_handle_t client = esp_http_client_init(&config);

    // POST
    const char *post_data = "{\"ollie_id\":\"64688d9f53f1cb253ffb67a2\"}";
    esp_http_client_set_url(client, "http://dev.noozle.nl:3000/scenes/getRepeatedAlarmsByOllieID");
    esp_http_client_set_method(client, HTTP_METHOD_POST);
    esp_http_client_set_header(client, "Content-Type", "application/json");
    esp_http_client_set_post_field(client, post_data, strlen(post_data));
    esp_err_t err = esp_http_client_perform(client);

    if (err == ESP_OK)
    {
        int status_code = esp_http_client_get_status_code(client);
        if (status_code == 200)
        {
            ESP_LOGI(TAG, "Message sent Successfully");
        }
        else
        {
            ESP_LOGI(TAG, "Message sent Failed");
        }
    }
    else
    {
        ESP_LOGI(TAG, "Message sent Failed");
    }
    esp_http_client_cleanup(client);
    vTaskDelete(NULL);
}

void app_main(void)
{
	esp_err_t ret = nvs_flash_init();
	if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
	{
		ESP_ERROR_CHECK(nvs_flash_erase());
		ret = nvs_flash_init();
	}
	ESP_ERROR_CHECK(ret);
	connect_wifi();
	if (wifi_connect_status)
	{
		xTaskCreate(&noozle_api_http, "noozle_api_http", 8192, NULL, 6, NULL);
	}
}