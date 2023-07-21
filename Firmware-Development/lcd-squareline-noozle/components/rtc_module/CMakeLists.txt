set(srcs "src/rtc_module.c")

idf_component_register(	SRCS ${srcs}
						INCLUDE_DIRS "include"
                        REQUIRES soc)