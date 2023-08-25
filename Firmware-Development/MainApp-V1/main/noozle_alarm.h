#pragma once

#include "noozle_factory_setup.h"
/* Deep Sleep */
#include "esp_timer.h"
#include "esp_sleep.h"

esp_err_t noozle_set_alarm();
esp_err_t noozle_get_alarm_epoch(int64_t* alarms);
void noozle_set_deep_sleep(int64_t wakeup_epoch);
esp_err_t noozle_set_alarm_sleep_time(int64_t* alarms);