#ifndef WIFI_MOUDLE_H
#define WIFI_MOUDLE_H

#include <stdio.h>
#include <string.h>
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "lwip/err.h"
#include "lwip/sys.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"

esp_err_t wifi_init(); //
esp_err_t wifi_create_event_group(); //
esp_err_t wifi_register_events();
esp_err_t wifi_sta_setup(wifi_config_t* wifi_config);
esp_err_t wifi_set_credentials(const char * ssid, const char * password );
esp_err_t wifi_connction_wait();

#endif