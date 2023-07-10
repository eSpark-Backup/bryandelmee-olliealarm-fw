SET(SOURCES screens/ui_Screen1.c
    screens/ui_Screen2.c
    screens/ui_Screen3.c
    screens/ui_Screen4.c
    screens/ui_Screen5.c
    screens/ui_Screen6.c
    screens/ui_Screen7.c
    screens/ui_Screen8.c
    screens/ui_Screen9.c
    screens/ui_Screen10.c
    screens/ui_Screen11.c
    screens/ui_Screen12.c
    screens/ui_Screen13.c
    screens/ui_Screen14.c
    screens/ui_Screen15.c
    screens/ui_Screen16.c
    screens/ui_Screen17.c
    screens/ui_Screen18.c
    screens/ui_Screen19.c
    screens/ui_Screen20.c
    screens/ui_Screen21.c
    screens/ui_Screen22.c
    screens/ui_Screen23.c
    screens/ui_Screen24.c
    screens/ui_Screen25.c
    screens/ui_Screen26.c
    screens/ui_Screen27.c
    screens/ui_Screen28.c
    screens/ui_Screen29.c
    screens/ui_Screen30.c
    ui.c
    ui_helpers.c
    images/ui_img_299130727.c
    images/ui_img_1714924325.c
    images/ui_img_mountain_png.c)

idf_component_register(SRCS ${SOURCES}
                    INCLUDE_DIRS "."
                    REQUIRES lvgl)
