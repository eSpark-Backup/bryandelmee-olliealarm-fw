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
#include "Noozle.hpp"
#include "esp_system.h"
#include "cloud_api_module.h"

/*STATUS*/
RTC_DATA_ATTR app_status_t noozleStatus = PROV;


/* FUNCTION PROTOTYPES*/
void display_task(void *pv);
void audio_task(void *pv);

#define WAV_FILE "/spiffs/audio/Piano.wav"

const char* TAG = "MAIN";

/* TASK HANDLES*/
TaskHandle_t audiohandle = NULL;
TaskHandle_t displayhandle = NULL;

/*GLOBAL OBJECT*/
DISPLAY display;
codec audio;
Noozle *app;

extern "C" void app_main(void)
{
    /* SETUP NOOZLE */
    app = new Noozle();
    app->setup();
    esp_err_t err = ESP_OK;

    /* HANDLE PROVISONING */
    if(app->getProvStatus() == PROVISIONING_STATUS_FAIL){
        app->connectBLE();
        app->setProvStatus(PROVISIONING_STATUS_OK);
        err = app->connectWifi();
    }
    else if(app->getProvStatus() == PROVISIONING_STATUS_OK){
        ESP_LOGI(TAG, " ELSIFPROVISIONING");
        
        err = app->connectWifi();
    }

    /* CHECK RESET REASON */
    esp_reset_reason_t rst_reason = esp_reset_reason();
    switch (rst_reason)
    {
    case ESP_RST_DEEPSLEEP:
    {
        esp_sleep_wakeup_cause_t wakeup_cause = esp_sleep_get_wakeup_cause();
        if(wakeup_cause == ESP_SLEEP_WAKEUP_EXT0){
            //User needs WIFI PROV = forced provisioning 
            ESP_LOGI(TAG, "RE PROV");  
        }
        else if(wakeup_cause == ESP_SLEEP_WAKEUP_TIMER){
            //Normal Sequence = updatebattery, checkalarm and sleep
            noozleStatus = RING_ALARM;
            ESP_LOGI(TAG, "CONTINUE NORMAL SEQUENCE");  
        }   
        break;
    }        
    case ESP_RST_POWERON:
        ESP_LOGI(TAG, "POWER ON");  //fetch time id alarms
        noozleStatus = FETCH_TIME;
        break;
    default:
        ESP_LOGI(TAG, "UNKNOWN RESET"); // fetch time id alarms
        break;
    };

    /* FETCH TIME ID AND ALARMS AND SAVE ALARMS IN NVS*/
    // noozle_cloud_service(NOOZLE_TIME_API); //sync time using API
    // noozle_cloud_service(NOOZLE_GET_ID_API);    //fetches Ollie ID
    // noozle_cloud_service(NOOZLE_REPEATED_ALARM_API); //Uses Ollie ID to get repeated Alarms

    /*UPDATE TIME*/
    // app->updateTime();

    /* SET ALARMS FROM NVS TO NOOZLE APPLICATION*/
    // app->setAlarmsFromNvs();
   
    /* Set nearest Alarm*/
    // app->setNearestAlarm();

    /* Ring alarm if alarm time is reached*/
    // app->checkAlarm();

    /* Update Battery Status*/
    // app->updateBatteryStatus();

    /* Check alarm either its under 60 mins or not and go deepsleep accordingly*/
    // app->setDeepSleep(); 


    
    /* MAIN WHILE LOOP*/
    while(1){
        ESP_LOGI(TAG, "WHILE LOOP");
        app->updateTime();

        switch (noozleStatus)
        {
            case FETCH_TIME:{
                noozle_cloud_service(NOOZLE_TIME_API); //sync time using API
                noozleStatus = FETCH_ID;
                break;
            }
            
            case FETCH_ID:{
                noozle_cloud_service(NOOZLE_GET_ID_API);    //fetches Ollie ID
                noozleStatus = FETCH_ALARM;
                break;
            }

            case FETCH_ALARM:{
                noozle_cloud_service(NOOZLE_REPEATED_ALARM_API); //Uses Ollie ID to get repeated Alarms
                noozleStatus = RING_ALARM;
                break;
            }

            case DEEP_SLEEP:{
                app->setDeepSleep();
                break;
            }

            case RING_ALARM:{
                app->checkAlarm();
                noozleStatus = UPDATE_BATTERY;
                break;
            }
            
            case UPDATE_BATTERY:{
                app->updateBatteryStatus();
                noozleStatus = SET_ALARMS;
                break;
            }
            
            case SET_ALARMS:{
                app->setAlarmsFromNvs();
                noozleStatus = SET_NEAREST;
                break;
            }

            case SET_NEAREST:{
                app->setNearestAlarm();
                noozleStatus = DEEP_SLEEP;
                break;
            }
            
            default:{

                break;
            }
        }
        vTaskDelay( 1000/portTICK_PERIOD_MS );
       
    }

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


//  switch (status)
//         {
//             case PROV:
//             /* HANDLE NORMAL PROVISIONING*/
//                 ESP_LOGI(TAG, " NORMAL PROVISIONING");
//                 if(app->getProvStatus() == PROVISIONING_STATUS_FAIL){
//                     app->connectBLE();
//                     app->setProvStatus(PROVISIONING_STATUS_OK);
//                     err = app->connectWifi();
//                 }
//                 else if(app->getProvStatus() == PROVISIONING_STATUS_OK){
//                 ESP_LOGI(TAG, " ELSIFPROVISIONING");
                    
//                     err = app->connectWifi();
//                 }
//                 err = app->connectWifi();

//                 ESP_LOGI(TAG, "OUT ELIF PROVISIONING");
//                 status = FETCH_TIME;
//                 break;

//             case FORCED_PROV:
//                 /* HANDLE FORCED PROVISIONING*/
//                 ESP_LOGI(TAG, "FORCED PROVISIONING");
//                 app->setProvStatus(PROVISIONING_STATUS_FAIL);
//                 app->connectBLE();
//                 err = app->connectWifi();
//                 app->setProvStatus(PROVISIONING_STATUS_OK);
//                 status = FETCH_TIME;
//                 break;
                
//             case FETCH_TIME:
//                 /*FETCH OLLIE DEVICE ID CORRESPONDING TO DEVICE PROV_ NAME*/ 
//                 ESP_LOGI(TAG, "FETHCING TIME FROM API");
//                 noozle_cloud_service(NOOZLE_TIME_API); //sync time using API
//                 status = FETCH_ID;
//                 break;

//             // case FETCH_ID:
//             //     ESP_LOGI(TAG, "FETHCING OLLIE ID");
//             //     noozle_cloud_service(NOOZLE_GET_ID_API);    //fetches Ollie ID
//             //     status = FETCH_ALARM;

//             // break;
//             // case FETCH_ALARM:
//             //     ESP_LOGI(TAG, "FETHCING ALARMS");
//             //     noozle_cloud_service(NOOZLE_REPEATED_ALARM_API); //Uses Ollie ID to get repeated Alarms
//             //     status = SET_ALARMS;
//             // break;
//             // case SET_ALARMS:
//             //     /* SET ALARMS FROM NVS TO NOOZLE APPLICATION*/
//             //     app->setAlarmsFromNvs();
//             //     status = SET_NEAREST;
//             // break;
//             // case SET_NEAREST:
//             //     /* Set nearest Alarm*/
//             //     app->setNearestAlarm();
//             //     status = DEEP_SLEEP;
//             // break;
//             // case DEEP_SLEEP:
//             //     /* Check alarm either its under 60 mins or not and go deepsleep accordingly*/
//             //     app->setDeepSleep();    
//             // break;
//             //     case LIGHT_SLEEP:
//             //     /* Check alarm either its under 60 mins or not and go deepsleep accordingly*/
//             //     app->setDeepSleep();    
//             // break;
//             // case RING_ALARM:
//             //     /* Ring alarm if alarm time is reached*/
//             //     app->checkAlarm();
//             //     status = FETCH_ALARM;
//             //     // status
//             // break;
//             // case UPDATE_BATTERY:
//             //     /* Update Battery Status*/
//             //     app->updateBatteryStatus();
//             //     status = DEEP_SLEEP;
//             // break;
//             default:
//                 ESP_LOGI(TAG, "DEFAULT CASE");
//                 break;

//         }