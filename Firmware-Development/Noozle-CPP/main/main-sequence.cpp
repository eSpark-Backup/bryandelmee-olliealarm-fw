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
RTC_DATA_ATTR app_status_t lastStatus = FORCED_PROV;
RTC_DATA_ATTR bool wifi_status = false;

#define WAV_FILE "/spiffs/audio/Piano.wav"

const char* TAG = "MAIN_SEQUENCE";

Noozle *app;

extern "C" void app_main(void)
{
    /* SETUP NOOZLE */
    app = new Noozle();
    app->setup();
    heap_caps_malloc_extmem_enable(0);  //to remove BLE INIT malloc error 
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
            noozleStatus = noozleStatus; //noozle status set before deep sleep
        }   
        break;
    }        
    case ESP_RST_POWERON:
    {
        ESP_LOGI(TAG, "POWER ON");  
            if(app->getProvStatus() == PROVISIONING_STATUS_FAIL){
                ESP_LOGI(TAG, "1 STATUS = %d", app->getProvStatus());
                noozleStatus = FORCED_PROV;
            }
            else if(app->getProvStatus() == PROVISIONING_STATUS_OK){
                ESP_LOGI(TAG, "0 STATUS = %d", app->getProvStatus()); 
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
                ESP_LOGI(TAG, "FORCED PROVISIONING START");

                app->setProvStatus(PROVISIONING_STATUS_FAIL);
                app->connectBLE();
                app->setProvStatus(PROVISIONING_STATUS_OK);
                noozleStatus = CONNECT_WIFI;
                break;
            }

            case CONNECT_WIFI:{
                ESP_LOGI(TAG, "CONNECT WIFI START");
                if(wifi_status != true){
                    app->connectWifi();
                    wifi_status = true;
                }
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
                ESP_LOGI(TAG, "FETCH TIME START");
                err |= noozle_cloud_service(NOOZLE_TIME_API);
                noozleStatus = FETCH_ID;
                break;
            }
            
            case FETCH_ID:{
                ESP_LOGI(TAG, "FETCH ID START");
                err |= noozle_cloud_service(NOOZLE_GET_ID_API);   
                if(err == ESP_FAIL){
                    ESP_LOGE(TAG,"Fetch ID failed, starting forced Provisioning");
                    noozleStatus = FORCED_PROV;
                    err = ESP_OK;
                    break;
                } 
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
                ESP_LOGI(TAG, "FETCH ALARM START");

                app->fetchRepeated();

                if(err != ESP_OK){
                    ESP_LOGE(TAG, "Fetch Repeated alarm Error");
                }

                noozleStatus = FETCH_NAP_ALARM;
                break;
            }

            case FETCH_NAP_ALARM:{
                ESP_LOGI(TAG, "FETCH NAP ALARM START");
                
                err = ESP_OK;
                err |= noozle_cloud_service(NOOZLE_NAP_ALARM_API); 

                if(err == ESP_FAIL){
                    ESP_LOGE(TAG, "Fetch Naps alarm Error");
                }

                noozleStatus = SET_ALARMS;
                break;
            }

            case DEEP_SLEEP:{
                ESP_LOGI(TAG, "DEEP SLEEP START");

                app->setDeepSleep();
                break;
            }

            case RING_ALARM:{
                ESP_LOGI(TAG, "RING ALARM START");
                
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
                app->updateBatteryStatus();

                noozleStatus = DEEP_SLEEP;
                break;
            }
            
            case SET_ALARMS:{
                ESP_LOGI(TAG, "SET ALARM START");

                err |= (app->setAlarmsFromNvs());

                if(err == ESP_FAIL){
                    ESP_LOGE(TAG, "Set alarm Error");
                }

                noozleStatus = SET_NEAREST;
                break;
            }

            case SET_NEAREST:{
                ESP_LOGI(TAG, "SET NEAREST START");

                //if NVS returns error skip setting nearest alarm
                // if(err == ESP_OK){ 
                    app->setNearestAlarm();
                // }
                
                lastStatus = SET_NEAREST;
                noozleStatus = RING_ALARM;
                break;
            }
            
            default:{
                ESP_LOGI(TAG, "DEFAULT CASE START");
                
                noozleStatus = CONNECT_WIFI;
                break;
            }
        }
        vTaskDelay( 50/portTICK_PERIOD_MS );
       
    }

}
