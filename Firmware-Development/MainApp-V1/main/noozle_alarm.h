#pragma once

#include "noozle_factory_setup.h"

esp_err_t noozle_set_alarm();
esp_err_t noozle_get_alarm_epoch(int64_t* alarms);
esp_err_t noozle_set_alarm_sleep_time(int64_t* alarms);