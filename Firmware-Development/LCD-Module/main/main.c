#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
/*LVGL*/
#include "lvgl.h"
#include "lv_examples.h"
#include "lv_port_disp.h"
/*SPIFFS*/
#include "esp_spiffs.h"

void create_button();

void app_main(void)
{
    esp_vfs_spiffs_conf_t config = {
      .base_path = "/spiffs",
      .partition_label = NULL,
      .max_files = 5,
      .format_if_mount_failed = true,
  };
  esp_vfs_spiffs_register(&config);

    lv_init();                 
    lv_port_disp_init();	      
    lv_port_tick_init();


    /*The rlottie library uses STDIO file API, so there is no driver letter for LVGL*/
    lv_obj_t * lottie = lv_rlottie_create_from_file(lv_scr_act(), 200, 200,
                                                    "/spiffs/lottie.json");
    lv_obj_center(lottie);
    while (1)
    {
        lv_task_handler();
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
    
}

void create_button()
{
    lv_obj_t * label;

    lv_obj_t * btn1 = lv_btn_create(lv_scr_act());
    lv_obj_align(btn1, LV_ALIGN_CENTER, 0, -40);
    label = lv_label_create(btn1);
    lv_label_set_text(label, "Button");
    lv_obj_center(label);
}
