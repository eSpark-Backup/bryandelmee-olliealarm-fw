#include <stdio.h>
#include "PCA9536.h"
#include "es8388.h"
static const char* TAG = "MAIN";


/*SPIFFS*/
#include "esp_spiffs.h"
#include "freertos/timers.h"

#define WAV_FILE "/spiffs/audio/Piano.wav"      // wav file to play
// handles
i2s_chan_handle_t tx_handle;
esp_err_t i2s_setup(void);
esp_err_t play_wav(char *fp);

void app_main(void)
{
    esp_vfs_spiffs_conf_t config = {
      .base_path = "/spiffs",
      .partition_label = NULL,
      .max_files = 5,
      .format_if_mount_failed = true,
    };
    esp_vfs_spiffs_register(&config);

    pca9536_i2c_master_init();
    pca9536_set_pin_mode_output(PCA9536_IO1);
    pca9536_set_pin_high(PCA9536_IO1);
    pca9536_i2c_deinit();
    vTaskDelay(1000/ portTICK_PERIOD_MS);

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

    esp_err_t ret;
    ret = es8388_init(&es8388_cfg, GPIO_NUM_40, GPIO_NUM_41);
    ESP_LOGI(TAG, "es8388_init = %s", esp_err_to_name(ret));
    es8388_config_i2s(es8388_cfg.codec_mode, &es8388_cfg.i2s_iface);
    es8388_ctrl_state(AUDIO_HAL_CODEC_MODE_DECODE, AUDIO_HAL_CTRL_START);
    es8388_set_voice_volume(90);

    es_i2s_setup();
    ESP_LOGI(TAG, "Start to play music");
    while (1)
    {   
        es_i2s_play_wav_from_spiffs(WAV_FILE);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
    
}

esp_err_t i2s_setup(void)
{
  // setup a standard config and the channel
  i2s_chan_config_t chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_AUTO, I2S_ROLE_MASTER);
  ESP_ERROR_CHECK(i2s_new_channel(&chan_cfg, &tx_handle, NULL));

  // setup the i2s config
  i2s_std_config_t std_cfg = {
      .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(44100),                                                    // the wav file sample rate
      .slot_cfg = I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_STEREO), // the wav faile bit and channel config
      .gpio_cfg = {
          // refer to configuration.h for pin setup
          .mclk = GPIO_NUM_42,
          .bclk = GPIO_NUM_46,
          .ws = GPIO_NUM_45,
          .dout = GPIO_NUM_43,
          .din = GPIO_NUM_44,
          .invert_flags = {
              .mclk_inv = false,
              .bclk_inv = false,
              .ws_inv = false,
          },
      },
  };
  return i2s_channel_init_std_mode(tx_handle, &std_cfg);
}

esp_err_t play_wav(char *fp)
{
  FILE *fh = fopen(fp, "rb");
  if (fh == NULL)
  {
    ESP_LOGE(TAG, "Failed to open file");
    return ESP_ERR_INVALID_ARG;
  }

  // skip the header...
  fseek(fh, 44, SEEK_SET);

  // create a writer buffer
  int16_t *buf = calloc(AUDIO_BUFFER, sizeof(int16_t));
  size_t bytes_read = 0;
  size_t bytes_written = 0;

  bytes_read = fread(buf, sizeof(int16_t), AUDIO_BUFFER, fh);

  i2s_channel_enable(tx_handle);

  while (bytes_read > 0)
  {
    // write the buffer to the i2s
    i2s_channel_write(tx_handle, buf, bytes_read * sizeof(int16_t), &bytes_written, portMAX_DELAY);
    bytes_read = fread(buf, sizeof(int16_t), AUDIO_BUFFER, fh);
    ESP_LOGV(TAG, "Bytes read: %d", bytes_read);
  }

  i2s_channel_disable(tx_handle);
  free(buf);
  fclose(fh);

  return ESP_OK;
}
