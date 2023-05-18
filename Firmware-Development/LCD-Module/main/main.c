#include <stdio.h>
#include "lvgl.h" /*LVGL*/
#include "lv_port_disp.h" /* LVGL Porting for ST7701*/
#include "rlottie_capi.h"

void create_button();

void app_main(void)
{
    lv_init();
    lv_port_disp_init();
    create_button();
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
