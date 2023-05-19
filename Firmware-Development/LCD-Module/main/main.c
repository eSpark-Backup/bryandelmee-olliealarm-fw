#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
/*LVGL*/
#include "lvgl.h"
#include "lv_examples.h"
#include "lv_port_disp.h"

void create_button();

void app_main(void)
{
    lv_init();                 
    lv_port_disp_init();	      
    lv_port_tick_init();

    lv_example_rlottie_1();
    // lv_example_rlottie_2();

    while (1)
    {
        lv_task_handler();
        vTaskDelay(50 / portTICK_PERIOD_MS);
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
