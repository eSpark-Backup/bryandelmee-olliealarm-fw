#include "cloud_api_module.h"

static const char* TAG = "cloud_api_json_parsing";
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
    cJSON_Delete(time_value);
    return ESP_OK;
}

esp_err_t cloud_api_parse_repeated_alarms(const char* jsonString)
{
    if(jsonString == NULL){
        ESP_LOGE(TAG,"jsonString is NULL.\n");
        return ESP_FAIL;
    }

    // Parse the JSON string
    cJSON* root = cJSON_Parse(jsonString);
    if (root == NULL) {
        printf("Error: Failed to parse JSON.\n");
        return ESP_FAIL;
    }

    // Get the "success" flag to validate the data.
    cJSON* success = cJSON_GetObjectItem(root, "success");
    if (cJSON_IsTrue(success)) {
        ESP_LOGI(TAG,"success == TRUE");
        return ESP_OK;
    }
    else
    {
        ESP_LOGI(TAG,"success == FALSE");
        cJSON_Delete(root);
        return ESP_FAIL;
    }

    cJSON_Delete(success);
    cJSON_Delete(root);
    return ESP_OK;
}