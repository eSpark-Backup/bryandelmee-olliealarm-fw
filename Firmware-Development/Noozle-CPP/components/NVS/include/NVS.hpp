#pragma once 

#include "nvs_flash.h"

class NVS
{
    public:
    esp_err_t init();
    esp_err_t setupKeyU8(const char* space_label, const char* key);
    esp_err_t setupKeyU32(const char* space_label, const char* key);
    esp_err_t eraseKey(const char* space_label, const char* key);
    esp_err_t writeKeyU32(const char* space_label, const char* key, uint32_t value);
    esp_err_t readKeyU32(const char* space_label, const char* key, uint32_t* value);
    esp_err_t writeKeyU8(const char* space_label, const char* key, uint8_t value);
    esp_err_t readKeyU8(const char* space_label, const char* key, uint8_t* value);
    esp_err_t writeKey(const char* space_label, const char* key, void* value, size_t size);
    esp_err_t readKey(const char* space_label, const char* key, void* value);

    private:
    nvs_handle_t handle;
};