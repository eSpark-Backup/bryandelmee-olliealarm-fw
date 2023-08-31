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
void setupExtWakeUpIO();

extern "C" void app_main(void)
{
    app = new Noozle();
    app->setup();

    setupExtWakeUpIO();
    lcd_backlight_setup();

    esp_reset_reason_t rst_reason = esp_reset_reason();
    switch (rst_reason)
    {
        case ESP_RST_DEEPSLEEP:
        {
            esp_sleep_wakeup_cause_t wakeup_cause = esp_sleep_get_wakeup_cause();
            if(wakeup_cause == ESP_SLEEP_WAKEUP_EXT0)
            {
                lcd_backlight_on();
                app->setProvStatus(1);
                app->connectBLE();
            }
            else if(wakeup_cause == ESP_SLEEP_WAKEUP_TIMER)
            {
                esp_err_t ret = app->connectWifi();
                if(ret != ESP_OK)
                    app->setProvStatus(1); /*1 : Device is not Provisioned */
            }
            break;
        }
        default:
        {
            app->checkProvStatusAndRunBleOrWifi();
        }
            break;
    }

    while (1)
    {
        ESP_LOGI(TAG, "HELLO WORLD!");
        vTaskDelay( (3*1000) /portTICK_PERIOD_MS);
        esp_sleep_enable_timer_wakeup(30 * 1000000);
        esp_sleep_enable_ext0_wakeup(GPIO_NUM_20, 0);
        esp_deep_sleep_start();
    }
}

void setupExtWakeUpIO()
{
    /* Initialize selected GPIO as RTC IO, enable input, disable pullup and pulldown */
    rtc_gpio_init(GPIO_NUM_20);
    rtc_gpio_set_direction(GPIO_NUM_20, RTC_GPIO_MODE_INPUT_ONLY);
    rtc_gpio_pulldown_dis(GPIO_NUM_20);
    rtc_gpio_pullup_dis(GPIO_NUM_20);
    rtc_gpio_hold_en(GPIO_NUM_20);
}

esp_err_t lcd_backlight_setup()
{
    gpio_config_t bl_gpio_conf = {
        .pin_bit_mask = 1ULL << LCD_GPIO_BL,
        .mode = GPIO_MODE_OUTPUT
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