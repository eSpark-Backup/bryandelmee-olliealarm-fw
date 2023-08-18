#ifndef _ES8388_I2S_H__
#define _ES8388_I2S_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "freertos/FreeRTOS.h"
#include "esp_check.h"
#include <driver/i2s_std.h>
#include "driver/gpio.h"

#define ES_I2S_MCLK GPIO_NUM_42
#define ES_I2S_BCLK GPIO_NUM_46
#define ES_I2S_WS   GPIO_NUM_45
#define ES_I2S_DOUT GPIO_NUM_43
#define ES_I2S_DIN  GPIO_NUM_44

#define AUDIO_BUFFER 2048                       // buffer size for reading the wav file and sending to i2s

esp_err_t es_i2s_setup();
esp_err_t es_i2s_play_wav_from_spiffs(char *fp);

#ifdef __cplusplus
}
#endif

#endif //_ES8388_I2S_H__