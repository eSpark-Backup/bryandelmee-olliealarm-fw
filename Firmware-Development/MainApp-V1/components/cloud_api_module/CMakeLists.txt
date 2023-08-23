set(srcs "src/cloud_api_module.c" "src/cloud_api_json_parsing.c")

idf_component_register(	SRCS ${srcs}
						INCLUDE_DIRS "include" 
						REQUIRES esp_http_client json rtc_module noozle_factory_setup
						PRIV_REQUIRES nvs_module)