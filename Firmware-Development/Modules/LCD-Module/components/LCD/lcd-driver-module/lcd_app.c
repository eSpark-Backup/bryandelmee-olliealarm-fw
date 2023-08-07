#include "lcd_app.h"

TaskHandle_t lcd_task_handle = NULL;

void lcd_get_screen(uint8_t scr_num);
void lcd_change_scr_timer_callback(void *param);

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
    BaseType_t ret = xTaskCreatePinnedToCore(lcd_app_run, "lcd_task", 1024*5, NULL, 10, &lcd_task_handle, 1);
    if(ret != pdTRUE)
        return ESP_FAIL;
    
    return ESP_OK;
}

void lcd_app_run( void* parameter )
{
    lcd_setup();
    ui_init();
    
    const esp_timer_create_args_t my_timer_args = {
      .callback = &lcd_change_scr_timer_callback,
      .name = "My Timer"};
     esp_timer_handle_t timer_handler;
     ESP_ERROR_CHECK(esp_timer_create(&my_timer_args, &timer_handler));
     ESP_ERROR_CHECK(esp_timer_start_periodic(timer_handler, 5 * 1000000));

    while (1)
    {
        lcd_display();
    }
}

void lcd_change_scr_timer_callback(void *param)
{ 
    static uint8_t ui_scr_num = 2;

    lcd_get_screen(ui_scr_num);
    if(ui_scr_num == 2)
        ui_scr_num = 3;
    else
        ui_scr_num = 2;
}

void lcd_get_screen(uint8_t scr_num)
{
     switch (scr_num)
     {
     case 1:
            lv_event_send(ui_Screen4, LV_EVENT_REFRESH, NULL);
            _ui_screen_change(ui_Screen1, LV_SCR_LOAD_ANIM_NONE, 0, 1200);
            break;

     case 2:
            _ui_screen_change(ui_Screen2, LV_SCR_LOAD_ANIM_NONE, 0, 10);
          break;
     
     case 3:
            lv_event_send(ui_Screen2, LV_EVENT_REFRESH, NULL);
            _ui_screen_change(ui_Screen3, LV_SCR_LOAD_ANIM_NONE, 0, 1200);
            break;
     
     case 4:
            _ui_screen_change(ui_Screen4, LV_SCR_LOAD_ANIM_NONE, 0, 10);
            break;
     
     default:
          break;
     }
}
