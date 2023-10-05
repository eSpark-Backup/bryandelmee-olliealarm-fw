#include "cloud_api_module.h"

char* sleep_scene_string[] = {
    "Almost Asleep", 
    "To Sleep", 
    "Almost Getting", 
    "To get up"
    };

char* sleep_scene_string_brightness[] = {
    "bAlmost Asleep", 
    "bTo Sleep", 
    "bAlmost Getting", 
    "bTo get up"
    };

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
esp_err_t cloud_api_parse_ollie_id(const char* jsonString, char** noozle_dev_id, char** noozle_batt_json)
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

    char service_name[12];

    uint8_t eth_mac[6];
    const char *ssid_prefix = "PROV_";
    ret = esp_wifi_get_mac(WIFI_IF_STA, eth_mac);   /* esp_wifi_init() must be called before, else it'll return an error.  */
    ESP_LOGI(TAG, "Service : %s%02X%02X%02X", ssid_prefix, eth_mac[3], eth_mac[4], eth_mac[5]);
    /* Make a string from ssid_prefix & MAC-Address index 3,4,5 & sets it in the service name */
    snprintf((const char*)service_name, sizeof(service_name), "%s%02X%02X%02X",
             ssid_prefix, eth_mac[3], eth_mac[4], eth_mac[5]);

    char *targetName = (char *)malloc(sizeof(service_name));
    strcpy(targetName,(const char*)service_name);
    ESP_LOGI(TAG, "device_name: %s" , targetName);


    // Iterate through the 'data' array to find the matching 'name'
    cJSON *item;
    cJSON_ArrayForEach(item, data) {
        cJSON *name = cJSON_GetObjectItemCaseSensitive(item, "name");
        if (cJSON_IsString(name) && (strcmp(name->valuestring, targetName) == 0)) {
            cJSON *id = cJSON_GetObjectItemCaseSensitive(item, "_id");
            if (cJSON_IsString(id)) {  
                    ESP_LOGI(TAG, "device_name: %s" , targetName);
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
    free(targetName);
    targetName = NULL;
    cJSON_Delete(root);
    return ret;
}

esp_err_t cloud_api_create_battery_status_json(char** noozle_dev_id, char** noozle_batt_json){
    //create battery update json
    printf("NOOZLEP POINTER ADDR = %p\n", noozle_dev_id);
    cJSON *battery_root = cJSON_Parse(*noozle_dev_id);   /* causing guru med when alarm ringing end and comes to update battery status*/
    char *out;
    // battery_root = cJSON_CreateObject();
    // cJSON_AddItemToObject(battery_root, "ollie_id", cJSON_CreateString(id->valuestring));
    cJSON_AddItemToObject(battery_root, "status", cJSON_CreateString("Charging"));
    cJSON_AddItemToObject(battery_root, "percentage", cJSON_CreateNumber(96));

    out = cJSON_Print(battery_root);
    // printf("%s\n",out);

    *noozle_batt_json = out;
    printf("%s\n",*noozle_batt_json);
    cJSON_Delete(battery_root); 
    return ESP_OK;
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
    nvs_flash_init();
    nvs_handle_t handle;
    
    if (data == NULL || !cJSON_IsArray(data)) {
        printf("Data field not found or is not an array.\n");
        // for(int i = 0; i<4; i++){  //set nvs alarms time to 0 and week days to 0
        //     nvs_handle_t handle;
        //     ESP_RETURN_ON_ERROR(nvs_open(sleep_scene_string[i], NVS_READWRITE, &handle), TAG, "opening NVS Namespace of sleep_time");
        //     ESP_RETURN_ON_ERROR(nvs_set_u32(handle, "time" , 0), TAG, "writing time epoch 0");
        //     ESP_RETURN_ON_ERROR(nvs_set_u32(handle, "brightness" , 0), TAG, "writing brightness value 0");

        //     ESP_RETURN_ON_ERROR(nvs_open(sleep_scene_string[i], NVS_READWRITE, &handle), TAG, "opening NVS Namespace of weekdays");
        //     ESP_RETURN_ON_ERROR(nvs_set_u8(handle, "weekday_bits" , 0), TAG, "writing weekday_bits epoch");
        //     ESP_RETURN_ON_ERROR(nvs_commit(handle), TAG, "Updated NVS\n");

        //     ESP_RETURN_ON_ERROR(nvs_commit(handle), TAG, "NVS data reset\n");
        //     nvs_close(handle);
        // }
        nvs_open("Scene1", NVS_READWRITE, &handle);
        nvs_erase_all(handle);
        nvs_commit(handle);
        nvs_close(handle);
        return ESP_FAIL;
    }
    
    int data_size = cJSON_GetArraySize(data);
    
    for (int8_t i = 0; i < data_size; i++) {
        cJSON *scene = cJSON_GetArrayItem(data, i);
        
        cJSON *scene_name = cJSON_GetObjectItemCaseSensitive(scene, "scene_name");
        printf("Scene: %s\n", scene_name->valuestring);

        char array[65] = "scene";
        // sprintf(array,"scene %d",i);
        snprintf(array, 18,"scene %d",i);
        ESP_LOGI(TAG, "sprintf : %s", array);

        //Saving Scene name in NVS with namespace as "scene_name"
        ESP_RETURN_ON_ERROR(nvs_open("scene_names", NVS_READWRITE, &handle), TAG, "opening NVS Namespace of scene_name");
        ESP_RETURN_ON_ERROR(nvs_set_u32(handle, "no_scenes" , data_size), TAG, "no. of scenes");
        ESP_RETURN_ON_ERROR(nvs_set_str(handle, array, scene_name->valuestring ), TAG, "name of the scene");

        //reading from NVS just to check that we are saving correct or not
        int value;
        ESP_RETURN_ON_ERROR(nvs_get_u32(handle, "no_scenes", &value), TAG, "readKey32: Read U32 key-value from NVS.");
        size_t required_size = 0;
        ESP_RETURN_ON_ERROR(nvs_get_str(handle, array,NULL,&required_size), TAG, "readKey32: Read U32 key-value from NVS.");
        char* server_name = malloc(required_size);
        ESP_RETURN_ON_ERROR(nvs_get_str(handle, array, server_name,&required_size), TAG, "readKey32: Read U32 key-value from NVS.");
        ESP_RETURN_ON_ERROR(nvs_commit(handle), TAG, "readKey32: Update NVS-Storage");
        nvs_close(handle);
        ESP_LOGI(TAG, "read no of scenes : %d", value);
        ESP_LOGI(TAG, "read name of scene : %s", server_name);

        cJSON *days = cJSON_GetObjectItemCaseSensitive(scene, "days");
        cJSON *day = NULL;
        int8_t weekday_bits = 0b00000000;
        cJSON_ArrayForEach(day, days)
        {
            printf("day: %s\n", day->valuestring);
            
            if(strcmp(day->valuestring, "Sun") == 0){
                weekday_bits |= 0b01000000;
            }
            else if(strcmp(day->valuestring, "Mon") == 0){
                weekday_bits |= 0b00100000;
            }                
            else if(strcmp(day->valuestring, "Tue") == 0){
                weekday_bits |= 0b00010000;
            }                
            else if(strcmp(day->valuestring, "Wed") == 0){
                weekday_bits |= 0b00001000;
            }                
            else if(strcmp(day->valuestring, "Thu") == 0){
                weekday_bits |= 0b00000100;
            }
            else if(strcmp(day->valuestring, "Fri") == 0){
                weekday_bits |= 0b00000010;
            }
            else if(strcmp(day->valuestring, "Sat") == 0){
                weekday_bits |= 0b00000001;
            }

            nvs_handle_t handle;
            ESP_RETURN_ON_ERROR(nvs_open(scene_name->valuestring, NVS_READWRITE, &handle), TAG, "opening NVS Namespace as sceneName");
            ESP_RETURN_ON_ERROR(nvs_set_u8(handle, "weekday_bits" , weekday_bits), TAG, "writing weekday_bits epoch");
            ESP_RETURN_ON_ERROR(nvs_commit(handle), TAG, "Updated NVS\n");
            nvs_close(handle);
                
        }
        printf("weekday_bits %d: ", weekday_bits);
        
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
            
            nvs_handle_t handle;
            ESP_RETURN_ON_ERROR(nvs_open(scene_name->valuestring, NVS_READWRITE, &handle), TAG, "opening NVS Namespace of sleep_time");
            ESP_RETURN_ON_ERROR(nvs_set_u32(handle, sleep_scene_string[j] , time->valueint), TAG, "writing time epoch");
            ESP_RETURN_ON_ERROR(nvs_set_u32(handle, sleep_scene_string_brightness[j] , brightness->valuedouble), TAG, "writing brightness value (brightness+scene_sleep_string[j])");
            ESP_RETURN_ON_ERROR(nvs_commit(handle), TAG, "Updated NVS\n");
            nvs_close(handle);

            
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
    esp_err_t err = nvs_set_u32(handle, ALARM_KEY, alarm_epoch);
    nvs_close(handle);
    return ESP_OK;
}