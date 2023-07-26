SET(SOURCES screens/ui_Screen1.c
    screens/ui_Screen2.c
    screens/ui_Screen3.c
    screens/ui_Screen4.c
    ui.c
    ui_helpers.c
    images/ui_img_880741718.c
    images/ui_img_1280572105.c
    images/ui_img_1398729226.c
    images/ui_img_moon_png.c
    images/ui_img_sun_png.c
    images/ui_img_stars_png.c)

idf_component_register(SRCS ${SOURCES}
                    INCLUDE_DIRS "."
                    REQUIRES lvgl)
