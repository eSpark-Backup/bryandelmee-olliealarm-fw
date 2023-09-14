#pragma once
#include <esp_err.h>

// #define DISPLAY_DAY 1
// #define DISPLAY_NIGHT 2
// #define DISPLAY_DUSK 3
// #define DISPLAY_DAWN 4

enum time_of_day_t : uint8_t { DAY =1, NIGHT =2, DUSK =3, DAWN =4};


class DISPLAY
{
    public:
        DISPLAY();
        esp_err_t setup();
        esp_err_t timer(uint32_t time);
        void operate();
        esp_err_t update(time_of_day_t time_of_day);
};