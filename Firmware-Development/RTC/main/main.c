#include "rtcTime.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"

static const char* TAG = "mainApp";

void app_main(void)
{
    char c[51];
    setenv("TZ","GMT-5",1);
    setEpochTime(1685540967, 0);
    while (1)
    {
        getDateTime(c, false);
        ESP_LOGI(TAG,"%s", c);   // (String) returns time with specified format 
        vTaskDelay(1000/portTICK_PERIOD_MS);
    }
}