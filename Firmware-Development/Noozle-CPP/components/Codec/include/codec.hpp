#pragma once

#include "esp_check.h"

#define AMP_SHUTDOWN_PIN 1 //SD pin of Audio Amplifer : system shutdown control

class codec
{
    public:
        codec();
        esp_err_t setup();
        esp_err_t play(const char* wav);
        esp_err_t setVol(uint8_t vol);
        esp_err_t codec_en(bool en);
};