set(srcs "src/Noozle.cpp")

idf_component_register(	SRCS ${srcs}
						INCLUDE_DIRS "include" 
						REQUIRES NVS Wifi BLEProv cloud_api_module alarm_config rtc_module
                        PRIV_REQUIRES wifi_provisioning)