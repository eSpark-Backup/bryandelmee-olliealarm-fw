#include "lcd_app.h"

TaskHandle_t lcd_task_handle = NULL;

esp_err_t lcd_setup()
{
    esp_err_t ret;
    ret = lcd_driver_startup();
    ret = lcd_rgb_driver_int();
    ret = lcd_lvgl_init();
    ret = lcd_lvgl_tick_init();
    ret = lcd_backlight_setup();
    ret = lcd_backlight_on();

    return ret;
}

void lcd_display()
{
    lv_timer_handler();
}

esp_err_t lcd_create_task(void* param)
{
    BaseType_t ret = xTaskCreatePinnedToCore(lcd_app_run, "lcd_task", 1024*4, param, 5, &lcd_task_handle, 1);
    if(ret != pdTRUE)
        return ESP_FAIL;
    
    return ESP_OK;
}

void lcd_app_run( void* parameter )
{
    lcd_config_alarm_t* lcd_alarm_config = (lcd_config_alarm_t*)(parameter); 
    lcd_setup();
    ui_init();
    if(lcd_alarm_config->alarm_name == 0)
        _ui_screen_change(ui_Screen1, LV_SCR_LOAD_ANIM_FADE_ON, 0, 0);
    else if(lcd_alarm_config->alarm_name == 1)
        _ui_screen_change(ui_Screen3, LV_SCR_LOAD_ANIM_FADE_ON, 0, 0);
    
    int64_t now;
    while (1)
    {
        now = time(NULL);

        if( now >= (lcd_alarm_config->start_time) && now <= (lcd_alarm_config->end_time) )
            lcd_display();
        else if(now > lcd_alarm_config->end_time)
        {
            lcd_backlight_off();
            vTaskDelete(NULL);
        }
        vTaskDelay(1/portTICK_PERIOD_MS);
    }
}