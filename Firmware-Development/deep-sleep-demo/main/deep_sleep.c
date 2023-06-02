#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <inttypes.h>
#include "sdkconfig.h"
#include "esp_timer.h"
#include "esp_sleep.h"
#include "esp_log.h"
#include "esp_err.h"
#include "soc/soc_caps.h"
#include "driver/rtc_io.h"
#include "soc/rtc.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

/*LCD*/
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_rgb.h"
#include "driver/gpio.h"
#include "lvgl.h"
#include "lv_examples.h"
#include "st7701.h"
#include "lv_port_disp.h"

/*SPIFFS*/
#include "esp_spiffs.h"
#include "freertos/timers.h"

static RTC_DATA_ATTR struct timeval sleep_enter_time;
static const char* TAG = "mainAPP";
void app_main(void)
{
    struct timeval now;
    gettimeofday(&now, NULL);
    int sleep_time_ms = (now.tv_sec - sleep_enter_time.tv_sec) * 1000 + (now.tv_usec - sleep_enter_time.tv_usec) / 1000;

    switch (esp_sleep_get_wakeup_cause()) {
        case ESP_SLEEP_WAKEUP_EXT0: {
            printf("Wake up from ext0\n");
            break;
        }

        case ESP_SLEEP_WAKEUP_TIMER: {
            printf("Wake up from timer. Time spent in deep sleep: %dms\n", sleep_time_ms);
            break;
        }

        case ESP_SLEEP_WAKEUP_UNDEFINED:
        default:
            printf("Not a deep sleep reset\n");
    }

    const int wakeup_time_sec = 10;
    const int ext_wakeup_pin_0 = 1;

    /*LCD*/
    esp_vfs_spiffs_conf_t config = {
      .base_path = "/spiffs",
      .partition_label = NULL,
      .max_files = 5,
      .format_if_mount_failed = true,
    };
    esp_vfs_spiffs_register(&config);
    
    lv_port_disp_init();
    lv_port_tick_init();

    lv_obj_t *lottie = lv_rlottie_create_from_file(lv_scr_act(), 400, 400, "/spiffs/lottie.json");
    lv_obj_align(lottie, LV_ALIGN_CENTER, 0, 0);
    lv_rlottie_set_current_frame(lottie, 10);
    /*LCD*/
    // vTaskDelay(5000 / portTICK_PERIOD_MS);

    esp_sleep_enable_timer_wakeup(wakeup_time_sec * 1000000);
    esp_sleep_enable_ext0_wakeup(ext_wakeup_pin_0, 0);
    rtc_gpio_pullup_en(ext_wakeup_pin_0);
    rtc_gpio_pulldown_dis(ext_wakeup_pin_0);
    gettimeofday(&sleep_enter_time, NULL);

    while (1)
    {
        gettimeofday(&now, NULL);
        int active_time = (now.tv_sec - sleep_enter_time.tv_sec) * 1000 + (now.tv_usec - sleep_enter_time.tv_usec) / 1000;

        if(active_time > 20000)
        {
            lv_obj_clean(lottie);
            lottie = NULL;
            lv_port_disp_del();
            ESP_LOGI(TAG,"Active Time Ends");
            printf("Entering deep sleep\n");
            gettimeofday(&sleep_enter_time, NULL);
            esp_deep_sleep_start();
            // ESP_LOGI(TAG,"20Sec Passed!");
        }
        lv_timer_handler();
        vTaskDelay(10/portTICK_PERIOD_MS);
    }
    
}
