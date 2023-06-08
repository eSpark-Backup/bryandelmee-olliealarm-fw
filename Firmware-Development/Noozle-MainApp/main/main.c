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
    // alarmTimeData[ALMOST_SLEEP]         = (alarmtime_t){.epoch_time = 1686143350, .brightness=0.0};
    // alarmTimeData[TO_SLEEP]             = (alarmtime_t){.epoch_time = 1686142472+30+50, .brightness=0.0};
    // alarmTimeData[ALMOST_GETTING_UP]    = (alarmtime_t){.epoch_time = 1686142472+30+120, .brightness=0.0};
    // alarmTimeData[TO_GET_UP]            = (alarmtime_t){.epoch_time = 1686142472+30+50, .brightness=0.0};
    uint64_t current_time = 0;

    wifi_prov_app_setup();

    cloud_api_create_event_group();
    xTaskCreate(cloud_api_current_time_task, "cloud_api_current_time_task", 8192, (void*)&current_time, 6, NULL);
    cloud_api_wait_current_time_event();

    alarmTimeData[ALMOST_SLEEP]         = (alarmtime_t){.epoch_time = (current_time/1000)+60, .brightness=0.0};

    char c[51];
    setenv("TZ","GMT-5",1);
    setEpochTime(current_time, 0);
    
    getDateTime(c, false);
    ESP_LOGI(TAG,"%s", c);

    setAlarmTime();
    esp_deep_sleep_start();
    
}

void setAlarmTime()
{
    // Function to set up the RTC and calculate the wakeup time
  // Set the current time
  struct timeval now;
  gettimeofday(&now, NULL);
  ESP_LOGI(TAG,"%lld", now.tv_sec);

  // Calculate the desired wakeup time (30 seconds from the current time)
  struct timeval wakeup,wakeup2;
  wakeup.tv_sec = (alarmTimeData[ALMOST_SLEEP].epoch_time);
  ESP_LOGI(TAG, "%lld",(wakeup.tv_sec - now.tv_sec) * 1000000);
  
  // Set the wakeup time in the RTC alarm
  esp_sleep_enable_timer_wakeup((wakeup.tv_sec - now.tv_sec) * 1000000);
}

