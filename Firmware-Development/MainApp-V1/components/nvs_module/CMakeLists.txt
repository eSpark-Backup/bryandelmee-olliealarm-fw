set(srcs "src/nvs_module.c")

idf_component_register(	SRCS ${srcs}
						INCLUDE_DIRS "include" 
						REQUIRES nvs_flash)