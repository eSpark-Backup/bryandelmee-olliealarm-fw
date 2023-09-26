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

esp_err_t NVS::setupKeyU8(const char* space_label, const char* key)
{
    uint8_t value;
    esp_err_t ret;
    ret = readKeyU8(space_label, key, &value);

    /* 
        readKey function is used here to verify that the 'KEY' is present in the nvs-namespace.
        So, if 'KEY' is already present then it simply returns ESP_OK.
        But, if 'KEY' is not present, we have to define it in the nvs-namespace by simply writing it with a default-value 0.
    */
    if(ret == ESP_ERR_NVS_NOT_FOUND)
    {
        value = 0;
        ESP_RETURN_ON_ERROR(writeKeyU8(space_label, key, value), TAG, "setupKeyU8 : Writes an U8 Key-value");
    }
    
    return ESP_OK;
}

esp_err_t NVS::setupKeyU32(const char* space_label, const char* key)
{
    uint32_t value;
    esp_err_t ret;
    ret = readKeyU32(space_label, key, &value);

    /* 
        readKey function is used here to verify that the 'KEY' is present in the nvs-namespace.
        So, if 'KEY' is already present then it simply returns ESP_OK.
        But, if 'KEY' is not present, we have to define it in the nvs-namespace by simply writing it with a default-value 0.
    */
    if(ret == ESP_ERR_NVS_NOT_FOUND)
    {
        value = 0; 
        ESP_RETURN_ON_ERROR(writeKeyU32(space_label, key, value), TAG, "setupKeyU32 : Writes a U32 Key-value");
    }
    
    return ESP_OK;
}

/**
 * @brief Erase key-value pair with given key name & update the storage.
 * 
 * @param space_label Open NVS storage with a given space_label from the default NVS partition.
 * @param key       Name of the Key to be erased
 * 
 * @return esp_err_t 
 *          ESP_OK if erase operation was successful
 *          ESP_FAIL if there is an internal error; most likely due to corrupted NVS partition (only if NVS assertion checks are disabled)
 *          ESP_ERR_NVS_INVALID_HANDLE if handle has been closed or is NULL
 *          ESP_ERR_NVS_READ_ONLY if handle was opened as read only
 *          ESP_ERR_NVS_NOT_FOUND if the requested key doesn’t exist
 */

esp_err_t NVS::eraseKey(const char* space_label, const char* key)
{
    ESP_RETURN_ON_ERROR(nvs_open(space_label, NVS_READWRITE, &handle), TAG, "eraseKey: Open NVS with the given space_label");
    ESP_RETURN_ON_ERROR(nvs_erase_key(handle, key), TAG, "eraseKey: Erase the key");
    ESP_RETURN_ON_ERROR(nvs_commit(handle), TAG, "eraseKey: Update NVS-storage");
    nvs_close(handle);
    return ESP_OK;
}

/**
 * @brief Write an Unsigned 32-bit Key-Value pair to the NVS storage.
 * 
 * @param space_label Open NVS storage with a given space_label from the default NVS partition.
 * @param key   Key name
 * @param value Value to be stored
 * 
 * @return esp_err_t ESP_OK/ESP_FAIL
 */

esp_err_t NVS::writeKeyU32(const char* space_label, const char* key, uint32_t value)
{
    ESP_RETURN_ON_ERROR(nvs_open(space_label, NVS_READWRITE, &handle), TAG, "writeKeyU32: Open NVS with the given space_label");
    ESP_RETURN_ON_ERROR(nvs_set_u32(handle, key, value), TAG, "writeKeyU32: Writes a U32 Key-Value");
    ESP_RETURN_ON_ERROR(nvs_commit(handle), TAG, "writeKeyU32: Update NVS-Storage");
    nvs_close(handle);
    return ESP_OK;
}

/**
 * @brief Read an Unsigned 32-bit Key-Value pair to the NVS storage.
 * 
 * @param space_label Open NVS storage with a given space_label from the default NVS partition.
 * @param key   Key-name
 * @param value  Variable to stored the retrieved value
 * 
 * @return esp_err_t ESP_OK/ESP_FAIL
 */

esp_err_t NVS::readKeyU32(const char* space_label, const char* key, uint32_t* value)
{
    ESP_RETURN_ON_ERROR(nvs_open(space_label, NVS_READWRITE, &handle), TAG, "readKey32: Open NVS with the given space_label");
    ESP_RETURN_ON_ERROR(nvs_get_u32(handle, key, value), TAG, "readKey32: Read U32 key-value from NVS.");
    ESP_RETURN_ON_ERROR(nvs_commit(handle), TAG, "readKey32: Update NVS-Storage");
    nvs_close(handle);
    return ESP_OK;
}

/**
 * @brief Write an Unsigned 8-bit Key-Value pair to the NVS storage.
 * 
 * @param space_label Open NVS storage with a given space_label from the default NVS partition.
 * @param key   Key name
 * @param value Value to be stored
 * 
 * @return esp_err_t ESP_OK/ESP_FAIL
 */
esp_err_t NVS::writeKeyU8(const char* space_label, const char* key, uint8_t value)
{
    ESP_RETURN_ON_ERROR(nvs_open(space_label, NVS_READWRITE, &handle), TAG, "writeKeyU8: Open NVS with the given space_label");
    ESP_RETURN_ON_ERROR(nvs_set_u8(handle, key, value), TAG, "writeKeyU8: Writes U8 Key-value");
    ESP_RETURN_ON_ERROR(nvs_commit(handle), TAG, "writeKeyU8: Update NVS-Storage");
    nvs_close(handle);
    return ESP_OK;
}

/**
 * @brief Read an Unsigned 8-bit Key-Value pair to the NVS storage.
 * 
 * @param space_label Open NVS storage with a given space_label from the default NVS partition.
 * @param key   Key-name
 * @param value  Variable to stored the retrieved value
 * 
 * @return esp_err_t ESP_OK/ESP_FAIL
 */
esp_err_t NVS::readKeyU8(const char* space_label, const char* key, uint8_t* value)
{
    ESP_RETURN_ON_ERROR(nvs_open(space_label, NVS_READWRITE, &handle), TAG, "readKeyU8: Open NVS with the given space_label");
    ESP_RETURN_ON_ERROR(nvs_get_u8(handle, key, value), TAG, "readKeyU8: Reads U8 key-value");
    ESP_RETURN_ON_ERROR(nvs_commit(handle), TAG, "readKeyU8: Update NVS-Storage");
    nvs_close(handle);
    return ESP_OK;
}


/**
 * @brief Write Key-Value pair to the NVS storage.
 * 
 * @param space_label Open NVS storage with a given space_label from the default NVS partition.
 * @param key   Key name
 * @param value Value to be stored
 * @param size  Size of the value.
 * @return esp_err_t ESP_OK/ESP_FAIL
 */
esp_err_t NVS::writeKey(const char* space_label, const char* key, void* value, size_t size)
{
    ESP_RETURN_ON_ERROR(nvs_open(space_label, NVS_READWRITE, &handle), TAG, "writeKey: Open NVS with the given space_label");
    ESP_RETURN_ON_ERROR(nvs_set_blob(handle, key, value, size), TAG, "writeKey: Writes a blob Key-value");
    ESP_RETURN_ON_ERROR(nvs_commit(handle), TAG, "writeKey: Update NVS-Storage");
    nvs_close(handle);
    return ESP_OK;
}

/**
 * @brief Read Key-Value pair from the NVS storage.
 * 
 * @param space_label Open NVS storage with a given space_label from the default NVS partition.
 * @param key   Key-name
 * @param value  Variable to stored the retrieved value
 * 
 * @return esp_err_t ESP_OK/ESP_FAIL
 */
esp_err_t NVS::readKey(const char* space_label, const char* key, void* value)
{
    size_t size;
    ESP_RETURN_ON_ERROR(nvs_open(space_label, NVS_READWRITE, &handle), TAG, "readKey: Open NVS with the given space_label");
    ESP_RETURN_ON_ERROR(nvs_get_blob(handle, key, value, &size), TAG, "readKey: Reads a blob Key-value");
    ESP_RETURN_ON_ERROR(nvs_commit(handle), TAG, "readKey: Update NVS-Storage");
    nvs_close(handle);
    return ESP_OK;
}