set(srcs "src/rtc_module.c" "src/rtc.cpp")

idf_component_register(	SRCS ${srcs}
						INCLUDE_DIRS "include"
                        REQUIRES soc esp_netif)