idf_component_register(SRCS "lv_port_disp.c" 
                    INCLUDE_DIRS "."
                    REQUIRES lcd freertos spi_flash esp_timer
                    PRIV_REQUIRES esp_lcd)