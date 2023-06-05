#include <stdio.h>
#include "es8388.h"
#include "PCA9536.h"
#include <driver/i2s.h>

#define TEST_I2S_NUM  I2S_NUM_0

static const char *TAG = "TEST_AUDIO_HAL";

extern const uint8_t test_pcm_start[] asm("_binary_test_pcm_start");
extern const uint8_t test_pcm_end[]   asm("_binary_test_pcm_end");
static void i2s_deinit();
static void i2s_init();

void app_main(void)
{

    pca9536_i2c_master_init();
    pca9536_set_pin_mode_output(PCA9536_IO1);
    pca9536_set_pin_high(PCA9536_IO1);
    pca9536_i2c_deinit();
    vTaskDelay(1000/ portTICK_PERIOD_MS);

    ESP_LOGI(TAG, "Initialize i2s");
    i2s_init();

    ESP_LOGI(TAG, "Start es8388 codec chip");
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
    es8388_init(&es8388_cfg, GPIO_NUM_40, GPIO_NUM_41);
    es8388_config_i2s(es8388_cfg.codec_mode, &es8388_cfg.i2s_iface);
    es8388_ctrl_state(AUDIO_HAL_CODEC_MODE_DECODE, AUDIO_HAL_CTRL_START);
    es8388_set_voice_volume(50);

    size_t bytes_written = 0;
    ESP_LOGI(TAG, "Start to play music");
    i2s_write(TEST_I2S_NUM, test_pcm_start, test_pcm_end - test_pcm_start, &bytes_written, portMAX_DELAY);

    ESP_LOGW(TAG, "Reach the end of music, release all resource");
    i2c_driver_delete(I2C_NUM_0);
    i2s_deinit();
}


static void i2s_init()
{
    i2s_config_t i2s_cfg = {
        .mode = I2S_MODE_MASTER | I2S_MODE_TX | I2S_MODE_RX,
        .sample_rate = 16000,
        .bits_per_sample = 16,
        .channel_format = I2S_CHANNEL_FMT_ONLY_RIGHT,
        .communication_format = I2S_COMM_FORMAT_I2S,
        .dma_buf_count = 3,
        .dma_buf_len = 300,
        .use_apll = 1,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL2,
    };

    i2s_driver_install(TEST_I2S_NUM, &i2s_cfg, 0, NULL);
    i2s_pin_config_t i2s_pin_cfg = {
        .mck_io_num = GPIO_NUM_42,
        .bck_io_num = GPIO_NUM_46,
        .ws_io_num = GPIO_NUM_45,
        .data_out_num = GPIO_NUM_37,
        .data_in_num = GPIO_NUM_36,
    };
    // get_i2s_pins(TEST_I2S_NUM, &i2s_pin_cfg);
    i2s_set_pin(TEST_I2S_NUM, &i2s_pin_cfg);
    // i2s_mclk_gpio_select(TEST_I2S_NUM, GPIO_NUM_0);
}

static void i2s_deinit()
{
    i2s_driver_uninstall(TEST_I2S_NUM);
}
