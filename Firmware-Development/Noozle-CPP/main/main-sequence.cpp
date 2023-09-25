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
#include "Noozle.hpp"
#include "esp_system.h"
#include "esp_sleep.h"
#include "cloud_api_module.h"

/* FUNCTION PROTOTYPES*/
void display_task(void *pv);
void audio_task(void *pv);
void deep_sleep(uint64_t sleepTimeUs);
void update_battery_status();
alarm_config createAlarm(time_t realtime);

#define WAV_FILE "/spiffs/audio/Piano.wav"

const char* TAG = "MAIN";

/* TASK HANDLES*/
TaskHandle_t audiohandle = NULL;
TaskHandle_t displayhandle = NULL;

/*RTC ATRR VAR*/
RTC_DATA_ATTR bool alarmState = false;

/*GLOBAL OBJECT*/
DISPLAY display;
codec audio;
Noozle *app;


extern "C" void app_main(void)
{
    app = new Noozle();
    app->setup();
    esp_err_t err = ESP_OK;

    /* HANDLE PROVISIONING*/
    if(app->getProvStatus() == PROVISIONING_STATUS_FAIL){
        app->connectBLE();
        app->setProvStatus(PROVISIONING_STATUS_OK);
        err = app->connectWifi();
    }
    else if(app->getProvStatus() == PROVISIONING_STATUS_OK){
        err = app->connectWifi();
    }
        

    /* SETUP RTC */
    // RTC clock;
    // clock.initialize();
    // clock.syncRtc();
    // clock.updateTime();
    
    /* RESET REASON */
    esp_reset_reason_t rst_reason = esp_reset_reason();
    switch (rst_reason)
    {
    case ESP_RST_DEEPSLEEP:
    {
        esp_sleep_wakeup_cause_t wakeup_cause = esp_sleep_get_wakeup_cause();
        if(wakeup_cause == ESP_SLEEP_WAKEUP_EXT0){
            //User needs WIFI PROV 
            ESP_LOGI(TAG, "RE PROV");  
        }
        else if(wakeup_cause == ESP_SLEEP_WAKEUP_TIMER){
            //Normal Sequence
            ESP_LOGI(TAG, "CONTINUE NORMAL SEQUENCE");  
        }   
        break;
    }        
    case ESP_RST_POWERON:
        ESP_LOGI(TAG, "POWER ON");
        break;
    default:
        ESP_LOGI(TAG, "UNKNOWN RESET");
        break;
    };

    /*FETCH OLLIE DEVICE ID CORRESPONDING TO DEVICE PROV_ NAME*/    
    noozle_cloud_service(NOOZLE_TIME_API); //sync time using API
    noozle_cloud_service(NOOZLE_GET_ID_API);    //fetches Ollie ID
    noozle_cloud_service(NOOZLE_REPEATED_ALARM_API); //Uses Ollie ID to get repeated Alarms
    
    /* SET ALARMS FROM NVS TO NOOZLE APPLICATION*/
    app->setAlarmsFromNvs();
    app->updateTime();

    /* Set nearest Alarm*/
    app->setNearestAlarm();

    /* Update Battery Status*/
    app->updateBatteryStatus();
    app->checkAlarm();
    

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

    // codec audio;
    // audio.setup();

    while(1){
        app->updateTime();
        // ESP_LOGI(TAG,"weekday = %d", clock.realtimeinfo.tm_wday );
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

    /* TASK FUNCTIONS*/

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
