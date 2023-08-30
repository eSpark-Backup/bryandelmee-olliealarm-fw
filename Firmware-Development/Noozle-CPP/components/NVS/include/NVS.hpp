#pragma once 

#include "nvs_flash.h"

class NVS
{
    private:
    nvs_handle_t handle;

    public:
    esp_err_t init();
    esp_err_t setupKeyU8(const char* name_space, const char* key);
    esp_err_t setupKeyU32(const char* name_space, const char* key);
    esp_err_t eraseKey(const char* name_space, const char* key);
    esp_err_t writeKeyU32(const char* name_space, const char* key, uint32_t value);
    esp_err_t readKeyU32(const char* name_space, const char* key, uint32_t* data);
    esp_err_t writeKeyU8(const char* name_space, const char* key, uint8_t value);
    esp_err_t readKeyU8(const char* name_space, const char* key, uint8_t* data);
    esp_err_t writeKey(const char* name_space, const char* key, void* value, size_t size);
    esp_err_t readKey(const char* name_space, const char* key, void* data);
};