set(srcs "src/BLEProv.cpp")

idf_component_register(	SRCS ${srcs}
						INCLUDE_DIRS "include" 
						PRIV_REQUIRES wifi_provisioning)