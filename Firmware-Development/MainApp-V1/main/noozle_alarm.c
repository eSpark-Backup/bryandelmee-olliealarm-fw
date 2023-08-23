#include "noozle_alarm.h"

static const char* TAG = "NOOZLE_ALARM";

esp_err_t noozle_set_alarm()
{
    esp_err_t ret = ESP_OK;
    int64_t alarms[4];
    ret |= noozle_get_alarm_epoch(alarms);
    ret |= noozle_set_alarm_sleep_time(alarms);
    return ret;
}

esp_err_t noozle_get_alarm_epoch(int64_t* alarms)
{
    esp_err_t ret = ESP_OK;
    ret |= nvs_open_namespace_rw(ALARM_NAMESPACE);
    ret |= nvs_read_key_value(ALARM_KEY, alarms);
    nvs_close_handle();

    return ret;
}

esp_err_t noozle_set_alarm_sleep_time(int64_t* alarms)
{
    esp_err_t ret = ESP_OK;
    int64_t current_time = time(NULL);
    if( (current_time < alarms[TO_SLEEP] || current_time < alarms[ALMOST_SLEEP]) && (alarms[TO_SLEEP] < alarms[TO_GET_UP]) )
        ESP_LOGI(TAG,"SET SLEEP ALARM");
    else
        ESP_LOGI(TAG,"SET WAKE ALARM");
    return ret;
}