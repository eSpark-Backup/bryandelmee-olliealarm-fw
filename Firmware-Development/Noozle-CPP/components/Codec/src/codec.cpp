#include "codec.hpp"
#include "es8388.h"
#include "es8388_i2c.h"
#include "es8388_i2s.h"
#include "es8388_reg.h"
#include "es8388_volume.h"
#include "esxxx_common.h"
#include "io_expander.hpp"

static const char* TAG = "Codec";

codec::codec()
{
    
}

esp_err_t codec::setup()
{
    /* Enable ES8388 Audio Codec Chip using the GPIO-Expander */
    io_expander io = io_expander();
    io.setOutputPin(ES8388_EN_PIN);
    io.setPinHigh(ES8388_EN_PIN);
    io.deinitI2C();

    audio_hal_codec_config_t es8388_cfg = {
        .adc_input  = AUDIO_HAL_ADC_INPUT_LINE1,        
        .dac_output = AUDIO_HAL_DAC_OUTPUT_ALL,         
        .codec_mode = AUDIO_HAL_CODEC_MODE_DECODE,        
        .i2s_iface = {                                  
            .mode = AUDIO_HAL_MODE_SLAVE,               
            .fmt = AUDIO_HAL_I2S_RIGHT,                
            .samples = AUDIO_HAL_48K_SAMPLES,            
            .bits = AUDIO_HAL_BIT_LENGTH_16BITS,        
        },
    };

    ESP_RETURN_ON_ERROR(es8388_init(&es8388_cfg, ES8388_SDA_PIN, ES8388_SCL_PIN), TAG, "setup: Initialize Audio Codec");
    ESP_RETURN_ON_ERROR(es8388_config_i2s(es8388_cfg.codec_mode, &es8388_cfg.i2s_iface), TAG, "setup: Configures the codec I2S");
    ESP_RETURN_ON_ERROR(es8388_ctrl_state(AUDIO_HAL_CODEC_MODE_DECODE, AUDIO_HAL_CTRL_START), TAG, "setup: Setting up Codec Mode");
    ESP_RETURN_ON_ERROR(es8388_set_voice_volume(0), TAG, "setup: Setting Volume");
    ESP_RETURN_ON_ERROR(es_i2s_setup(), TAG, "setup: Setting up MCU I2S driver");
    return ESP_OK;
}

esp_err_t codec::play(const char* wav)
{
    ESP_RETURN_ON_ERROR(es_i2s_play_wav_from_spiffs(wav), TAG, "Play Wav file from spiffs");
    return ESP_OK;
}

esp_err_t codec::setVol(uint8_t vol)
{
    ESP_RETURN_ON_ERROR(es8388_set_voice_volume(vol), TAG, "Set Volume Level");
    return ESP_OK;
}

/**
 * @brief Enables or Disables the Codec's Enable Pin using IO Expander
 * 
 * @param true : to enable the codec
 * @param false : to disable the codec
 */
void codec::codec_en(bool en){
    io_expander io = io_expander();
    if(en){
        ESP_LOGI(TAG, "Enable");
        io.setPinHigh(1);
    }
    else{
        ESP_LOGI(TAG, "Disable");        
        io.setPinLow(1);
    }
    io.readPin(1);
    io.deinitI2C();
    // io.~io_expander();
}