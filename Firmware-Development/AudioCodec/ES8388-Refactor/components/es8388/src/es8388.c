#include "es8388.h"

static const char *ES_TAG = "ES8388_DRIVER";

esp_err_t es8388_init(audio_hal_codec_config_t *cfg, int sda_pin, int scl_pin)
{
    esp_err_t res;
    res = es_i2c_init(sda_pin, scl_pin); // ESP32 in master mode

    /* DAC mute and disabled digital volume control soft ramp */
    es_write_reg(ES8388_ADDR, ES8388_DACCONTROL3, 0x04);  

    /* Chip Control and Power Management */
    es_write_reg(ES8388_ADDR, ES8388_CONTROL2, 0x50);
    es_write_reg(ES8388_ADDR, ES8388_CHIPPOWER, 0x00); //normal all and power up all

    // Disable the internal DLL to improve 8K sample rate
    es_write_reg(ES8388_ADDR, 0x35, 0xA0);
    es_write_reg(ES8388_ADDR, 0x37, 0xD0);
    es_write_reg(ES8388_ADDR, 0x39, 0xD0);

    /* Configure ES8388 IN I2S MASTER/SLAVE MODE */
    es_write_reg(ES8388_ADDR, ES8388_MASTERMODE, cfg->i2s_iface.mode); 

    /* DAC */
    es_write_reg(ES8388_ADDR, ES8388_DACPOWER, 0xC0);       //disable DAC and disable Lout/Rout/1/2
    es_write_reg(ES8388_ADDR, ES8388_CONTROL1, 0x12);       //Enfr=0,Play&Record Mode,(0x17-both of mic&paly)
    es_write_reg(ES8388_ADDR, ES8388_DACCONTROL1, 0x18);    //1a 0x18:16bit iis , 0x00:24
    es_write_reg(ES8388_ADDR, ES8388_DACCONTROL2, 0x02);    //DACFsMode,SINGLE SPEED; DACFsRatio,256
    es_write_reg(ES8388_ADDR, ES8388_DACCONTROL16, 0x00);   // 0x00 audio on LIN1&RIN1,  0x09 LIN2&RIN2
    es_write_reg(ES8388_ADDR, ES8388_DACCONTROL17, 0x90);   // only left DAC to left mixer enable 0db
    es_write_reg(ES8388_ADDR, ES8388_DACCONTROL20, 0x90);   // only right DAC to right mixer enable 0db
    es_write_reg(ES8388_ADDR, ES8388_DACCONTROL21, 0x80);   // set internal ADC and DAC use the same LRCK clock, ADC LRCK as internal LRCK
    es_write_reg(ES8388_ADDR, ES8388_DACCONTROL23, 0x00);   // vroi=0

    es_write_reg(ES8388_ADDR, ES8388_DACCONTROL24, 0x1E);   // Set L1 R1 L2 R2 volume. 0x00: -30dB, 0x1E: 0dB, 0x21: 3dB
    es_write_reg(ES8388_ADDR, ES8388_DACCONTROL25, 0x1E);
    es_write_reg(ES8388_ADDR, ES8388_DACCONTROL26, 0);
    es_write_reg(ES8388_ADDR, ES8388_DACCONTROL27, 0);
    es8388_set_adc_dac_volume(ES_MODULE_DAC, 90, 0);       // 0db
    int tmp = 0;
    if (AUDIO_HAL_DAC_OUTPUT_LINE2 == cfg->dac_output) {
        tmp = DAC_OUTPUT_LOUT1 | DAC_OUTPUT_ROUT1;
    } else if (AUDIO_HAL_DAC_OUTPUT_LINE1 == cfg->dac_output) {
        tmp = DAC_OUTPUT_LOUT2 | DAC_OUTPUT_ROUT2;
    } else {
        tmp = DAC_OUTPUT_LOUT1 | DAC_OUTPUT_LOUT2 | DAC_OUTPUT_ROUT1 | DAC_OUTPUT_ROUT2;
    }
    es_write_reg(ES8388_ADDR, ES8388_DACPOWER, tmp);  //0x3c Enable DAC and Enable Lout/Rout/1/2

    /* ADC */
    es_write_reg(ES8388_ADDR, ES8388_ADCPOWER, 0xFF);    // Power off ADC

    return res;
}

esp_err_t es8388_config_i2s(audio_hal_codec_mode_t mode, audio_hal_codec_i2s_iface_t *iface)
{
    esp_err_t res = ESP_OK;
    int tmp = 0;
    res |= es8388_config_fmt(mode, iface->fmt);
    if (iface->bits == AUDIO_HAL_BIT_LENGTH_16BITS) {
        tmp = BIT_LENGTH_16BITS;
    } else if (iface->bits == AUDIO_HAL_BIT_LENGTH_24BITS) {
        tmp = BIT_LENGTH_24BITS;
    } else {
        tmp = BIT_LENGTH_32BITS;
    }
    res |= es8388_set_bits_per_sample(ES_MODULE_ADC_DAC, tmp);
    return res;
}

/**
 * @brief Configure ES8388 I2S format
 *
 * @param mode:           set ADC or DAC or all
 * @param bitPerSample:   see Es8388I2sFmt
 *
 * @return
 *     - (-1) Error
 *     - (0)  Success
 */
esp_err_t es8388_config_fmt(es_module_t mode, es_i2s_fmt_t fmt)
{
    esp_err_t res = ESP_OK;
    uint8_t reg = 0;
    if (mode == ES_MODULE_ADC || mode == ES_MODULE_ADC_DAC) {
        res = es_read_reg(ES8388_ADCCONTROL4, &reg);
        reg = reg & 0xfc;
        res |= es_write_reg(ES8388_ADDR, ES8388_ADCCONTROL4, reg | fmt);
    }
    if (mode == ES_MODULE_DAC || mode == ES_MODULE_ADC_DAC) {
        res = es_read_reg(ES8388_DACCONTROL1, &reg);
        reg = reg & 0xf9;
        res |= es_write_reg(ES8388_ADDR, ES8388_DACCONTROL1, reg | (fmt << 1));
    }
    return res;
}

int es8388_ctrl_state(audio_hal_codec_mode_t mode, audio_hal_ctrl_t ctrl_state)
{
    int res = 0;
    int es_mode_t = 0;
    switch (mode) {
        case AUDIO_HAL_CODEC_MODE_ENCODE:
            es_mode_t  = ES_MODULE_ADC;
            break;
        case AUDIO_HAL_CODEC_MODE_LINE_IN:
            es_mode_t  = ES_MODULE_LINE;
            break;
        case AUDIO_HAL_CODEC_MODE_DECODE:
            es_mode_t  = ES_MODULE_DAC;
            break;
        case AUDIO_HAL_CODEC_MODE_BOTH:
            es_mode_t  = ES_MODULE_ADC_DAC;
            break;
        default:
            es_mode_t = ES_MODULE_DAC;
            ESP_LOGW(ES_TAG, "Codec mode not support, default is decode mode");
            break;
    }
    if (AUDIO_HAL_CTRL_STOP == ctrl_state) {
        res = es8388_stop(es_mode_t);
    } else {
        res = es8388_start(es_mode_t);
        ESP_LOGD(ES_TAG, "start default is decode mode:%d", es_mode_t);
    }
    return res;
}

/**
 * @brief Power Management
 *
 * @param mod:      if ES_POWER_CHIP, the whole chip including ADC and DAC is enabled
 * @param enable:   false to disable true to enable
 *
 * @return
 *     - (-1)  Error
 *     - (0)   Success
 */
esp_err_t es8388_start(es_module_t mode)
{
    esp_err_t res = ESP_OK;
    uint8_t prev_data = 0, data = 0;
    es_read_reg(ES8388_DACCONTROL21, &prev_data);
    if (mode == ES_MODULE_LINE) {
        res |= es_write_reg(ES8388_ADDR, ES8388_DACCONTROL16, 0x09); // 0x00 audio on LIN1&RIN1,  0x09 LIN2&RIN2 by pass enable
        res |= es_write_reg(ES8388_ADDR, ES8388_DACCONTROL17, 0x50); // left DAC to left mixer enable  and  LIN signal to left mixer enable 0db  : bupass enable
        res |= es_write_reg(ES8388_ADDR, ES8388_DACCONTROL20, 0x50); // right DAC to right mixer enable  and  LIN signal to right mixer enable 0db : bupass enable
        res |= es_write_reg(ES8388_ADDR, ES8388_DACCONTROL21, 0xC0); //enable adc
    } else {
        res |= es_write_reg(ES8388_ADDR, ES8388_DACCONTROL21, 0x80);   //enable dac
    }
    es_read_reg(ES8388_DACCONTROL21, &data);
    if (prev_data != data) {
        res |= es_write_reg(ES8388_ADDR, ES8388_CHIPPOWER, 0xF0);   //start state machine
        // res |= es_write_reg(ES8388_ADDR, ES8388_CONTROL1, 0x16);
        // res |= es_write_reg(ES8388_ADDR, ES8388_CONTROL2, 0x50);
        res |= es_write_reg(ES8388_ADDR, ES8388_CHIPPOWER, 0x00);   //start state machine
    }
    if (mode == ES_MODULE_ADC || mode == ES_MODULE_ADC_DAC || mode == ES_MODULE_LINE) {
        res |= es_write_reg(ES8388_ADDR, ES8388_ADCPOWER, 0x00);   //power up adc and line in
    }
    if (mode == ES_MODULE_DAC || mode == ES_MODULE_ADC_DAC || mode == ES_MODULE_LINE) {
        res |= es_write_reg(ES8388_ADDR, ES8388_DACPOWER, 0x3c);   //power up dac and line out
        res |= es8388_set_voice_mute(false);
        ESP_LOGD(ES_TAG, "es8388_start default is mode:%d", mode);
    }

    return res;
}

/**
 * @brief Power Management
 *
 * @param mod:      if ES_POWER_CHIP, the whole chip including ADC and DAC is enabled
 * @param enable:   false to disable true to enable
 *
 * @return
 *     - (-1)  Error
 *     - (0)   Success
 */
esp_err_t es8388_stop(es_module_t mode)
{
    esp_err_t res = ESP_OK;
    if (mode == ES_MODULE_LINE) {
        res |= es_write_reg(ES8388_ADDR, ES8388_DACCONTROL21, 0x80); //enable dac
        res |= es_write_reg(ES8388_ADDR, ES8388_DACCONTROL16, 0x00); // 0x00 audio on LIN1&RIN1,  0x09 LIN2&RIN2
        res |= es_write_reg(ES8388_ADDR, ES8388_DACCONTROL17, 0x90); // only left DAC to left mixer enable 0db
        res |= es_write_reg(ES8388_ADDR, ES8388_DACCONTROL20, 0x90); // only right DAC to right mixer enable 0db
        return res;
    }
    if (mode == ES_MODULE_DAC || mode == ES_MODULE_ADC_DAC) {
        res |= es_write_reg(ES8388_ADDR, ES8388_DACPOWER, 0x00);
        res |= es8388_set_voice_mute(true); //res |= Es8388SetAdcDacVolume(ES_MODULE_DAC, -96, 5);      // 0db
        //res |= es_write_reg(ES8388_ADDR, ES8388_DACPOWER, 0xC0);  //power down dac and line out
    }
    if (mode == ES_MODULE_ADC || mode == ES_MODULE_ADC_DAC) {
        //res |= Es8388SetAdcDacVolume(ES_MODULE_ADC, -96, 5);      // 0db
        res |= es_write_reg(ES8388_ADDR, ES8388_ADCPOWER, 0xFF);  //power down adc and line in
    }
    if (mode == ES_MODULE_ADC_DAC) {
        res |= es_write_reg(ES8388_ADDR, ES8388_DACCONTROL21, 0x9C);  //disable mclk
//        res |= es_write_reg(ES8388_ADDR, ES8388_CONTROL1, 0x00);
//        res |= es_write_reg(ES8388_ADDR, ES8388_CONTROL2, 0x58);
//        res |= es_write_reg(ES8388_ADDR, ES8388_CHIPPOWER, 0xF3);  //stop state machine
    }

    return res;
}

/**
 * @brief Configure ES8388 ADC and DAC volume. Basicly you can consider this as ADC and DAC gain
 *
 * @param mode:             set ADC or DAC or all
 * @param volume:           -96 ~ 0              for example Es8388SetAdcDacVolume(ES_MODULE_ADC, 30, 6); means set ADC volume -30.5db
 * @param dot:              whether include 0.5. for example Es8388SetAdcDacVolume(ES_MODULE_ADC, 30, 4); means set ADC volume -30db
 *
 * @return
 *     - (-1) Parameter error
 *     - (0)   Success
 */
int es8388_set_adc_dac_volume(int mode, int volume, int dot)
{
    int res = 0;
    if ( volume < -96 || volume > 100 ) {
        ESP_LOGW(ES_TAG, "Warning: volume < -96! or > 0!\n");
        if (volume < -96)
            volume = -96;
        else
            volume = 100;
    }
    dot = (dot >= 5 ? 1 : 0);
    volume = (-volume << 1) + dot;
    if (mode == ES_MODULE_ADC || mode == ES_MODULE_ADC_DAC) {
        res |= es_write_reg(ES8388_ADDR, ES8388_ADCCONTROL8, volume);
        res |= es_write_reg(ES8388_ADDR, ES8388_ADCCONTROL9, volume);  //ADC Right Volume=0db
    }
    if (mode == ES_MODULE_DAC || mode == ES_MODULE_ADC_DAC) {
        res |= es_write_reg(ES8388_ADDR, ES8388_DACCONTROL5, volume);
        res |= es_write_reg(ES8388_ADDR, ES8388_DACCONTROL4, volume);
    }
    return res;
}

/**
 * @brief Set voice volume
 *
 * @note Register values. 0xC0: -96 dB, 0x64: -50 dB, 0x00: 0 dB
 * @note Accuracy of gain is 0.5 dB
 *
 * @param volume: voice volume (0~100)
 *
 * @return
 *     - ESP_OK
 *     - ESP_FAIL
 */
esp_err_t es8388_set_voice_volume(int volume)
{
    esp_err_t res = ESP_OK;
    uint8_t reg = 0;
    reg = es8388_get_dac_reg_value(volume);
    res |= es_write_reg(ES8388_ADDR, ES8388_DACCONTROL5, reg);
    res |= es_write_reg(ES8388_ADDR, ES8388_DACCONTROL4, reg);

    return res;
}

/**
 * @brief Configure ES8388 DAC mute or not. Basically you can use this function to mute the output or unmute
 *
 * @param enable: enable or disable
 *
 * @return
 *     - (-1) Parameter error
 *     - (0)   Success
 */
esp_err_t es8388_set_voice_mute(bool enable)
{
    esp_err_t res = ESP_OK;
    uint8_t reg = 0;
    res = es_read_reg(ES8388_DACCONTROL3, &reg);
    reg = reg & 0xFB;
    res |= es_write_reg(ES8388_ADDR, ES8388_DACCONTROL3, reg | (((int)enable) << 2));
    return res;
}

/**
 * @brief Get DAC volume offset from user set volume, you can use an array or function to finish this map
 *
 * @note The max DAC volume is 0 dB when the user volume is 100. 0 dB means there is no attenuation of the sound source,
 *       and it is the original sound source. It can not exceed 0 dB. Otherwise, there is a risk of clipping noise.
 * @note For better audio dynamic range, we'd better use 0dB full scale digital gain and lower analog gain.
 * @note DAC volume offset is positively correlated with the user volume.
 *
 * @param volume User set volume (1-100)
 *
 * @return
 *     - Codec DAC volume offset. The max value must be 0 dB.
 */

static inline float es8388_get_dac_volume_offset(int volume)
{
    float offset = dac_volume_offset[volume - 1] / 2.0;
    return offset;
}

/**
 * @brief The register value is linear to the dac_volume
 */
static inline uint8_t es8388_calculate_dac_vol_reg(float dac_volume)
{
    uint8_t reg = (uint8_t) (dac_volume / (DAC_VOL_SYMBOL * VOL_ACCURACY) + ZERO_VOL_REG);
    return reg;
}

uint8_t es8388_get_dac_reg_value(int volume)
{
    float dac_volume = 0;
    int user_volume = volume;
    uint8_t reg_value;

    if (user_volume < 0) {
        user_volume = 0;
    } else if (user_volume > 100) {
        user_volume = 100;
    }

    if (user_volume == 0) {
        dac_volume = MIN_DAC_VOLUME; // Make sure the speaker voice is near silent
    } else {
        /*
         * For better audio performance, at the max volume, we need to ensure:
         * Audio Process Gain + Codec DAC Volume + PA Gain <= MAX_GAIN.
         * The PA Gain and Audio Process Gain are known when the board design is fixed, so
         * max Codec DAC Volume = MAX_GAIN - PA Gain - Audio Process Gain，then
         * the volume mapping table shift accordingly.
         */
        dac_volume = es8388_get_dac_volume_offset(user_volume) + MAX_GAIN - PA_GAIN;
        dac_volume = dac_volume < MAX_DAC_VOLUME ? dac_volume : MAX_DAC_VOLUME;
    }
    ESP_LOGI(ES_TAG,"dac_volume = %f",dac_volume);
    reg_value = es8388_calculate_dac_vol_reg(dac_volume);
    return reg_value;
}


/**
 * @brief Configure ES8388 data sample bits
 *
 * @param mode:             set ADC or DAC or all
 * @param bitPerSample:   see BitsLength
 *
 * @return
 *     - (-1) Parameter error
 *     - (0)   Success
 */
esp_err_t es8388_set_bits_per_sample(es_module_t mode, es_bits_length_t bits_length)
{
    esp_err_t res = ESP_OK;
    uint8_t reg = 0;
    int bits = (int)bits_length;

    if (mode == ES_MODULE_ADC || mode == ES_MODULE_ADC_DAC) {
        res = es_read_reg(ES8388_ADCCONTROL4, &reg);
        reg = reg & 0xe3;
        res |=  es_write_reg(ES8388_ADDR, ES8388_ADCCONTROL4, reg | (bits << 2));
    }
    if (mode == ES_MODULE_DAC || mode == ES_MODULE_ADC_DAC) {
        res = es_read_reg(ES8388_DACCONTROL1, &reg);
        reg = reg & 0xc7;
        res |= es_write_reg(ES8388_ADDR, ES8388_DACCONTROL1, reg | (bits << 3));
    }
    return res;
}
