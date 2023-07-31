#include "lcd_lvgl.h"

/* "lv_disp_draw_buf_t disp_buf" : contains internal graphic buffer(s) called draw buffer(s) */
static lv_disp_draw_buf_t disp_buf; 

/* "lv_disp_drv_t disp_drv" contains callback functions */
static lv_disp_drv_t disp_drv;      

/* LVGL draw buffers */
void *buf1 = NULL;
void *buf2 = NULL;

/*  TODO: Replacing extern approach for lcd_panel_handle */
extern esp_lcd_panel_handle_t lcd_panel_handle;


esp_err_t lcd_lvgl_init()
{
    lv_init();

    /* "Allocate separate LVGL draw buffers from PSRAM" */
    buf1 = heap_caps_malloc(480*480 * sizeof(lv_color_t), MALLOC_CAP_SPIRAM);
    buf2 = heap_caps_malloc(480*480  * sizeof(lv_color_t), MALLOC_CAP_SPIRAM);

    /* initialize LVGL draw buffers */
    lv_disp_draw_buf_init(&disp_buf, buf1, buf2, 480*480*2 );

    /* Register display driver to LVGL */
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = LCD_HEIGHT;
    disp_drv.ver_res = LCD_WIDTH;
    disp_drv.flush_cb = lcd_lvgl_flush_cb;
    disp_drv.draw_buf = &disp_buf;
    disp_drv.user_data = lcd_panel_handle;
    disp_drv.set_px_cb = NULL;

    lv_disp_t *disp = lv_disp_drv_register(&disp_drv);
    if(disp == NULL)
        return ESP_FAIL;
    
    return ESP_OK;
}

esp_err_t lcd_lvgl_tick_init()
{
    esp_err_t ret;
    /* Tick interface for LVGL (using esp_timer to generate 2ms periodic event)*/
    const esp_timer_create_args_t lvgl_tick_timer_args = {
        .callback = &lcd_increase_lvgl_tick,
        .name = "lvgl_tick"
    };
    esp_timer_handle_t lvgl_tick_timer = NULL;
    ret = esp_timer_create(&lvgl_tick_timer_args, &lvgl_tick_timer);
    ret = esp_timer_start_periodic(lvgl_tick_timer, LVGL_TICK_PERIOD_MS * 1000);

    return ret;
}

static void lcd_lvgl_flush_cb(lv_disp_drv_t *drv, const lv_area_t *area, lv_color_t *color_map)
{
    esp_lcd_panel_handle_t panel_handle = (esp_lcd_panel_handle_t) drv->user_data;
    int offsetx1 = area->x1;
    int offsetx2 = area->x2;
    int offsety1 = area->y1;
    int offsety2 = area->y2;

    // pass the draw buffer to the driver
    esp_lcd_panel_draw_bitmap(panel_handle, offsetx1, offsety1, offsetx2 + 1, offsety2 + 1, color_map);
    lv_disp_flush_ready(drv);
}

static void lcd_increase_lvgl_tick(void *arg)
{
    /* Tell LVGL how many milliseconds has elapsed */
    lv_tick_inc(LVGL_TICK_PERIOD_MS);
}