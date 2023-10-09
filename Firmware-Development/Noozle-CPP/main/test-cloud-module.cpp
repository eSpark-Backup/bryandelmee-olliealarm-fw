#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/param.h>
#include "driver/gpio.h"
#include "driver/rtc_io.h"
#include "esp_check.h"
#include "esp_timer.h"
#include "esp_sleep.h"
#include "esp_netif.h"
#include "esp_http_client.h"
#include "esp_event.h"
#include "cloud_api_module.h"
#include "freertos/FreeRTOS.h"
#include "NVS.hpp"

#include "Noozle.hpp"

static const char* TAG = "MAIN";
void bits_to_bool(uint8_t bits);
Noozle* app;


extern char* sleep_scene_string[];
bool weekday[7];


extern "C" void app_main(void)
{
    app = new Noozle();
    app->setup();
    esp_err_t status = ESP_OK;
    // app->setProvStatus(PROVISIONING_STATUS_FAIL);
    // app->connectBLE();
    // app->setProvStatus(PROVISIONING_STATUS_OK);
    app->connectWifi();


    NVS alarmNVS;
    alarmNVS.init();

    status |= noozle_cloud_service(NOOZLE_TIME_API); 
    app->updateTime();
    status |= noozle_cloud_service(NOOZLE_GET_ID_API);  //Fetches alarm and also save in NVS
    status |= noozle_cloud_service(NOOZLE_REPEATED_ALARM_API);  //Fetches alarm and also save in NVS

    
    if(status == ESP_OK){

        app->setAlarmsFromNvs();
        app->setNearestAlarm();
    }

    while (1)
    {
        app->updateTime();
        vTaskDelay(1000/portTICK_PERIOD_MS);
    }
}

void bits_to_bool(uint8_t bits){
    for(int i=0; i<7; i++){
        if((bits & (1 << (6-i))) != 0){
            weekday[i] = true;
        }
        else{
            weekday[i] = false;
        }
    }

    for(int j = 0; j<7; j++){
        printf("WEEKDAY %d = %d\n", j, weekday[j]);
    }

}