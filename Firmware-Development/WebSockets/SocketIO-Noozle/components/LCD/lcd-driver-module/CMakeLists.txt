idf_component_register( SRCS "display.cpp" "lcd_app.c" "lcd_lvgl.c" "lcd_rgb_driver.c" "lcd_spi_driver.c"
                        INCLUDE_DIRS "include"
                        REQUIRES driver lvgl esp_lcd ui-module)
