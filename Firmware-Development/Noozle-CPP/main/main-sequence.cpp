#include <stdio.h>
#include "esp_check.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "display.hpp"
#include "codec.hpp"
#include "Noozle.hpp"
#include "cloud_api_module.h"

/*STATUS*/
RTC_DATA_ATTR app_status_t noozleStatus = FORCED_PROV;
RTC_DATA_ATTR app_status_t lastStatus;
RTC_DATA_ATTR bool wifi_status = false;


/* FUNCTION PROTOTYPES*/
void display_task(void *pv);
void audio_task(void *pv);

#define WAV_FILE "/spiffs/audio/Piano.wav"

const char* TAG = "MAIN_SEQUENCE";

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

    wifi_status = false;

    /* CHECK RESET REASON */
    esp_reset_reason_t rst_reason = esp_reset_reason();
    switch (rst_reason)
    {
    case ESP_RST_DEEPSLEEP:
    {
        esp_sleep_wakeup_cause_t wakeup_cause = esp_sleep_get_wakeup_cause();
        if(wakeup_cause == ESP_SLEEP_WAKEUP_EXT0){
            ESP_LOGI(TAG, "FORCED PROV");  
                noozleStatus = FORCED_PROV;
        }
        else if(wakeup_cause == ESP_SLEEP_WAKEUP_TIMER){
            ESP_LOGI(TAG, "CONTINUE ALARM SEQUENCE");  
                noozleStatus = noozleStatus;            
        }   
        break;
    }        
    case ESP_RST_POWERON:
    {
        ESP_LOGI(TAG, "POWER ON");  
            if(app->getProvStatus() != PROVISIONING_STATUS_OK){
                ESP_LOGI(TAG, "1 STATUS = %d", app->getProvStatus());
                noozleStatus = FORCED_PROV;
            }
            else{
                noozleStatus = CONNECT_WIFI;
            }
        break;        
    }            
    default:
        ESP_LOGI(TAG, "UNKNOWN RESET"); // fetch time id alarms
        noozleStatus = CONNECT_WIFI;
        break;
    };
    
    /* MAIN WHILE LOOP*/
    while(1){
        ESP_LOGI(TAG, "===========================================\n");
        app->updateTime();

        switch (noozleStatus)
        {   
            case FORCED_PROV:{
                app->connectBLE();
                app->setProvStatus(PROVISIONING_STATUS_OK);
                noozleStatus = CONNECT_WIFI;
                break;
            }

            case CONNECT_WIFI:{
                if(wifi_status != true){
                    app->connectWifi();
                    wifi_status = true;
                }
                wifi_status = true;
                    if( err != ESP_OK){
                        noozleStatus = FORCED_PROV;
                        break;
                    }else{
                        noozleStatus = FETCH_TIME;
                        break;
                    }
                break;
            }

            case FETCH_TIME:{
                noozle_cloud_service(NOOZLE_TIME_API);
                noozleStatus = FETCH_ID;
                break;
            }
            
            case FETCH_ID:{
                noozle_cloud_service(NOOZLE_GET_ID_API);    
                if(lastStatus == RING_ALARM){
                        noozleStatus = UPDATE_BATTERY;
                        lastStatus = UPDATE_BATTERY;
                    }
                    else{
                        noozleStatus = FETCH_ALARM;
                }
                break;
            }

            case FETCH_ALARM:{
                noozle_cloud_service(NOOZLE_REPEATED_ALARM_API); 
                noozleStatus = SET_ALARMS;
                break;
            }

            case DEEP_SLEEP:{
                app->setDeepSleep();
                break;
            }

            case RING_ALARM:{
                app->checkAlarm();
                lastStatus = RING_ALARM;
                noozleStatus = UPDATE_BATTERY;
                break;
            }
            
            case UPDATE_BATTERY:{
                ESP_LOGI(TAG, "UPDATE BATTERY CASE");
                if(wifi_status != true){
                    app->connectWifi();
                    wifi_status = true;
                }
                noozle_cloud_service(NOOZLE_GET_ID_API);
                app->updateBatteryStatus();
                noozleStatus = DEEP_SLEEP;
                break;
            }
            
            case SET_ALARMS:{
                app->setAlarmsFromNvs();
                noozleStatus = SET_NEAREST;
                break;
            }

            case SET_NEAREST:{
                app->setNearestAlarm();
                lastStatus = SET_NEAREST;
                noozleStatus = RING_ALARM;
                break;
            }
            
            default:{
                noozleStatus = CONNECT_WIFI;
                break;
            }
        }
        vTaskDelay( 50/portTICK_PERIOD_MS );
       
    }

}

 
/* xTASK FUNCTIONS*/

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