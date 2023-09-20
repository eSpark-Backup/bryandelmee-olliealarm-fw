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

    char *targetName = "OLLIE123";  // Noozle Device Name to fetch ID.

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
    int64_t alarm_epoch[4];
    // Parse the JSON string
    cJSON* root = cJSON_Parse(jsonString);
    if (root == NULL) {
        printf("Error: Failed to parse JSON.\n");
        return ESP_FAIL;
    }

    ret |= cloud_api_success_status(root);
    ret |= cloud_api_parse_repeated_alarms_epoch(root, alarm_epoch);
    ret |= save_alarms_in_nvs(alarm_epoch, sizeof(alarm_epoch));
    
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

esp_err_t cloud_api_parse_repeated_alarms_epoch(cJSON* root, int64_t* alarm_epoch)
{
    cJSON *data = cJSON_GetObjectItemCaseSensitive(root, "data");
    
    if (data == NULL || !cJSON_IsArray(data)) {
        printf("Data field not found or is not an array.\n");
        return ESP_FAIL;
    }
    
    int data_size = cJSON_GetArraySize(data);
    
    for (int i = 0; i < data_size; i++) {
        cJSON *scene = cJSON_GetArrayItem(data, i);
        
        cJSON *scene_name = cJSON_GetObjectItemCaseSensitive(scene, "scene_name");
        printf("Scene: %s\n", scene_name->valuestring);
        
        cJSON *scene_sleep_time = cJSON_GetObjectItemCaseSensitive(scene, "scene_sleep_time");
        
        if (scene_sleep_time == NULL || !cJSON_IsArray(scene_sleep_time)) {
            printf("Scene sleep time field not found or is not an array.\n");
            continue;
        }
        
        int sleep_time_size = cJSON_GetArraySize(scene_sleep_time);
        
        for (int j = 0; j < sleep_time_size; j++) {
            cJSON *sleep_time_item = cJSON_GetArrayItem(scene_sleep_time, j);
            
            cJSON *time = cJSON_GetObjectItemCaseSensitive(sleep_time_item, "time");
            cJSON *name = cJSON_GetObjectItemCaseSensitive(sleep_time_item, "scene_sleep_time_name");
            cJSON *brightness = cJSON_GetObjectItemCaseSensitive(sleep_time_item, "brightness");
            
            printf("Time: %d\n", time->valueint);
            printf("Scene Sleep Time Name: %s\n", name->valuestring);
            printf("Brightness: %lf\n", brightness->valuedouble);
            
            alarm_epoch[j] = time->valueint;
            printf("\n");
        }
    }
    return ESP_OK;
}

esp_err_t save_alarms_in_nvs(int64_t* alarm_epoch, size_t size)
{
    nvs_handle_t handle;
    nvs_open(ALARM_NAMESPACE, NVS_READWRITE, &handle);
    // nvs_open_namespace_rw(ALARM_NAMESPACE);
    esp_err_t err = nvs_set_u32(handle, ALARM_KEY, alarm_epoch);
    nvs_close(handle);
    // NVS cloudNVS;
    // cloudNVS.init();
    // cloutNVS.writeKeyu32(ALARM_NAMESPACE, ALARM_KEY, (uint32_t)size);
    return ESP_OK;
}