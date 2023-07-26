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

esp_err_t lcd_create_task()
{
    BaseType_t ret = xTaskCreatePinnedToCore(lcd_app_run, "lcd_task", 1024*4, NULL, 5, &lcd_task_handle, 1);
    if(ret != pdTRUE)
        return ESP_FAIL;
    
    return ESP_OK;
}

void lcd_app_run( void* parameter )
{
    lcd_setup();
    ui_init();
    _ui_screen_change(ui_Screen1, LV_SCR_LOAD_ANIM_FADE_ON, 0, 0);
    
    while (1)
    {
        lcd_display();
    }
}