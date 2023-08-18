#include "es8388_i2s.h"

static const char* TAG = "es8388_i2s";
i2s_chan_handle_t es_i2s_handle;

esp_err_t es_i2s_setup()
{
  i2s_chan_config_t chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_AUTO, I2S_ROLE_MASTER);
  ESP_RETURN_ON_ERROR(i2s_new_channel(&chan_cfg, &es_i2s_handle, NULL), TAG, "i2s_new_channel");

  i2s_std_config_t std_cfg = {
      .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(44100),                                                    // the wav file sample rate
      .slot_cfg = I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_STEREO), // the wav faile bit and channel config
      .gpio_cfg = {
          .mclk = ES_I2S_MCLK,
          .bclk = ES_I2S_BCLK,
          .ws   = ES_I2S_WS,
          .dout = ES_I2S_DOUT,
          .din  = ES_I2S_DIN,
          .invert_flags = {
              .mclk_inv = false,
              .bclk_inv = false,
              .ws_inv = false,
          },
      },
  };
  ESP_RETURN_ON_ERROR(i2s_channel_init_std_mode(es_i2s_handle, &std_cfg), TAG, "i2s_channel_init_std_mode");

  return ESP_OK;
}

esp_err_t es_i2s_play_wav_from_spiffs(char *fp)
{
  FILE *fh = fopen(fp, "rb");
  if (fh == NULL)
  {
    ESP_LOGE(TAG, "Failed to open file");
    return ESP_ERR_INVALID_ARG;
  }

  // skip the wav header...
  fseek(fh, 44, SEEK_SET);

  // create a writer buffer
  int16_t *buf = calloc(AUDIO_BUFFER, sizeof(int16_t));
  size_t bytes_read = 0;
  size_t bytes_written = 0;

  bytes_read = fread(buf, sizeof(int16_t), AUDIO_BUFFER, fh);

  i2s_channel_enable(es_i2s_handle);

  while (bytes_read > 0)
  {
    // write the buffer to the i2s
    i2s_channel_write(es_i2s_handle, buf, bytes_read * sizeof(int16_t), &bytes_written, portMAX_DELAY);
    bytes_read = fread(buf, sizeof(int16_t), AUDIO_BUFFER, fh);
    ESP_LOGV(TAG, "Bytes read: %d", bytes_read);
  }

  i2s_channel_disable(es_i2s_handle);
  free(buf);
  fclose(fh);

  return ESP_OK;
}
