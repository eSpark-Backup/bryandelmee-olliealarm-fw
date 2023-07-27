#include <stdio.h>
/* WiFi / BLE-Provisioning */
#include "wifi_prov_app.h"
/* Cloud-API */
#include "cloud_api_module.h"
/* RTC */
#include "rtc_module.h"
/* Deep Sleep */
#include "esp_timer.h"
#include "esp_sleep.h"

static const char *TAG = "MainAPP";

void setDeepSleepTime();

alarmtime_t alarmTimeData[4];
uint64_t current_time = 0;
static RTC_DATA_ATTR uint8_t active_time_duration = 10;

void app_main(void)
{
     heap_caps_malloc_extmem_enable(0);
     wifi_prov_app_setup();
     cloud_api_create_event_group();
     xTaskCreate(cloud_api_current_time_task, "cloud_api_current_time_task", 8192, (void *)&current_time, 6, NULL);
     cloud_api_wait_current_time_event();

     const char *jsonString = "{\n   \"alarm_id\": 2,\n   \"ollie_id\": 2,\n   \"scene_name\": \"tesy\",\n   \"isActive\": 1,\n   \"days\": \"2023-01-24 00:00:00.000Z\",\n   \"scene_sleep_time\":[\n        {\n          \"scene_sleep_time_name\":\"Almost Asleep\",\n          \"time\":\"1690473700\",\n\t\t  \"color\" : \"Color(0xffcs23)\",\n\t\t  \"brightness\" : 60,\n\t\t  \"countdown_function\" :true,\n\t\t  \"soundname\" : \"Rain\",\n\t\t  \"sound_minutes\": 50\n        },\n        {\n          \"scene_sleep_time_name\":\"To sleep\",\n          \"time\":\"1690473730\",\n\t\t  \"color\" : \"Color(0xffcs23)\",\n\t\t  \"brightness\" : 60,\n\t\t  \"countdown_function\" :true,\n\t\t  \"soundname\" : \"Rain\",\n\t\t  \"sound_minutes\": 50\n        },\n\t {\n          \"scene_sleep_time_name\":\"Almost getting up\",\n          \"time\":\"1690473760\",\n\t\t  \"color\" : \"Color(0xffcs23)\",\n\t\t  \"brightness\" : 60,\n\t\t  \"countdown_function\" :true,\n\t\t  \"soundname\" : \"Rain\",\n\t\t  \"sound_minutes\": 50\n        },\n\t {\n          \"scene_sleep_time_name\":\"To get up\",\n          \"time\":\"1690473798\",\n\t\t  \"color\" : \"Color(0xffcs23)\",\n\t\t  \"brightness\" : 60,\n\t\t  \"countdown_function\" :true,\n\t\t  \"soundname\" : \"Rain\",\n\t\t  \"sound_minutes\": 50\n        }\n       ]\n}";
     cloud_api_parse_time_and_brightness(jsonString, alarmTimeData);

     char c[51];
     setenv("TZ", "GMT-5", 1);
     setEpochTime(current_time, 0);

     struct timeval now;
     uint64_t active_time;
     while (1)
     {
          gettimeofday(&now, NULL);
          active_time = now.tv_sec - (current_time / 1000);
          ESP_LOGI(TAG,"%lld",active_time);

          if (active_time >= active_time_duration)
          {
               ESP_LOGI(TAG, "active_time %llu", active_time);
               ESP_LOGI(TAG, "Active Time Ends");
               printf("Entering deep sleep\n");
               esp_wifi_stop();
               setDeepSleepTime();
               getDateTime(c, false);
               ESP_LOGI(TAG, "%s", c);
               esp_deep_sleep_start();
               // ESP_LOGI(TAG,"20Sec Passed!");
          }
          vTaskDelay(1000 / portTICK_PERIOD_MS);
     }
}

/* Function to set up the RTC and calculate the wakeup time */
void setDeepSleepTime()
{
     struct timeval now, alarm_start_time, alarm_stop_time;

     // Set the current time
     gettimeofday(&now, NULL);

     if( alarmTimeData[ALMOST_SLEEP].epoch_time < alarmTimeData[ALMOST_GETTING_UP].epoch_time &&  now.tv_sec < alarmTimeData[ALMOST_SLEEP].epoch_time)
     {
          // Calculate the desired wakeup time
          alarm_start_time.tv_sec = alarmTimeData[ALMOST_SLEEP].epoch_time;
          alarm_stop_time.tv_sec = alarmTimeData[TO_SLEEP].epoch_time;
     }
     else if(now.tv_sec > alarmTimeData[ALMOST_SLEEP].epoch_time)
     {
          // Calculate the desired wakeup time
          alarm_start_time.tv_sec = alarmTimeData[ALMOST_GETTING_UP].epoch_time;
          alarm_stop_time.tv_sec = alarmTimeData[TO_GET_UP].epoch_time;
     }

     // Update the active time duration of screen :
     active_time_duration = alarm_stop_time.tv_sec - alarm_start_time.tv_sec;
     ESP_LOGI(TAG, "active_duration_time %u", active_time_duration);

     // Set the wakeup time in the RTC alarm
     esp_sleep_enable_timer_wakeup((alarm_start_time.tv_sec - now.tv_sec) * 1000000);
}
