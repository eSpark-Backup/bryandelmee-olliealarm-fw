#include <stdio.h>
#include "driver/gpio.h"
#include "driver/rtc_io.h"
#include "esp_check.h"
#include "esp_timer.h"
#include "esp_sleep.h"
#include "esp_netif.h"
#include "freertos/FreeRTOS.h"
static const char* TAG = "MAIN";

#include "Noozle.hpp"
Noozle* app;

#define LCD_GPIO_BL     (GPIO_NUM_38)
esp_err_t lcd_backlight_setup();
esp_err_t lcd_backlight_on();
esp_err_t lcd_backlight_off();

extern "C" void app_main(void)
{
    app = new Noozle();
    app->setup();

    lcd_backlight_setup();

    esp_reset_reason_t rst_reason = esp_reset_reason();
    switch (rst_reason)
    {
        case ESP_RST_DEEPSLEEP:
        {
            esp_sleep_wakeup_cause_t wakeup_cause = esp_sleep_get_wakeup_cause();
            if(wakeup_cause == ESP_SLEEP_WAKEUP_TIMER)
            {
                esp_err_t ret = app->connectWifi();
                if(ret != ESP_OK)
                    app->setProvStatus(PROVISIONING_STATUS_FAIL); /*1 : Device is not Provisioned */

                /*LED - BLINK 2Times on Timer wakeup (After Connect Wifi)*/
                lcd_backlight_on();
                vTaskDelay(1000/portTICK_PERIOD_MS);
                lcd_backlight_off();
                vTaskDelay(1000/portTICK_PERIOD_MS);
                lcd_backlight_on();
                vTaskDelay(1000/portTICK_PERIOD_MS);
                lcd_backlight_off();
                vTaskDelay(1000/portTICK_PERIOD_MS);
            }
            break;
        }
        default:
        {
            app->checkProvStatusAndRunBleOrWifi();
            /*LED - BLINK 1Time on RST (after initial provisioning or connectWifi)*/
            lcd_backlight_on();
            vTaskDelay(1000/portTICK_PERIOD_MS);
            lcd_backlight_off();
            vTaskDelay(1000/portTICK_PERIOD_MS);
        }
            break;
    }

    while (1)
    {
        ESP_LOGI(TAG, "HELLO WORLD!");
        esp_sleep_enable_timer_wakeup(10 * 1000000);
        esp_deep_sleep_start();
    }
}

esp_err_t lcd_backlight_setup()
{
    gpio_config_t bl_gpio_conf = {
        .pin_bit_mask = 1ULL << LCD_GPIO_BL,
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_ENABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    return gpio_config(&bl_gpio_conf);
}

esp_err_t lcd_backlight_on()
{
    return gpio_set_level(LCD_GPIO_BL, 1);
}

esp_err_t lcd_backlight_off()
{
    return gpio_set_level(LCD_GPIO_BL, 0);
}