set(srcs "src/prov_module.c")

idf_component_register(	SRCS ${srcs}
						INCLUDE_DIRS "include" 
						REQUIRES wifi_module nvs_module wifi_provisioning)