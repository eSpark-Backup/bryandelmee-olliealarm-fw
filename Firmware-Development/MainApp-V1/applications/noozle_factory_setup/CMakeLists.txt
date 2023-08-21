set(srcs "src/noozle_factory_setup.c")

idf_component_register(	SRCS ${srcs}
						INCLUDE_DIRS "include" 
						REQUIRES wifi_module nvs_module prov_module cloud_api_module)