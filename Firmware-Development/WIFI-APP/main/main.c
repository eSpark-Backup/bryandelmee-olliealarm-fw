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
/*LCD*/
#include "lcd_app.h"

static const char *TAG = "MainAPP";

void set_alarm_time();
void device_startup();
void lcd_get_screen(uint8_t scr_num);
void timer_callback(void *param);

static RTC_DATA_ATTR alarmtime_t alarmTimeData[4];
static RTC_DATA_ATTR bool alarm_time_set = false;

/* Default Values for sleep time & active time */
static RTC_DATA_ATTR uint64_t mcu_sleep_time =  3600 * 1000000;
static RTC_DATA_ATTR uint32_t mcu_active_time = 120; 

/* LCD Screen Num*/
static RTC_DATA_ATTR uint8_t ui_scr_num = 0;

volatile bool main_loop_run; 

void timer_callback(void *param)
{ 
     static uint8_t count;
     count++;
     if(count == 2)
     {
          lv_obj_clean(lv_scr_act());
          main_loop_run = false;
          ui_scr_num = 0;
          count = 0;
          return;
     }

     if(ui_scr_num == 1)
          ui_scr_num = 2;
     else if(ui_scr_num == 3)
          ui_scr_num = 4;
     
     lv_obj_clean(lv_scr_act());
     lcd_get_screen(ui_scr_num);
}

void lcd_get_screen(uint8_t scr_num)
{
     switch (scr_num)
     {
     case 1:
          _ui_screen_change(ui_Screen1, LV_SCR_LOAD_ANIM_NONE, 0, 10);
          break;

     case 2:
          _ui_screen_change(ui_Screen2, LV_SCR_LOAD_ANIM_NONE, 0, 10);
          break;
     
     case 3:
          _ui_screen_change(ui_Screen3, LV_SCR_LOAD_ANIM_NONE, 0, 10);
          break;
     
     case 4:
          _ui_screen_change(ui_Screen4, LV_SCR_LOAD_ANIM_NONE, 0, 10);
          break;
     
     default:
          break;
     }
}

void app_main(void)
{
     lcd_setup();
     ui_init();

     device_startup();

     const esp_timer_create_args_t my_timer_args = {
      .callback = &timer_callback,
      .name = "My Timer"};
     esp_timer_handle_t timer_handler;
     ESP_ERROR_CHECK(esp_timer_create(&my_timer_args, &timer_handler));
     ESP_ERROR_CHECK(esp_timer_start_periodic(timer_handler, (mcu_active_time/2) * 1000000));

     main_loop_run = true;
     lcd_get_screen(ui_scr_num);

     while (1)
     {
          if(main_loop_run)
          {
               lcd_display();
          }
          else
          {
               alarm_time_set = false;
               device_startup();
          }
          vTaskDelay(1 / portTICK_PERIOD_MS);
     }
}

void device_startup()
{
     heap_caps_malloc_extmem_enable(0);
     if(alarm_time_set == false){
     /* WIFI - BLE*/
     wifi_prov_app_setup();

     /*CLOUD : TIME API*/
     uint64_t current_time = 0;
     cloud_api_create_event_group();
     xTaskCreate(cloud_api_current_time_task, "cloud_api_current_time_task", 8192, (void *)&current_time, 6, NULL);
     cloud_api_wait_current_time_event();
     setenv("TZ", "GMT-5", 1);
     setEpochTime(current_time, 0);

     /*CLOUD : ALARM API*/
     // const char *jsonString = "{\n   \"alarm_id\": 2,\n   \"ollie_id\": 2,\n   \"scene_name\": \"tesy\",\n   \"isActive\": 1,\n   \"days\": \"2023-01-24 00:00:00.000Z\",\n   \"scene_sleep_time\":[\n        {\n          \"scene_sleep_time_name\":\"Almost Asleep\",\n          \"time\":\"1690881800\",\n\t\t  \"color\" : \"Color(0xffcs23)\",\n\t\t  \"brightness\" : 60,\n\t\t  \"countdown_function\" :true,\n\t\t  \"soundname\" : \"Rain\",\n\t\t  \"sound_minutes\": 50\n        },\n        {\n          \"scene_sleep_time_name\":\"To sleep\",\n          \"time\":\"1690881850\",\n\t\t  \"color\" : \"Color(0xffcs23)\",\n\t\t  \"brightness\" : 60,\n\t\t  \"countdown_function\" :true,\n\t\t  \"soundname\" : \"Rain\",\n\t\t  \"sound_minutes\": 50\n        },\n\t {\n          \"scene_sleep_time_name\":\"Almost getting up\",\n          \"time\":\"1690881900\",\n\t\t  \"color\" : \"Color(0xffcs23)\",\n\t\t  \"brightness\" : 60,\n\t\t  \"countdown_function\" :true,\n\t\t  \"soundname\" : \"Rain\",\n\t\t  \"sound_minutes\": 50\n        },\n\t {\n          \"scene_sleep_time_name\":\"To get up\",\n          \"time\":\"1690881940\",\n\t\t  \"color\" : \"Color(0xffcs23)\",\n\t\t  \"brightness\" : 60,\n\t\t  \"countdown_function\" :true,\n\t\t  \"soundname\" : \"Rain\",\n\t\t  \"sound_minutes\": 50\n        }\n       ]\n}";
     xTaskCreate(cloud_api_repeated_alarm_task, "cloud_api_task", 8192, alarmTimeData, 7, NULL);
     cloud_api_wait_repeated_alarm_event();
     // cloud_api_parse_time_and_brightness(jsonString, alarmTimeData);
     set_alarm_time();
     }
}

/* Function to set up the RTC and calculate the wakeup time */
void set_alarm_time()
{
     struct timeval now;
     uint64_t alarm_start_time = 0, alarm_stop_time = 0;
     // Set the current time
     gettimeofday(&now, NULL);

     if( alarmTimeData[ALMOST_SLEEP].epoch_time < alarmTimeData[ALMOST_GETTING_UP].epoch_time &&  now.tv_sec < alarmTimeData[ALMOST_SLEEP].epoch_time)
     {
          // Calculate the desired wakeup time
          alarm_start_time = alarmTimeData[ALMOST_SLEEP].epoch_time;
          alarm_stop_time = alarmTimeData[TO_SLEEP].epoch_time;
          ESP_LOGI(TAG,"set_alarm_time : Sleep Alarm");
          ui_scr_num = 1;
     }
     else if(now.tv_sec > alarmTimeData[ALMOST_SLEEP].epoch_time && alarmTimeData[ALMOST_GETTING_UP].epoch_time > now.tv_sec)
     {
          // Calculate the desired wakeup time
          alarm_start_time = alarmTimeData[ALMOST_GETTING_UP].epoch_time;
          alarm_stop_time = alarmTimeData[TO_GET_UP].epoch_time;
          ESP_LOGI(TAG,"set_alarm_time : WakeUp Alarm");
          ui_scr_num = 3;
     }
     else
     {
          /* Default Case : No Future Alarm */
          mcu_sleep_time =  3600;
          mcu_active_time = 120; 
          esp_sleep_enable_timer_wakeup(mcu_sleep_time * 1000000);
          esp_deep_sleep_start();
     }

     // Update the active time duration of screen :
     mcu_active_time = alarm_stop_time - alarm_start_time;
     ESP_LOGI(TAG, "mcu_active_time %lu", mcu_active_time);

     // Set the wakeup time in the RTC alarm
     mcu_sleep_time = alarm_start_time - (uint64_t)now.tv_sec;
     ESP_LOGI(TAG, "mcu_sleep_time %llu", mcu_sleep_time);
     esp_sleep_enable_timer_wakeup((mcu_sleep_time-5) * 1000000); /*Minus 5sec offset is used for mcu startup*/
     alarm_time_set = true;
     esp_deep_sleep_start();
}
