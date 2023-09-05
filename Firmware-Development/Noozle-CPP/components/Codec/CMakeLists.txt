idf_component_register( SRCS "src/codec.cpp" "src/es8388.c" "src/es8388_i2c.c" "src/es8388_i2s.c"
                        INCLUDE_DIRS "include"
                        REQUIRES driver esp_common IO_EXPANDER)