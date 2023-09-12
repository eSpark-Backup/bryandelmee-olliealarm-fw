#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_spiffs.h"
#include "codec.hpp"

static const char* TAG = "MAIN";

void audioTask(void* para);

extern "C" void app_main(void)
{
    esp_vfs_spiffs_conf_t config = {
      .base_path = "/spiffs",
      .partition_label = NULL,
      .max_files = 5,
      .format_if_mount_failed = true,
    };
    esp_vfs_spiffs_register(&config);

    xTaskCreatePinnedToCore(audioTask, "audioTask", 1024*4, NULL, 5, NULL, 1);
    while (1)
    {
        ESP_LOGI(TAG, "HELLO!");
        vTaskDelay(1000/portTICK_PERIOD_MS);    
    }
}  

void audioTask(void* para)
{
    codec es8388 = codec();
    es8388.setup();
    es8388.setVol(100);
    while (1)
    {
        es8388.play("/spiffs/audio/Piano.wav");
        vTaskDelay(1/portTICK_PERIOD_MS);
    }
    
}