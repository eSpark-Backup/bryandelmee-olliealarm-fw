set(COMPONENT_ADD_INCLUDEDIRS ./include
                            ./driver/es8388
                            ./driver/include)

# Edit following two lines to set component requirements (see docs)
set(COMPONENT_REQUIRES )
set(COMPONENT_PRIV_REQUIRES audio_sal mbedtls esp_peripherals)

set(COMPONENT_SRCS ./audio_hal.c
                    ./audio_volume.c
                    ./driver/es8388/es8388.c
                    )

register_component()
