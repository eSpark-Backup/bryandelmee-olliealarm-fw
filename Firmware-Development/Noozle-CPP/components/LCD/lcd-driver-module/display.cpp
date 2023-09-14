#include "display.hpp"
#include "esp_check.h"

/* lcd-driver-moudle*/
#include "lcd_spi_driver.h"
#include "lcd_rgb_driver.h"
#include "lcd_lvgl.h"

/* ui-moudle */
#include "ui.h"
#include "ui_helpers.h"


static const char* TAG = "DISPLAY";

/* Function prototypes*/
void lcd_change_screen(void *param);
void lcd_set_screen(uint8_t scr_num);


/**
 * @brief Display object constructor
 * 
*/
DISPLAY::DISPLAY(){

}

/**
 * @brief LCD Setup function
 * 
 * @return esp_err_t
*/
esp_err_t DISPLAY::setup()
{
    esp_err_t ret = ESP_OK;
    ret = lcd_driver_startup();
    ret = lcd_rgb_driver_int();
    ret = lcd_lvgl_init();
    ret = lcd_lvgl_tick_init();
    ret = lcd_backlight_setup();
    ret = lcd_backlight_on();

    ui_init();
    return ret;
}

/**
 * @brief LCD timer setup. callbacks after the desired time to a callback function
 * 
 * @return esp_err_t
*/
esp_err_t DISPLAY::timer(uint32_t time){
    time  = 5 * 1000000;
    const esp_timer_create_args_t my_timer_args = {
    .callback = &lcd_change_screen,
    .name = "My Timer"};
    esp_timer_handle_t timer_handler;
    ESP_ERROR_CHECK(esp_timer_create(&my_timer_args, &timer_handler));
    ESP_ERROR_CHECK(esp_timer_start_periodic(timer_handler, time));
    return ESP_OK;
}

/**
 * @brief call lv_timer_handler
*/
void DISPLAY::operate(){
    lv_timer_handler();
}

/**
 * @brief primarily used as change screen timer callback
*/
void lcd_change_screen(void *param)
{ 
    static uint8_t ui_scr_num = 4;

    lcd_set_screen(ui_scr_num);
    if(ui_scr_num == 4)
        ui_scr_num = 1;
    else if(ui_scr_num == 1)
        ui_scr_num = 2;
    else if(ui_scr_num == 2)
        ui_scr_num = 3;
    else
        ui_scr_num = 4;
}

/**
 * @brief Set the screen/ui ie; day, night, dawn, dusk 
 * @param 1: sceen1
 * @param 2: sceen2
 * @param 3: sceen3
 * @param 4: sceen4
*/
void lcd_set_screen(uint8_t scr_num)
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

esp_err_t DISPLAY::update(time_of_day_t time_of_day){

    lcd_set_screen(time_of_day); //currently passing 1 will change later
    return ESP_OK;
}
