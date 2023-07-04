#include "lv_port_disp.h"

static const char *TAG = "lv_port_disp";

/* Global Variables */
esp_lcd_panel_handle_t panel_handle = NULL;
uint16_t *buf = NULL;

void esp_lcd_setup()
{
    st7701_reg_init();
    esp_lcd_backlight_init();
    esp_lcd_init();
}

void esp_lcd_backlight_init()
{
    ESP_LOGI(TAG, "Turn off LCD backlight");
    gpio_config_t bk_gpio_config = {
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = 1ULL << EXAMPLE_PIN_NUM_BK_LIGHT
    };
    ESP_ERROR_CHECK(gpio_config(&bk_gpio_config));

    ESP_LOGI(TAG, "Turn on LCD backlight");
    gpio_set_level(EXAMPLE_PIN_NUM_BK_LIGHT, EXAMPLE_LCD_BK_LIGHT_ON_LEVEL);
}

void esp_lcd_init()
{
    esp_lcd_rgb_panel_config_t panel_config = {
        .data_width = 16, // RGB565 in parallel mode, thus 16bit in width
        .num_fbs = 2,     // allocate double frame buffer
        .clk_src = LCD_CLK_SRC_DEFAULT,
        .disp_gpio_num = EXAMPLE_PIN_NUM_DISP_EN,
        .pclk_gpio_num = EXAMPLE_PIN_NUM_PCLK,
        .vsync_gpio_num = EXAMPLE_PIN_NUM_VSYNC,
        .hsync_gpio_num = EXAMPLE_PIN_NUM_HSYNC,
        .de_gpio_num = EXAMPLE_PIN_NUM_DE,
        .data_gpio_nums = {
            EXAMPLE_PIN_NUM_DATA0,
            EXAMPLE_PIN_NUM_DATA1,
            EXAMPLE_PIN_NUM_DATA2,
            EXAMPLE_PIN_NUM_DATA3,
            EXAMPLE_PIN_NUM_DATA4,
            EXAMPLE_PIN_NUM_DATA5,
            EXAMPLE_PIN_NUM_DATA6,
            EXAMPLE_PIN_NUM_DATA7,
            EXAMPLE_PIN_NUM_DATA8,
            EXAMPLE_PIN_NUM_DATA9,
            EXAMPLE_PIN_NUM_DATA10,
            EXAMPLE_PIN_NUM_DATA11,
            EXAMPLE_PIN_NUM_DATA12,
            EXAMPLE_PIN_NUM_DATA13,
            EXAMPLE_PIN_NUM_DATA14,
            EXAMPLE_PIN_NUM_DATA15,
        },
        // The timing parameters should refer to your LCD spec
        .timings = {
            .pclk_hz = EXAMPLE_LCD_PIXEL_CLOCK_HZ,
            .h_res = EXAMPLE_LCD_H_RES,
            .v_res = EXAMPLE_LCD_V_RES,
            .hsync_back_porch = 24,
            .hsync_front_porch = 26,
            .hsync_pulse_width = 4,
            .vsync_back_porch = 15,
            .vsync_front_porch = 60,
            .vsync_pulse_width = 4,
        },
        .flags.fb_in_psram = true, // allocate frame buffer from PSRAM
    };
    ESP_ERROR_CHECK(esp_lcd_new_rgb_panel(&panel_config, &panel_handle));

    ESP_LOGI(TAG, "Initialize RGB LCD panel");
    ESP_ERROR_CHECK(esp_lcd_panel_reset(panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_init(panel_handle));

    buf = heap_caps_malloc(480 * 480 * sizeof(uint16_t), MALLOC_CAP_SPIRAM);
    my_memset(buf, 0xF800, 480*480*2);

    esp_lcd_panel_draw_bitmap(panel_handle, 0, 0, 480, 480, buf);
}

//Own Created memset
void *my_memset(void *s, int c,  unsigned int len)
{
    uint16_t*p = s;
    while(len--)
    {
        *p++ = (uint16_t)c;
    }
    return s;
}