#include "nvs_module.h"

nvs_handle_t nvsHandle;

/**
 * @brief Initialize NVS partition
 * @return esp_err_t  
 *          ESP_OK if storage was successfully initialized.
 *          ESP_ERR_NVS_NO_FREE_PAGES if the NVS storage contains no empty pages (which may happen if NVS partition was truncated)
 *          ESP_ERR_NOT_FOUND if no partition with label “nvs” is found in the partition table
 *          ESP_ERR_NO_MEM in case memory could not be allocated for the internal structures
 */    
esp_err_t nvs_init()
{
    return nvs_flash_init();
}

/**
 * @brief Update the pending changes to NVS storage.
 * 
 * @return esp_err_t 
 *              ESP_OK if the changes have been written successfully
 *              ESP_ERR_NVS_INVALID_HANDLE if handle has been closed or is NULL
 */

esp_err_t nvs_update()
{
    return nvs_commit(nvsHandle);
}

/**
 * @brief Close the NVS storage handle and free allocated resources.
 */
void nvs_close_handle()
{
    nvs_close(nvsHandle);
}
/**
 * @brief Open NVS storage with a given namespace from the default NVS partition.
 * 
 * @param namespace 
 * @return esp_err_t ESP_OK/ESP_FAIL
 */
esp_err_t nvs_open_namespace_rw(const char* namespace)
{
    return nvs_open(namespace, NVS_READWRITE, &nvsHandle);
}


/**
 * @brief Erase key-value pair with given key name & update the storage.
 * 
 * @param key_name Name of the Key to be erased
 * @return esp_err_t 
 *          ESP_OK if erase operation was successful
 *          ESP_FAIL if there is an internal error; most likely due to corrupted NVS partition (only if NVS assertion checks are disabled)
 *          ESP_ERR_NVS_INVALID_HANDLE if handle has been closed or is NULL
 *          ESP_ERR_NVS_READ_ONLY if handle was opened as read only
 *          ESP_ERR_NVS_NOT_FOUND if the requested key doesn’t exist
 */

esp_err_t nvs_erase_key_value(const char *key_name)
{
    esp_err_t ret = nvs_erase_key(nvsHandle, key_name);
    if(ret != ESP_OK)
        return ret;

    ret = nvs_update();
    return ret;
}

/**
 * @brief Wrie Key-Value pair to the NVS storage.
 * 
 * @param key   Key name
 * @param value Value to be stored
 * @param size  Size of the value.
 * @return esp_err_t ESP_OK/ESP_FAIL
 */
esp_err_t nvs_write_key_value(const char* key, void *value, size_t size)
{
    esp_err_t ret = nvs_set_blob(nvsHandle, key, value, size);
    if(ret != ESP_OK)
        return ret;

    ret |= nvs_update();
    return ret;
}

/**
 * @brief 
 * 
 * @param key   Key-name
 * @param data  Variable to stored the retrieved data
 * @param size  
 * @return esp_err_t ESP_OK/ESP_FAIL
 */
esp_err_t nvs_read_key_value(const char* key, void *data)
{
    size_t size;
    return nvs_get_blob(nvsHandle, key, data, &size); 
}

/**
 * @brief Wrie Key-Value pair to the NVS storage.
 * 
 * @param key   Key name
 * @param value Value to be stored
 * @param size  Size of the value.
 * @return esp_err_t ESP_OK/ESP_FAIL
 */
esp_err_t nvs_write_key_value_u8(const char* key, uint8_t value)
{
    esp_err_t ret = nvs_set_u8(nvsHandle, key, value);
    if(ret != ESP_OK)
        return ret;

    ret |= nvs_update();
    return ret;
}

/**
 * @brief 
 * 
 * @param key   Key-name
 * @param data  Variable to stored the retrieved data
 * @param size  
 * @return esp_err_t ESP_OK/ESP_FAIL
 */
esp_err_t nvs_read_key_value_u8(const char* key, uint8_t *data)
{
    return nvs_get_u8(nvsHandle, key, data); 
}
