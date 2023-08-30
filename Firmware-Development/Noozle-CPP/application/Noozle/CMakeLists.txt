set(srcs "src/Noozle.cpp")

idf_component_register(	SRCS ${srcs}
						INCLUDE_DIRS "include" 
						REQUIRES NVS Wifi BLEProv
                        PRIV_REQUIRES wifi_provisioning)