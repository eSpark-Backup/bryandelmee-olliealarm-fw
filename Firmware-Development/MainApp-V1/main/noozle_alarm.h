#pragma once

#include "noozle_factory_setup.h"
/* Deep Sleep */
#include "esp_timer.h"
#include "esp_sleep.h"

esp_err_t noozle_set_alarm();
esp_err_t noozle_get_alarm_epoch(int64_t* alarms);
void noozle_set_deep_sleep(int64_t alarm_epoch, alarm_name_t alarm_name);
esp_err_t noozle_set_alarm_sleep_time(int64_t* alarms);