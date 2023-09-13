set(COMPONENT_ADD_INCLUDEDIRS ./include)

# Edit following two lines to set component requirements (see docs)
# set(COMPONENT_REQUIRES )
set(COMPONENT_PRIV_REQUIRES esp_netif)

set(COMPONENT_SRCS ./alarm_config.c)

register_component()