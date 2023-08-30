#include "NVS.hpp"
#include "esp_check.h"

static const char* TAG = "NVS";

/**
 * @brief Initialize NVS partition
 * @return esp_err_t  
 *          ESP_OK if storage was successfully initialized.
 *          ESP_ERR_NVS_NO_FREE_PAGES if the NVS storage contains no empty pages (which may happen if NVS partition was truncated)
 *          ESP_ERR_NOT_FOUND if no partition with label “nvs” is found in the partition table
 *          ESP_ERR_NO_MEM in case memory could not be allocated for the internal structures
 */    
esp_err_t NVS::init()
{
    return nvs_flash_init();
}

esp_err_t NVS::setupKeyU8(const char* name_space, const char* key)
{
    uint8_t value = 0;
    esp_err_t ret;
    ret = readKeyU8(name_space, key, &value);

    if(ret == ESP_ERR_NVS_NOT_FOUND)
        ESP_RETURN_ON_ERROR(writeKeyU8(name_space, key, value), TAG, "setupKeyU8 : writeKeyU8");
    
    return ESP_OK;
}

esp_err_t NVS::setupKeyU32(const char* name_space, const char* key)
{
    uint32_t value = 0;
    esp_err_t ret;
    ret = readKeyU32(name_space, key, &value);

    if(ret == ESP_ERR_NVS_NOT_FOUND)
        ESP_RETURN_ON_ERROR(writeKeyU32(name_space, key, value), TAG, "setupKeyU32 : writeKeyU32");
    
    return ESP_OK;
}

/**
 * @brief Erase key-value pair with given key name & update the storage.
 * 
 * @param name_space Open NVS storage with a given name_space from the default NVS partition.
 * @param key       Name of the Key to be erased
 * 
 * @return esp_err_t 
 *          ESP_OK if erase operation was successful
 *          ESP_FAIL if there is an internal error; most likely due to corrupted NVS partition (only if NVS assertion checks are disabled)
 *          ESP_ERR_NVS_INVALID_HANDLE if handle has been closed or is NULL
 *          ESP_ERR_NVS_READ_ONLY if handle was opened as read only
 *          ESP_ERR_NVS_NOT_FOUND if the requested key doesn’t exist
 */

esp_err_t NVS::eraseKey(const char* name_space, const char* key)
{
    ESP_RETURN_ON_ERROR(nvs_open(name_space, NVS_READWRITE, &handle), TAG, "eraseKey: nvs_open");
    ESP_RETURN_ON_ERROR(nvs_erase_key(handle, key), TAG, "eraseKey: nvs_erase_key");
    ESP_RETURN_ON_ERROR(nvs_commit(handle), TAG, "eraseKey: nvs_commit");
    nvs_close(handle);
    return ESP_OK;
}

/**
 * @brief Write an Unsigned 32-bit Key-Value pair to the NVS storage.
 * 
 * @param name_space Open NVS storage with a given name_space from the default NVS partition.
 * @param key   Key name
 * @param value Value to be stored
 * 
 * @return esp_err_t ESP_OK/ESP_FAIL
 */

esp_err_t NVS::writeKeyU32(const char* name_space, const char* key, uint32_t value)
{
    ESP_RETURN_ON_ERROR(nvs_open(name_space, NVS_READWRITE, &handle), TAG, "writeKeyU32: nvs_open");
    ESP_RETURN_ON_ERROR(nvs_set_u32(handle, key, value), TAG, "writeKeyU32: nvs_set_u8");
    ESP_RETURN_ON_ERROR(nvs_commit(handle), TAG, "writeKeyU32: nvs_commit");
    nvs_close(handle);
    return ESP_OK;
}

/**
 * @brief Read an Unsigned 32-bit Key-Value pair to the NVS storage.
 * 
 * @param name_space Open NVS storage with a given name_space from the default NVS partition.
 * @param key   Key-name
 * @param data  Variable to stored the retrieved data
 * 
 * @return esp_err_t ESP_OK/ESP_FAIL
 */

esp_err_t NVS::readKeyU32(const char* name_space, const char* key, uint32_t* data)
{
    ESP_RETURN_ON_ERROR(nvs_open(name_space, NVS_READWRITE, &handle), TAG, "readKey32: nvs_open");
    ESP_RETURN_ON_ERROR(nvs_get_u32(handle, key, data), TAG, "readKey32: nvs_set_u8");
    ESP_RETURN_ON_ERROR(nvs_commit(handle), TAG, "readKey32: nvs_commit");
    nvs_close(handle);
    return ESP_OK;
}

/**
 * @brief Write an Unsigned 8-bit Key-Value pair to the NVS storage.
 * 
 * @param name_space Open NVS storage with a given name_space from the default NVS partition.
 * @param key   Key name
 * @param value Value to be stored
 * 
 * @return esp_err_t ESP_OK/ESP_FAIL
 */
esp_err_t NVS::writeKeyU8(const char* name_space, const char* key, uint8_t value)
{
    ESP_RETURN_ON_ERROR(nvs_open(name_space, NVS_READWRITE, &handle), TAG, "writeKeyU8: nvs_open");
    ESP_RETURN_ON_ERROR(nvs_set_u8(handle, key, value), TAG, "writeKeyU8: nvs_set_u8");
    ESP_RETURN_ON_ERROR(nvs_commit(handle), TAG, "writeKeyU8: nvs_commit");
    nvs_close(handle);
    return ESP_OK;
}

/**
 * @brief Read an Unsigned 8-bit Key-Value pair to the NVS storage.
 * 
 * @param name_space Open NVS storage with a given name_space from the default NVS partition.
 * @param key   Key-name
 * @param data  Variable to stored the retrieved data
 * 
 * @return esp_err_t ESP_OK/ESP_FAIL
 */
esp_err_t NVS::readKeyU8(const char* name_space, const char* key, uint8_t* data)
{
    ESP_RETURN_ON_ERROR(nvs_open(name_space, NVS_READWRITE, &handle), TAG, "readKeyU8: nvs_open");
    ESP_RETURN_ON_ERROR(nvs_get_u8(handle, key, data), TAG, "readKeyU8: nvs_get_u8");
    ESP_RETURN_ON_ERROR(nvs_commit(handle), TAG, "readKeyU8: nvs_commit");
    nvs_close(handle);
    return ESP_OK;
}


/**
 * @brief Write Key-Value pair to the NVS storage.
 * 
 * @param name_space Open NVS storage with a given name_space from the default NVS partition.
 * @param key   Key name
 * @param value Value to be stored
 * @param size  Size of the value.
 * @return esp_err_t ESP_OK/ESP_FAIL
 */
esp_err_t NVS::writeKey(const char* name_space, const char* key, void* value, size_t size)
{
    ESP_RETURN_ON_ERROR(nvs_open(name_space, NVS_READWRITE, &handle), TAG, "writeKey: nvs_open");
    ESP_RETURN_ON_ERROR(nvs_set_blob(handle, key, value, size), TAG, "writeKey: nvs_set_blob");
    ESP_RETURN_ON_ERROR(nvs_commit(handle), TAG, "writeKey: nvs_commit");
    nvs_close(handle);
    return ESP_OK;
}

/**
 * @brief Read Key-Value pair from the NVS storage.
 * 
 * @param name_space Open NVS storage with a given name_space from the default NVS partition.
 * @param key   Key-name
 * @param data  Variable to stored the retrieved data
 * 
 * @return esp_err_t ESP_OK/ESP_FAIL
 */
esp_err_t NVS::readKey(const char* name_space, const char* key, void* data)
{
    size_t size;
    ESP_RETURN_ON_ERROR(nvs_open(name_space, NVS_READWRITE, &handle), TAG, "readKey: nvs_open");
    ESP_RETURN_ON_ERROR(nvs_get_blob(handle, key, data, &size), TAG, "readKey: nvs_get_blob");
    ESP_RETURN_ON_ERROR(nvs_commit(handle), TAG, "readKey: nvs_commit");
    nvs_close(handle);
    return ESP_OK;
}