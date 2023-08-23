#include <stdio.h>
#include "lcd_app.h"

/* Deep Sleep */
#include "esp_timer.h"
#include "esp_sleep.h"
/* Cloud-API */
#include "cloud_api_module.h"

#include "noozle_factory_setup.h"
#include "noozle_alarm.h"
static const char *TAG = "MainAPP";

void app_main(void)
{
     esp_reset_reason_t rst_reason = esp_reset_reason();
     ESP_LOGI(TAG, "%d", rst_reason);

     noozle_device_initialization();
     noozle_factory_setup();

     switch (rst_reason)
     {
          case ESP_RST_POWERON:
               ESP_LOGI(TAG,"rst_reason : ESP_RST_POWERON");
               while(ESP_OK != noozle_cloud_service(NOOZLE_TIME_API));
               while(ESP_OK != noozle_cloud_service(NOOZLE_GET_ID_API));
               while(ESP_OK != noozle_cloud_service(NOOZLE_REPEATED_ALARM_API));
               noozle_set_alarm();
               break;

          case ESP_RST_DEEPSLEEP:
               ESP_LOGI(TAG,"rst_reason : ESP_RST_DEEPSLEEP");
               break;

          default:
               break;
     }

     char time[50];

     while (1)
     {
          getDateTime(time, 0);
          ESP_LOGI(TAG, "%s", time);
          vTaskDelay(1000 / portTICK_PERIOD_MS);
     }
}