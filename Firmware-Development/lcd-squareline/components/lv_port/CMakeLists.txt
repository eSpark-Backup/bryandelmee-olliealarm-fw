idf_component_register(SRCS "lv_port_disp.c" 
                    INCLUDE_DIRS "."
                    REQUIRES lcd lvgl freertos spi_flash
                    PRIV_REQUIRES esp_lcd)