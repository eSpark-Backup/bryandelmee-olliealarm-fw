#pragma once
#include <esp_err.h>

enum alarm_screen_t : uint8_t { DAY =1, DAWN =2, DUSK =3, NIGHT =4};

class DISPLAY
{
    public:
        DISPLAY();
        esp_err_t setup();
        esp_err_t timer(uint32_t time);
        void operate();
        esp_err_t update(alarm_screen_t time_of_day);
        esp_err_t off();
};