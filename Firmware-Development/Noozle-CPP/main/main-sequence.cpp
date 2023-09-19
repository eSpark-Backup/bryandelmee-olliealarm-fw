#include <stdio.h>
#include "driver/gpio.h"
#include <driver/i2c.h>

#include "driver/rtc_io.h"
#include "esp_check.h"
#include "esp_timer.h"
#include "esp_sleep.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "rtc.hpp"
#include "NVS.hpp"
#include "wifi.hpp"
#include "display.hpp"
#include "alarm_config.h"
#include "codec.hpp"
#include <dirent.h>
#include "esp_spiffs.h"


void display_task(void *pv);
void audio_task(void *pv);
void deep_sleep(uint64_t sleepTimeUs);
void update_battery_status();
alarm_config createAlarm(time_t realtime);

#define WAV_FILE "/spiffs/audio/Piano.wav"
#define SLEEP_WAKEUP_OFFSET 15

const char* TAG = "MAIN";
TaskHandle_t audiohandle = NULL;
TaskHandle_t displayhandle = NULL;
RTC_DATA_ATTR bool alarmState = false;
DISPLAY display;
codec audio;

void display_task(void *pv){
    while(1){
        display.operate();
        vTaskDelay(1/portTICK_PERIOD_MS);
    }
}

void audio_task(void *pv){
    audio.setup();
    while(1){
        audio.setVol(90);
        audio.play(WAV_FILE);   
        audio.setVol(0);
        vTaskDelay(1000/portTICK_PERIOD_MS);
    }
}

extern "C" void app_main(void)
{
    WIFI wifi;
    // alarm_config alarm;
    nvs_flash_init();
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    wifi.setup();
    wifi.run();
    wifi.wait();

    RTC clock;
    clock.initialize();
    clock.syncRtc();
    clock.updateTime();

    alarm_config Alarm_1;//using as current alarm
    alarm_config Alarm_2 = createAlarm(clock.real_time);
    alarm_config Alarm_3 = createAlarm(Alarm_2.start_time);
    alarm_config Alarm_4 = createAlarm(Alarm_3.start_time);

    alarm_config myAlarms[] = {Alarm_4, Alarm_2, Alarm_3};

    /*SELECTING CLOSEST ALARM*/
    int size = sizeof(myAlarms)/248;
    time_t lowest = myAlarms[0].start_time;
    int nearestAlarm;
    for(int i = 0; i < size; i++){
        if(myAlarms[i].start_time < lowest){
            lowest = myAlarms[i].start_time; 
            nearestAlarm = i;
        }
    }

    Alarm_1 = myAlarms[nearestAlarm];

    ESP_LOGI(TAG, "SIZE OF MY ALARMS ARRAY %d", size);   
    

    /*SPIFFS SETUP*/
    esp_vfs_spiffs_conf_t config = {
      .base_path = "/spiffs",
      .partition_label = NULL,
      .max_files = 5,
      .format_if_mount_failed = true,
    };
    esp_vfs_spiffs_register(&config);

    DIR* dir = opendir("/spiffs/audio");
    if (dir == NULL) {
        return;
    }

    while (true) {
        struct dirent* de = readdir(dir);
        if (!de) {
            break;
        }

        printf("Found file: %s\n", de->d_name);
    }

    // DISPLAY display;
    // display.setup();

    codec audio;
    audio.setup();

    while(1){
        clock.updateTime();
        if(alarmState)
        {
            while(clock.real_time <= Alarm_1.end_time){
                clock.updateTime();
                audio.setVol(80);
                audio.play(Alarm_1.file_name);
                audio.setVol(0);
            }
            ESP_LOGI(TAG, "Alarm Ringing");
            /* here this passed alarm can be deleted*/
            alarmState = false;
        }
        else if(!alarmState)
        {   
            
            show_alarm_info(&Alarm_1);
            update_battery_status();
            int8_t minuteDifference = clock.minDifference(Alarm_1.start_timeinfo);
            int8_t secondDifference = clock.secDifference(Alarm_1.start_timeinfo);
            if( minuteDifference >= 60)
            {
                ESP_LOGI(TAG, "ESP CAN GO 1HR DEEP SLEEP");
                deep_sleep(60*60*1000*1000);
            }
            else if(minuteDifference < 60 && minuteDifference > 0)
            {
                ESP_LOGI(TAG, "ESP CAN GO DEEP SLEEP FOR %d Mins ideal ", minuteDifference );
                uint64_t calculatedSleepUs = ((minuteDifference * 60) + (secondDifference - SLEEP_WAKEUP_OFFSET)) * 1000000;
                alarmState = true;
                deep_sleep(calculatedSleepUs);
            }
            else if(minuteDifference < 0){
                    ESP_LOGI(TAG, "Alarm Passed waiting for new alarm");
            }
            ESP_LOGI(TAG, "----------------------------------------- = %d", minuteDifference);

        }
        vTaskDelay(1000/portTICK_PERIOD_MS);
        }

}


void deep_sleep(uint64_t sleepTimeUs){
    // uint64_t sleepTimeUs = sleepTime * 60 * 1000 * 1000;
    uint64_t timer = sleepTimeUs;
    ESP_LOGI(TAG, "Activating Deep Sleep for %llu US", timer);
    esp_sleep_enable_timer_wakeup(timer);
    esp_deep_sleep_start();
}

void update_battery_status(){
    ESP_LOGI(TAG, "Updating Battery Status"); //as well as fetch alarm if available
}

/* HELPER FUNCTION */
alarm_config createAlarm(time_t realtime){
    alarm_config thisAlarm;
    strcpy(thisAlarm.file_name, WAV_FILE);
    // thisAlarm.file_name = WAV_FILE;
    time_t offset_time = (60*2);    
    // time_t set_time = 1695130175;
    // time_t end_time = set_time + SLEEP_WAKEUP_OFFSET;
    time_t set_time = realtime + offset_time;
    time_t end_time = set_time + offset_time;
    set_alarm_start_time(&thisAlarm, set_time);  //set epochs before uploading code
    set_alarm_end_time(&thisAlarm, end_time);
    return thisAlarm;
}

