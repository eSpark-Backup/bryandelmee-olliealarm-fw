idf_component_register( SRCS "src/es8388.c" "src/es8388_i2c.c" "src/es8388_i2s.c"
                        INCLUDE_DIRS "."
                        REQUIRES driver esp_common)