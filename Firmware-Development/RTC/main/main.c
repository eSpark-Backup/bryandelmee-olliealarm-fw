#include "rtcTime.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"

static const char* TAG = "mainApp";

void app_main(void)
{
    char* c = (char *)calloc(51, sizeof(char));
    setenv("TZ","GMT-5",1);
    setTime(27, 49, 18, 31, 5, 2023, 0);
    while (1)
    {
        getDateTime(c);
        ESP_LOGI(TAG,"%s", c);   // (String) returns time with specified format 
        vTaskDelay(1000/portTICK_PERIOD_MS);
    }
}