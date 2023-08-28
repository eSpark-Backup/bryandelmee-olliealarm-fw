#include "lcd_app.h"
#include "freertos/event_groups.h"

TaskHandle_t lcd_task_handle = NULL;
static const char* TAG = "LCD-APP";

#define ALARM_SETUP_BIT BIT0
static EventGroupHandle_t alarm_event_group;

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
    BaseType_t ret = xTaskCreatePinnedToCore(lcd_app_run, "lcd_task", 1024*4, param, 10, &lcd_task_handle, 1);
    if(ret != pdTRUE)
        return ESP_FAIL;
    
    return ESP_OK;
}

void lcd_app_wait()
{
    // xEventGroupWaitBits(alarm_event_group, ALARM_SETUP_BIT, false, true, portMAX_DELAY);
}

extern bool alarm_Trigger;
void lcd_app_run( void* parameter )
{
    lcd_config_alarm_t* lcd_alarm_config = (lcd_config_alarm_t*)(parameter); 
    lcd_setup();
    ui_init();

    if(lcd_alarm_config->alarm_name == 0)
    {
        _ui_screen_change(ui_Screen1, LV_SCR_LOAD_ANIM_FADE_ON, 0, 0);
        ESP_LOGI(TAG,"ui_Screen1 Sets");
    }
    else if(lcd_alarm_config->alarm_name == 1){
        _ui_screen_change(ui_Screen3, LV_SCR_LOAD_ANIM_FADE_ON, 0, 0);
        ESP_LOGI(TAG,"ui_Screen3 Sets");
    }
    
    int64_t now;
    while (1)
    {
        now = time(NULL);

        if( now >= (lcd_alarm_config->start_time) && now <= (lcd_alarm_config->end_time) )
        {
            lcd_display();
            ESP_LOGI(TAG,"lcd_display");
        }
        else if(now > lcd_alarm_config->end_time)
        {
            lcd_backlight_off();
            alarm_Trigger = true;
            vTaskDelete(NULL);
        }
        vTaskDelay(1/portTICK_PERIOD_MS);
    }
}

void lcd_test()
{
    lcd_setup();
    ui_init();
    _ui_screen_change(ui_Screen1, LV_SCR_LOAD_ANIM_FADE_ON, 0, 0);

    while (1)
    {
        lcd_display();
        vTaskDelay(1/portTICK_PERIOD_MS);
    }
    
}