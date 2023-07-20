#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <inttypes.h>
#include "sdkconfig.h"
#include "esp_timer.h"
#include "esp_sleep.h"
#include "esp_log.h"
#include "esp_err.h"
#include "soc/soc_caps.h"
#include "driver/rtc_io.h"
#include "soc/rtc.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "sdkconfig.h"

/*LCD*/
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_rgb.h"
#include "driver/gpio.h"
#include "lvgl.h"
#include "lv_examples.h"
#include "st7701.h"
#include "lv_port_disp.h"

/*SPIFFS*/
#include "esp_spiffs.h"
#include "freertos/timers.h"

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

static RTC_DATA_ATTR struct timeval sleep_enter_time;
static RTC_DATA_ATTR uint8_t active_time_duration = 120;
static RTC_DATA_ATTR uint8_t frame_number = 0;
static const char *TAG = "MainAPP";
alarmtime_t alarmTimeData[4];   /* Variable to hold Alarm Data's: [ (Almost Sleep)-(To Sleep) & (Almost Getting Up) - (To Get Up)] */

void app_main(void)
{
    /*LCD*/
    esp_vfs_spiffs_conf_t config = {
      .base_path = "/spiffs",
      .partition_label = NULL,
      .max_files = 5,
      .format_if_mount_failed = true,
    };
    esp_vfs_spiffs_register(&config);

    heap_caps_malloc_extmem_enable(0);
    uint64_t current_time = 0;
    wifi_prov_app_setup();
    cloud_api_create_event_group();
    xTaskCreate(cloud_api_repeated_alarm_task, "cloud_api_task", 8192, alarmTimeData, 7, NULL);
    cloud_api_wait_repeated_alarm_event();
    vTaskDelay(500/portTICK_PERIOD_MS);
    xTaskCreate(cloud_api_current_time_task, "cloud_api_current_time_task", 8192, (void*)&current_time, 6, NULL);
    cloud_api_wait_current_time_event();

    char c[51];
    setenv("TZ","GMT-5",1);
    setEpochTime(current_time, 0);
    getDateTime(c, false);
    ESP_LOGI(TAG,"%s", c);
    
    lv_port_disp_init();
    lv_port_tick_init();
    
    lv_obj_t *lottie = lv_rlottie_create_from_file(lv_scr_act(), 700, 700, "/spiffs/lottie.json");
    lv_obj_align(lottie, LV_ALIGN_CENTER, -12, -15);
    lv_rlottie_set_current_frame(lottie, frame_number);
    
    struct timeval now;
    uint64_t active_time;
    while (1)
    {
        gettimeofday(&now, NULL);
        active_time = now.tv_sec - (current_time/1000);
        
        if(active_time >= active_time_duration)
        {
            ESP_LOGI(TAG,"active_time %llu", active_time);
            lv_obj_clean(lottie);
            lottie = NULL;
            lv_port_disp_del();
            ESP_LOGI(TAG,"Active Time Ends");
            printf("Entering deep sleep\n");
            esp_wifi_stop();
            gettimeofday(&sleep_enter_time, NULL);
            setAlarmTime();
            getDateTime(c, false);
            ESP_LOGI(TAG,"%s", c);
            esp_deep_sleep_start();
            // ESP_LOGI(TAG,"20Sec Passed!");
          }
        lv_timer_handler();
        vTaskDelay(10/portTICK_PERIOD_MS);
    }
}

/* Function to set up the RTC and calculate the wakeup time */
void setAlarmTime()
{
  struct timeval now, wakeup1, wakeup2;
  // Set the current time
  gettimeofday(&now, NULL);

  //Manipulate the alarm times retrive from api:
  alarmTimeData[TO_SLEEP].epoch_time = now.tv_sec+30;
  alarmTimeData[ALMOST_SLEEP].epoch_time = now.tv_sec+70;
  frame_number = 120;

  // Calculate the desired wakeup time (30 seconds from the current time)
  wakeup1.tv_sec = alarmTimeData[TO_SLEEP].epoch_time;
  wakeup2.tv_sec = alarmTimeData[ALMOST_SLEEP].epoch_time;
  // Update the active time duration of screen : 
  active_time_duration = wakeup2.tv_sec - wakeup1.tv_sec;
  ESP_LOGI(TAG,"hehe %u", active_time_duration);

  int64_t time1_us = (wakeup1.tv_sec - now.tv_sec) * 1000000;
  int64_t time2_us = (wakeup2.tv_sec - now.tv_sec) * 1000000;
  
  // Set the wakeup time in the RTC alarm
  esp_sleep_enable_timer_wakeup((wakeup1.tv_sec - now.tv_sec) * 1000000);
}
