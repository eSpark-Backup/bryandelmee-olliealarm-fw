set(srcs "src/wifi_module.c")

idf_component_register(	SRCS ${srcs}
						INCLUDE_DIRS "include" 
						REQUIRES nvs_module prov_module)