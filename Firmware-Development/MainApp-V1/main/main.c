#include <stdio.h>
#include "lcd_app.h"

/* Deep Sleep */
#include "esp_timer.h"
#include "esp_sleep.h"
/* Cloud-API */
#include "cloud_api_module.h"

#include "noozle_factory_setup.h"
#include "noozle_alarm.h"

#include <time.h>

static const char *TAG = "MainAPP";
bool volatile alarm_Trigger = false;

void noozle_set_alarm_config_lcd(lcd_config_alarm_t* lcd_alarm_config ,int64_t* alarms, alarm_name_t alarm_name);

void app_main(void)
{
     esp_reset_reason_t rst_reason = esp_reset_reason();
     ESP_LOGI(TAG, "%d", rst_reason);

     noozle_device_initialization();
     noozle_factory_setup();

     alarm_name_t alarm_name; /*Variable to get the value of alarm name from nvs, to run the respective alarm*/
     switch (rst_reason)
     {
          case ESP_RST_POWERON:
               ESP_LOGI(TAG,"rst_reason : ESP_RST_POWERON");
               while(ESP_OK != noozle_cloud_service(NOOZLE_TIME_API));
               while(ESP_OK != noozle_cloud_service(NOOZLE_GET_ID_API));
               while(ESP_OK != noozle_cloud_service(NOOZLE_REPEATED_ALARM_API));
               noozle_set_alarm();
               break;

          case ESP_RST_DEEPSLEEP:
               ESP_LOGI(TAG,"rst_reason : ESP_RST_DEEPSLEEP");
               nvs_open_namespace_rw(ALARM_NAMESPACE);
               nvs_read_key_value(ALARM_NAME, &alarm_name);
               nvs_close_handle();
               ESP_LOGI(TAG,"alarm_name : %d", alarm_name);
               int64_t alarms[4];
               noozle_get_alarm_epoch(alarms);
               lcd_config_alarm_t lcd_alarm_config;
               noozle_set_alarm_config_lcd(&lcd_alarm_config, alarms, alarm_name);
               if(alarm_name != ALARM_NONE){
                    alarm_Trigger = false;
                    lcd_create_task(&lcd_alarm_config);
               }

               break;

          default:
          break;
     }

     int64_t epoch = 0;

     while (1)
     {
          if(alarm_Trigger)
               noozle_set_alarm();
          epoch = time(NULL);
          ESP_LOGI(TAG,"%lld", epoch);
          ESP_LOGI(TAG,"%d",alarm_name);
          vTaskDelay(500 / portTICK_PERIOD_MS);
     }
}

void noozle_set_alarm_config_lcd(lcd_config_alarm_t* lcd_alarm_config ,int64_t* alarms, alarm_name_t alarm_name)
{
     if(alarm_name == ALARM_SLEEP)
     {
          lcd_alarm_config->start_time = alarms[TO_SLEEP];
          lcd_alarm_config->end_time = alarms[ALMOST_SLEEP];
          lcd_alarm_config->alarm_name = alarm_name; 
          ESP_LOGI(TAG,"ALARM_SLEEP is configured in lcd_config_alarm_t");
     }
     else if(alarm_name == ALARM_WAKE)
     {
          lcd_alarm_config->start_time = alarms[TO_GET_UP];
          lcd_alarm_config->end_time = alarms[ALMOST_GETTING_UP];
          lcd_alarm_config->alarm_name = alarm_name; 
          ESP_LOGI(TAG,"ALARM_WAKE is configured in lcd_config_alarm_t");
     }
}