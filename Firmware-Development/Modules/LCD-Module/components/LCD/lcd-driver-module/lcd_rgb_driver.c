#include "lcd_rgb_driver.h"

esp_lcd_panel_handle_t lcd_panel_handle = NULL;

esp_err_t lcd_rgb_driver_int()
{
    esp_lcd_rgb_panel_config_t panel_config = {
        .data_width = 16, 
        .bits_per_pixel = 16,
        .psram_trans_align = 64,
        .clk_src = LCD_CLK_SRC_PLL160M,
        .disp_gpio_num = LCD_GPIO_DISP,
        .pclk_gpio_num = LCD_GPIO_PCLK,
        .vsync_gpio_num = LCD_GPIO_VSYNC,
        .hsync_gpio_num = LCD_GPIO_HSYNC,
        .de_gpio_num = LCD_GPIO_DE,
        .data_gpio_nums = {
            LCD_GPIO_B0,
            LCD_GPIO_B1,
            LCD_GPIO_B2,
            LCD_GPIO_B3,
            LCD_GPIO_B4,
            LCD_GPIO_G0,
            LCD_GPIO_G1,
            LCD_GPIO_G2,
            LCD_GPIO_G3,
            LCD_GPIO_G4,
            LCD_GPIO_G5,
            LCD_GPIO_R0,
            LCD_GPIO_R1,
            LCD_GPIO_R2,
            LCD_GPIO_R3,
            LCD_GPIO_R4,
        },
        .timings = {
            .pclk_hz = LCD_PIXEL_CLOCK_HZ,
            .h_res = LCD_HEIGHT,
            .v_res = LCD_HEIGHT,
            // The following parameters should refer to LCD spec
            .hsync_back_porch = LCD_RGB_HBP,
            .hsync_front_porch = LCD_RGB_HFP,
            .hsync_pulse_width = LCD_RGB_HPW,
            .vsync_back_porch = LCD_RGB_VBP,
            .vsync_front_porch = LCD_RGB_VFP,
            .vsync_pulse_width = LCD_RGB_VPW,
            .flags.pclk_active_neg = 1,
            .flags.de_idle_high = 0,
        },
        .num_fbs = 2,
        .flags.fb_in_psram = true, // allocate frame buffer in PSRAM
    };

    esp_err_t ret = ESP_OK;
    ret |= esp_lcd_new_rgb_panel(&panel_config, &lcd_panel_handle);
    ret |= esp_lcd_panel_reset(lcd_panel_handle);
    ret |= esp_lcd_panel_init(lcd_panel_handle);

    return ret;
}

esp_err_t lcd_backlight_setup()
{
    gpio_config_t bl_gpio_conf = {
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = 1ULL << LCD_GPIO_BL
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