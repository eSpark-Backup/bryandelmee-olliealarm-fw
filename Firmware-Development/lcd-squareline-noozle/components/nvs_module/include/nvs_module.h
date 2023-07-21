#ifndef NVS_MOUDLE_H
#define NVS_MOUDLE_H

#include "nvs_flash.h"

esp_err_t nvs_init();
esp_err_t nvs_update();
void nvs_close_handle();
esp_err_t nvs_open_namespace_rw(const char* namespace);
esp_err_t nvs_erase_key_value(const char *key_name);
esp_err_t nvs_write_key_value(const char* key, void *value, size_t size);
esp_err_t nvs_read_key_value(const char* key, void *data);

#endif