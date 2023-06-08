#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include "esp_log.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

/* WiFi / BLE-Provisioning */
#include "wifi_prov_app.h"

/* Cloud-API */
#include "cloud_api_module.h"

/* RTC */
#include "rtc_module.h"

/* Deep Sleep */
#include "esp_timer.h"
#include "esp_sleep.h"
void setAlarmTime();

static const char *TAG = "MainAPP";
alarmtime_t alarmTimeData[4];   /* Variable to hold Alarm Data's: [ (Almost Sleep)-(To Sleep) & (Almost Getting Up) - (To Get Up)] */

void app_main()
{
    vTaskDelay(1000/portTICK_PERIOD_MS);
    uint64_t current_time = 0;
    wifi_prov_app_setup();
    
    cloud_api_create_event_group();
    xTaskCreate(cloud_api_repeated_alarm_task, "cloud_api_task", 8192, alarmTimeData, 7, NULL);
    cloud_api_wait_repeated_alarm_event();
    vTaskDelay(500/portTICK_PERIOD_MS);
    xTaskCreate(cloud_api_current_time_task, "cloud_api_current_time_task", 8192, (void*)&current_time, 6, NULL);
    cloud_api_wait_current_time_event();

    /*hardcode alarm values*/
    alarmTimeData[ALMOST_SLEEP]         = (alarmtime_t){.epoch_time = (current_time/1000)+20, .brightness=0.0};
    alarmTimeData[ALMOST_GETTING_UP]    = (alarmtime_t){.epoch_time = (current_time/1000)-50, .brightness=0.0};
    // alarmTimeData[ALMOST_GETTING_UP]    = (alarmtime_t){.epoch_time = 1686142472+30+120, .brightness=0.0};
    // alarmTimeData[TO_GET_UP]            = (alarmtime_t){.epoch_time = 1686142472+30+50, .brightness=0.0};

    char c[51];
    setenv("TZ","GMT-5",1);
    setEpochTime(current_time, 0);
    getDateTime(c, false);
    ESP_LOGI(TAG,"%s", c);

    setAlarmTime();
    // esp_deep_sleep_start();
    
}

/* Function to set up the RTC and calculate the wakeup time */
void setAlarmTime()
{
  // Set the current time
  struct timeval now;
  gettimeofday(&now, NULL);
  ESP_LOGI(TAG,"%lld", now.tv_sec);

  // Calculate the desired wakeup time (30 seconds from the current time)
  struct timeval wakeup1,wakeup2;
  wakeup1.tv_sec = (alarmTimeData[ALMOST_SLEEP].epoch_time);
  ESP_LOGI(TAG, "%lld",(wakeup1.tv_sec - now.tv_sec) * 1000000);

  wakeup2.tv_sec = (alarmTimeData[ALMOST_GETTING_UP].epoch_time);
  ESP_LOGI(TAG, "%lld",(wakeup2.tv_sec - now.tv_sec) * 1000000);

  int64_t time1_us = (wakeup1.tv_sec - now.tv_sec) * 1000000;
  int64_t time2_us = (wakeup2.tv_sec - now.tv_sec) * 1000000;
  ESP_LOGI(TAG,"time1_us = %lld", time1_us);
  ESP_LOGI(TAG,"time2_us = %lld", time2_us);

  bool alarmSelect = 0; /* which alarm to be set for RTC */

  if((time1_us < 0))
    alarmSelect = 1;
  else if((time2_us < 0))
    alarmSelect = 0;
  else if( (time1_us < time2_us) )
    alarmSelect = 1;
  else if( (time2_us < time1_us) )
    alarmSelect = 0;
  
  
  // Set the wakeup time in the RTC alarm
  if(alarmSelect == 0)
    esp_sleep_enable_timer_wakeup((wakeup1.tv_sec - now.tv_sec) * 1000000);
  else
    esp_sleep_enable_timer_wakeup((wakeup1.tv_sec - now.tv_sec) * 1000000);
}

