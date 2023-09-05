#pragma once

#include "esp_check.h"

class codec
{
    public:
        codec();
        esp_err_t setup();
        esp_err_t play(const char* wav);
        esp_err_t setVol(uint8_t vol);
};