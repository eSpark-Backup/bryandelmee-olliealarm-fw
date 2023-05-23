idf_component_register(SRCS "lvgl_demo_ui.c" "main.c"
                    INCLUDE_DIRS ".")
spiffs_create_partition_image(storage ../spiffs_data FLASH_IN_PROJECT)