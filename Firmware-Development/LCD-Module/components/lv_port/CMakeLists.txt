idf_component_register(SRCS "lv_port_disp.c" 
                    INCLUDE_DIRS "."
                    REQUIRES pwm bsp lcd lvgl
                    PRIV_REQUIRES esp_lcd)