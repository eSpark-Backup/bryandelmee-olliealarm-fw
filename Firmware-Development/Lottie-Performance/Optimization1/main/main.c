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
#include "sdkconfig.h"

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

static const char *TAG = "MainAPP";
void app_main(void)
{
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
    lv_obj_align(lottie, LV_ALIGN_CENTER, 0,0);

    while (1)
    {
        lv_timer_handler();
    }
}