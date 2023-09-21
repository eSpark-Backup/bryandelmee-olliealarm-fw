#include <driver/rtc_io.h>
#include "display.hpp"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_sleep.h"
#include "esp_timer.h"
#include "codec.hpp"
#include <dirent.h>
#include "esp_spiffs.h"


/* UPDATE CMAKELISTS.TXT BEFORE idf.py build*
    Add demo.cpp and remove main.c SRCS

*/

#define BUTTON_GPIO GPIO_NUM_20
#define DEEP_SLEEP_TIMER
#define DEEP_SLEEP_EXT0
#define WAV_FILE "/spiffs/audio/Piano.wav"

void audio_task(void *pv);
void display_task(void *pv);
void button_task(void *pv);

TaskHandle_t displayhandle = NULL;
TaskHandle_t audiohandle = NULL;
TaskHandle_t buttonhandle = NULL;
    DISPLAY display;
    codec audio;

void enter_sleep()
{   
    display.off();
    audio.setVol(0);
    vTaskDelay(3000/portTICK_PERIOD_MS);
#ifdef DEEP_SLEEP_TIMER  
  esp_sleep_enable_timer_wakeup(3000000000);
#endif
#ifdef DEEP_SLEEP_EXT0 
  gpio_set_direction(BUTTON_GPIO, GPIO_MODE_INPUT);
  rtc_gpio_hold_en(BUTTON_GPIO);
  esp_sleep_enable_ext0_wakeup(BUTTON_GPIO, 0);
#endif
  esp_deep_sleep_start();
}

extern "C" void app_main(void)
{
    vTaskDelay(1500/portTICK_PERIOD_MS);
    esp_vfs_spiffs_conf_t config = {
      .base_path = "/spiffs",
      .partition_label = NULL,
      .max_files = 5,
      .format_if_mount_failed = true,
    };
    esp_vfs_spiffs_register(&config);

    DIR* dir = opendir("/spiffs/audio");
    if (dir == NULL) {
        return;
    }

    while (true) {
        struct dirent* de = readdir(dir);
        if (!de) {
            break;
        }

        // printf("Found file: %s\n", de->d_name);
    }


    gpio_set_direction(BUTTON_GPIO, GPIO_MODE_INPUT);
    xTaskCreatePinnedToCore(display_task,"Display task", 2*2048, NULL, 10, &displayhandle, 0);
    // xTaskCreatePinnedToCore(audio_task,"audio task", 2*2048, NULL, 10, &audiohandle, 1);
    xTaskCreate(audio_task,"audio task", 2*2048, NULL, 10, &audiohandle);
    xTaskCreate(button_task,"button task", 2*2048, NULL, 10, &buttonhandle);
    while(1)
    {

    }
}

void display_task(void *pv){
    display.setup();
    display.update(DAY);
    while(1){
        display.operate();
    }
}


void audio_task(void *pv){
    audio.setup();
    while(1){
        audio.setVol(100);
        audio.play(WAV_FILE);   
        audio.setVol(0);
        vTaskDelay(500/portTICK_PERIOD_MS);
    }
}
void button_task(void *pv){
    while(1){

        if (gpio_get_level(BUTTON_GPIO) == 0)
        {
            
            enter_sleep();
        } 
    }
}