/*
 * SPDX-FileCopyrightText: 2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include <stdio.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_timer.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_rgb.h"
#include "driver/gpio.h"
#include "esp_err.h"
#include "esp_log.h"
#include "lvgl.h"
#include "lv_examples.h"
#include "st7701.h"
#include "lv_port_disp.h"
/*SPIFFS*/
#include "esp_spiffs.h"
#include "freertos/timers.h"

static const char *TAG = "example";

void app_main(void)
{
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
    lv_rlottie_set_current_frame(lottie, 650);

    while (1) {
        lv_timer_handler(); /*~600ms execution time*/
        /* Get the current frame index */
        ESP_LOGI(TAG,"Current Frame = %d",lv_rlottie_get_current_frame(lottie));
    }
}