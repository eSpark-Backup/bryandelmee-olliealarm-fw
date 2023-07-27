set(srcs "src/wifi_prov_app.c")

idf_component_register(	SRCS ${srcs}
						INCLUDE_DIRS "include" 
						REQUIRES wifi_module nvs_module prov_module)