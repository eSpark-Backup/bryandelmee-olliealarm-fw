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

void noozle_set_deep_sleep(int64_t alarm_epoch, alarm_name_t alarm_name)
{
    int64_t current_time = time(NULL);
    uint64_t sleep_time = alarm_epoch - current_time;
    sleep_time = (sleep_time-10) * 1000000;
    ESP_LOGI(TAG,"current_epoch = %lld", current_time);
    ESP_LOGI(TAG,"alarm_epoch = %lld", alarm_epoch);
    ESP_LOGI(TAG,"sleep_time = %llu", sleep_time);
    // ESP_LOGI(TAG,"sleep_time_new = %llu", (alarm_epoch - current_time) * 1000000);
    /*Saving the alarm name to check which alarm needs to trigger on wakeup*/
    nvs_open_namespace_rw(ALARM_NAMESPACE);
    nvs_write_key_value(ALARM_NAME, &alarm_name, sizeof(alarm_name_t));
    nvs_close_handle();

    esp_sleep_enable_timer_wakeup(sleep_time);
    esp_deep_sleep_start();
}

esp_err_t noozle_set_alarm_sleep_time(int64_t* alarms)
{
    esp_err_t ret = ESP_OK;
    int64_t current_time = time(NULL);
    int64_t dif0 = alarms[TO_SLEEP] - current_time;
    int64_t dif2 = alarms[TO_GET_UP] - current_time;

    if( dif0 > 0 )
    {
        if( dif0 < dif2 || dif2 < 0)
        {
            noozle_set_deep_sleep(alarms[TO_SLEEP], ALARM_SLEEP);
            ESP_LOGI(TAG,"SET SLEEP ALARM"); 
        }
            
    }
    if( dif2 > 0 )
    {
        if( dif2 < dif0 || dif0 < 0){
            ESP_LOGI(TAG,"SET WAKE ALARM");
            noozle_set_deep_sleep(alarms[TO_GET_UP], ALARM_WAKE);
        }
            
    }
    if( dif0 < 0 && dif2 < 0)
    {
        /* Setting ALARM_NONE if current time exceeds the alarm sleep & alarm wake time*/
        alarm_name_t alarm_name = ALARM_NONE;
        nvs_open_namespace_rw(ALARM_NAMESPACE);
        nvs_write_key_value(ALARM_NAME, &alarm_name, sizeof(alarm_name_t));
        nvs_close_handle();
        ESP_LOGI(TAG, "NO ALARM SET");
    }
    return ret;
}