set(SOURCES src/wifi.cpp src/wifi_event.cpp)

idf_component_register(	SRCS ${SOURCES}
                    	INCLUDE_DIRS "inc"
		    			REQUIRES esp_wifi)