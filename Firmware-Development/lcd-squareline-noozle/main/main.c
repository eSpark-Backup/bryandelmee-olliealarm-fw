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
#include "lv_demos.h"
#include "ui.h"
#include "ui_helpers.h"
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

// void timer_callback(void *param)
// {
//   static int count = 0;
//   static int screenNum = 0;
//   count++;
  
//   if(count == 10)
//   {
//     if(screenNum == 0){
//       ui_Screen1_screen_init();
//       lv_disp_load_scr(ui_Screen2);
//     }
      
//     else if(screenNum == 1){
//       lv_obj_del(ui_Screen1);
//       ui_Screen1 = NULL;
//       ui_Screen2_screen_init();
//       lv_disp_load_scr(ui_Screen2);
//     }
//     else if(screenNum == 2){
//       lv_obj_del(ui_Screen2);
//       ui_Screen2 = NULL;
//       ui_Screen3_screen_init();
//       lv_disp_load_scr(ui_Screen3);
//     }
//     else{
//       lv_obj_del(ui_Screen3);
//       ui_Screen3 = NULL;
//       ui_Screen3_screen_init();
//       lv_disp_load_scr(ui_Screen4);
//     }
//     count = 0;
//     screenNum++;
//     if(screenNum == 4)
//       screenNum = 0;
//   }
// }
volatile bool lvTimerHandler = false;
void timer_callback(void *param)
{
  static int count = 10;
  static int screenNum = 0;
  
  if(count == 10)
  {
      // lvTimerHandler = true;
      // ui_Screen1_screen_init();
      // lv_disp_load_scr(ui_Screen1);
      // _ui_screen_change(ui_Screen1, LV_SCR_LOAD_ANIM_FADE_ON, 1000, 1000);
      _ui_screen_change(ui_Screen3, LV_SCR_LOAD_ANIM_FADE_ON, 0, 0);
  }
  else if(count == 20)
  {
    // lvTimerHandler = false;
    _ui_screen_change(ui_Screen1, LV_SCR_LOAD_ANIM_FADE_ON, 0, 0);
  }
  else if(count == 30)
  {
    count = 10;
  }
  // else if(count == 21) 
  // {
  //   lv_anim_del(&PropertyAnimation_0, NULL);
  //   lv_obj_clean(ui_Screen1);
  //   ui_Screen1 = NULL;
  //   count = 9;
  // }
  count++;
}


void app_main(void)
{
    // heap_caps_malloc_extmem_enable(0);
    // uint64_t current_time = 0;
    // wifi_prov_app_setup();
    // cloud_api_create_event_group();
    // xTaskCreate(cloud_api_repeated_alarm_task, "cloud_api_task", 8192, alarmTimeData, 7, NULL);
    // // cloud_api_wait_repeated_alarm_event();
    // vTaskDelay(500/portTICK_PERIOD_MS);
    // xTaskCreate(cloud_api_current_time_task, "cloud_api_current_time_task", 8192, (void*)&current_time, 6, NULL);
    // cloud_api_wait_current_time_event();

    // char c[51];
    // setenv("TZ","GMT-5",1);
    // setEpochTime(current_time, 0);
    // getDateTime(c, false);
    // ESP_LOGI(TAG,"%s", c);
    
    lv_port_disp_init();
    lv_port_tick_init();
    ui_init();
    // _ui_screen_change(ui_Screen1, LV_SCR_LOAD_ANIM_FADE_ON, 0, 0);
    // lv_disp_load_scr(ui_Screen1);
    // lv_disp_load_scr(ui_Screen4);
    // _ui_screen_change(ui_Screen1, LV_SCR_LOAD_ANIM_FADE_ON, 0, 10000);
    // _ui_screen_change(ui_Screen3, LV_SCR_LOAD_ANIM_FADE_ON, 10, 10000);

    // struct timeval now;
    // uint64_t active_time;
      const esp_timer_create_args_t my_timer_args = {
        .callback = &timer_callback,
        .name = "My Timer"};
      esp_timer_handle_t timer_handler;
      ESP_ERROR_CHECK(esp_timer_create(&my_timer_args, &timer_handler));
      ESP_ERROR_CHECK(esp_timer_start_periodic(timer_handler, 1000000));

    while (1)
    {
        // gettimeofday(&now, NULL);
        // active_time = now.tv_sec - (current_time/1000);
        
        // if(active_time >= active_time_duration)
        // {
        //     ESP_LOGI(TAG,"active_time %llu", active_time);
        //     lv_port_disp_del();
        //     ESP_LOGI(TAG,"Active Time Ends");
        //     printf("Entering deep sleep\n");
        //     esp_wifi_stop();
        //     gettimeofday(&sleep_enter_time, NULL);
        //     setAlarmTime();
        //     getDateTime(c, false);
        //     ESP_LOGI(TAG,"%s", c);
        //     esp_deep_sleep_start();
        //     // ESP_LOGI(TAG,"20Sec Passed!");
        //   }
        // if(lvTimerHandler)
          lv_timer_handler();
        
        vTaskDelay(1/portTICK_PERIOD_MS);
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
