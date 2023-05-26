/* Deep sleep wake up example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <inttypes.h>
#include "sdkconfig.h"
#include "soc/soc_caps.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_sleep.h"
#include "esp_log.h"
#include "driver/rtc_io.h"
#include "soc/rtc.h"
#include "nvs_flash.h"
#include "nvs.h"

static RTC_DATA_ATTR int a = 0;

void app_main(void)
{
    // gpio_set_direction(GPIO_NUM_38, GPIO_MODE_INPUT);
    // gpio_pullup_en(GPIO_NUM_38);
    vTaskDelay(1000/portTICK_PERIOD_MS);
    esp_sleep_wakeup_cause_t sleep_cause = esp_sleep_get_wakeup_cause();
    ESP_LOGI("TAG","%d",sleep_cause);

    switch (sleep_cause) 
    {
        case ESP_SLEEP_WAKEUP_EXT0: {
            ESP_LOGI("TAG","Wake up from ext0\n");
            // a ^= 1;
            // gpio_set_direction(GPIO_NUM_38, a);
            break;
        }
        case ESP_SLEEP_WAKEUP_UNDEFINED:
            ESP_LOGI("TAG","Undefined\n");
            break;
        default:
                printf("Not a deep sleep reset\n");
                break;
    }

    vTaskDelay(5000 / portTICK_PERIOD_MS);

    const int ext_wakeup_pin_0 = 20;

    printf("Enabling EXT0 wakeup on pin GPIO%d\n", ext_wakeup_pin_0);
    ESP_ERROR_CHECK(esp_sleep_enable_ext0_wakeup(ext_wakeup_pin_0, 0));
    ESP_ERROR_CHECK(rtc_gpio_pullup_en(ext_wakeup_pin_0));
    ESP_ERROR_CHECK(rtc_gpio_pulldown_dis(ext_wakeup_pin_0));
    
    while (1)
    {        
        ESP_LOGI("TAG","HEHE");
        vTaskDelay(3000/portTICK_PERIOD_MS);
        ESP_LOGI("TAG","ENTERING DEEP SLEEP");
        esp_deep_sleep_start(); 
    }
    
}