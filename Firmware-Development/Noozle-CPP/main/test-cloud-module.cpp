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
    // app->setProvStatus(PROVISIONING_STATUS_FAIL);
    // app->connectBLE();
    // app->setProvStatus(PROVISIONING_STATUS_OK);
    app->connectWifi();


    NVS alarmNVS;
    alarmNVS.init();

    noozle_cloud_service(NOOZLE_TIME_API); 
    app->updateTime();
    noozle_cloud_service(NOOZLE_GET_ID_API);  //Fetches alarm and aslo save in NVS

    
    if(noozle_cloud_service(NOOZLE_REPEATED_ALARM_API) == ESP_OK){
        ESP_LOGI(TAG, "ALARMS FETCHED!\n");
        vTaskDelay(1000/portTICK_PERIOD_MS);

        app->setAlarmsFromNvs();
        app->setNearestAlarm();

        // uint32_t time;
        // alarmNVS.readKeyU32("Scene1", sleep_scene_string[ALMOST_SLEEP], &time);
        // ESP_LOGI(TAG, "Almost Asleep time NVS : %ld\n", time);
        // vTaskDelay(1000/portTICK_PERIOD_MS);

        // alarmNVS.readKeyU32("Scene1", sleep_scene_string[TO_SLEEP], &time);
        // ESP_LOGI(TAG, "To SLeep time NVS : %ld\n", time);
        // vTaskDelay(1000/portTICK_PERIOD_MS);

        // alarmNVS.readKeyU32("Scene1", sleep_scene_string[ALMOST_GETTING_UP], &time);
        // ESP_LOGI(TAG, "Almost Getting up time NVS : %ld\n", time);
        // vTaskDelay(1000/portTICK_PERIOD_MS);

        // alarmNVS.readKeyU32("Scene1", sleep_scene_string[TO_GET_UP], &time);
        // ESP_LOGI(TAG, "To get up time NVS : %ld\n", time);
        // vTaskDelay(1000/portTICK_PERIOD_MS);

        // uint8_t weekday_bits;
        // alarmNVS.readKeyU8("Scene1", "weekday_bits", &weekday_bits);
        // ESP_LOGI(TAG, "Week days in binary: %d\n", weekday_bits);
        // vTaskDelay(1000/portTICK_PERIOD_MS);
        
        // bits_to_bool(weekday_bits);
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