cmake_minimum_required(VERSION 3.5)

file(GLOB SOURCES *.c)
idf_component_register(SRCS ${SOURCES}
                    INCLUDE_DIRS "."
                    REQUIRES lvgl
                    )

target_link_libraries(${COMPONENT_LIB} PUBLIC rlottie)