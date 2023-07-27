#include <stdio.h>
/* WiFi / BLE-Provisioning */
#include "wifi_prov_app.h"
/* Cloud-API */
#include "cloud_api_module.h"
/* RTC */
#include "rtc_module.h"

void app_main(void)
{
    heap_caps_malloc_extmem_enable(0);
    uint64_t current_time = 0;
    wifi_prov_app_setup();
    cloud_api_create_event_group();
    xTaskCreate(cloud_api_current_time_task, "cloud_api_current_time_task", 8192, (void*)&current_time, 6, NULL);
    cloud_api_wait_current_time_event();

    char c[51];
    setenv("TZ","GMT-5",1);
    setEpochTime(current_time, 0);
   
   while (1)
   {
        getDateTime(c, false);
        ESP_LOGI("Main","%s", c);
        vTaskDelay(1000/portTICK_PERIOD_MS);
   }
   
}
