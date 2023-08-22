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
    return ESP_OK;
}
extern uint32_t noozle_device_id_len;
esp_err_t cloud_api_parse_ollie_id(const char* jsonString, char** noozle_dev_id)
{
    esp_err_t ret = ESP_FAIL;
    // Parse the JSON string
    cJSON* root = cJSON_Parse(jsonString);
    if (root == NULL) {
        printf("Error: Failed to parse JSON.\n");
        return ESP_FAIL;
    }

    ret |= cloud_api_success_status(root);

    cJSON *data = cJSON_GetObjectItemCaseSensitive(root, "data");
    if (!cJSON_IsArray(data)) {
        fprintf(stderr, "Error: 'data' is not an array.\n");
        cJSON_Delete(root);
        return ESP_FAIL;
    }

    char *targetName = noozle_get_device_name();  // Noozle Device Name to fetch ID.

    // Iterate through the 'data' array to find the matching 'name'
    cJSON *item;
    cJSON_ArrayForEach(item, data) {
        cJSON *name = cJSON_GetObjectItemCaseSensitive(item, "name");
        if (cJSON_IsString(name) && (strcmp(name->valuestring, targetName) == 0)) {
            cJSON *id = cJSON_GetObjectItemCaseSensitive(item, "_id");
            if (cJSON_IsString(id)) {
                ESP_LOGI(TAG, "device_name: %s , id[%d] : %s", targetName, strlen(id->valuestring), id->valuestring);
                //Update noozle_device_id.
                if(*noozle_dev_id != NULL){
                    free(*noozle_dev_id);
                    *noozle_dev_id = NULL;
                }
                // Create the JSON string
                noozle_device_id_len = snprintf(NULL, 0, "{\"ollie_id\":\"%s\"}", id->valuestring);
                ESP_LOGI(TAG,"noozle_device_id_len = %lu", noozle_device_id_len);
                *noozle_dev_id = malloc(noozle_device_id_len+1); // +1 for null terminator
                snprintf(*noozle_dev_id, noozle_device_id_len+1, "{\"ollie_id\":\"%s\"}", id->valuestring);
                ESP_LOGI(TAG,"noozle_device_id : %s", *noozle_dev_id);
                ret = ESP_OK;
                // break;
            }
        }
            
    }
    // free(targetName);
    // targetName = NULL;
    cJSON_Delete(root);
    return ret;
}

esp_err_t cloud_api_parse_repeated_alarms(const char* jsonString)
{
    esp_err_t ret = ESP_OK;
    // Parse the JSON string
    cJSON* root = cJSON_Parse(jsonString);
    if (root == NULL) {
        printf("Error: Failed to parse JSON.\n");
        return ESP_FAIL;
    }

    ret |= cloud_api_success_status(root);

    cJSON_Delete(root);
    return ret;
}

esp_err_t cloud_api_success_status(cJSON* root)
{
    // Get the "success" flag to validate the data.
    cJSON* success = cJSON_GetObjectItem(root, "success");
    if (cJSON_IsTrue(success)) {
        ESP_LOGI(TAG,"success == TRUE");
        return ESP_OK;
    }
    else
    {
        ESP_LOGI(TAG,"success == FALSE");
        return ESP_FAIL;
    }
    cJSON_Delete(success);
    return ESP_FAIL;
}