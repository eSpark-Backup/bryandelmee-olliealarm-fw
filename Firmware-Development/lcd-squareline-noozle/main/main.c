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
#include "lv_demos.h"
#include "ui.h"
#include "ui_helpers.h"
/* WiFi / BLE-Provisioning */
#include "wifi_prov_app.h"

/* Cloud-API */
#include "cloud_api_module.h"

/* RTC */
#include "rtc_module.h"

/* Deep Sleep */
#include "esp_timer.h"
#include "esp_sleep.h"

volatile bool screen_change = true;
volatile int screen_num = 1;

void timer_callback(void *param)
{
  static int count = 0; 
  count++;
  if(count == 5)
  {
    count = 0;
    screen_change = true;
    screen_num++;
  }
}

void change_screen()
{
  if(screen_change == true)
  {
    screen_change = false;

    if(screen_num == 1)
    {
      _ui_screen_change(ui_Screen1, LV_SCR_LOAD_ANIM_FADE_ON, 0, 0);
    }
    else if(screen_num == 2)
    {
      _ui_screen_change(ui_Screen2, LV_SCR_LOAD_ANIM_FADE_ON, 0, 0);
    }
    else if(screen_num == 3)
    {
      _ui_screen_change(ui_Screen3, LV_SCR_LOAD_ANIM_FADE_ON, 0, 0);
    }
    else if(screen_num == 4)
    {
      _ui_screen_change(ui_Screen4, LV_SCR_LOAD_ANIM_FADE_ON, 0, 0);
      screen_num = 0;
    }
  }
}

void app_main(void)
{
    
    lv_port_disp_init();
    lv_port_tick_init();
    ui_init();

      const esp_timer_create_args_t my_timer_args = {
        .callback = &timer_callback,
        .name = "My Timer"};
      esp_timer_handle_t timer_handler;
      ESP_ERROR_CHECK(esp_timer_create(&my_timer_args, &timer_handler));
      ESP_ERROR_CHECK(esp_timer_start_periodic(timer_handler, 1000000));

    while (1)
    {
        change_screen();
        lv_timer_handler();
        vTaskDelay(1/portTICK_PERIOD_MS);
    }
}
