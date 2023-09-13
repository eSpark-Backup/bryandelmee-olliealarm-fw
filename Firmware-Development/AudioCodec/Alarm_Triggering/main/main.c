#include <stdio.h>
#include "es8388.h"
#include "PCA9536.h"
#include <driver/i2s.h>
#include <driver/i2s_std.h>
#include "alarm_config.h"
// #include <time.h>
// #include "esp_netif_sntp.h"

/*SPIFFS*/
#include "esp_spiffs.h"
#include "freertos/timers.h"

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_http_client.h"
#include "esp_spiffs.h"
#include "nvs_flash.h"
#include <dirent.h>
#include <string.h>


static const char *TAG = "TEST_AUDIO_HAL";

#define AUDIO_BUFFER 2048                       // buffer size for reading the wav file and sending to i2s
#define WAV_FILE "/spiffs/audio/downloaded.wav"      // wav file to play
// handles
i2s_chan_handle_t tx_handle;
esp_err_t i2s_setup(void);
esp_err_t play_wav(char *fp);

/* Download WAV */
static FILE *wav_file = NULL;
static EventGroupHandle_t wifi_event_group;
const int CONNECTED_BIT = BIT0;
void wifi_init_sta();
esp_err_t http_event_handler(esp_http_client_event_t *evt);
esp_err_t wifi_event_handler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data);
void download_wav_file(const char *url);
void alarm_task(void * alarm);

void app_main(void)
{
    esp_vfs_spiffs_conf_t config = {
      .base_path = "/spiffs",
      .partition_label = NULL,
      .max_files = 5,
      .format_if_mount_failed = true,
    };
    esp_vfs_spiffs_register(&config);

    ESP_LOGI(TAG, "Initializing WiFi...");
    wifi_init_sta();
    
    // Replace with the actual URL of your WAV file
    const char *wavFileUrl = "https://drive.usercontent.google.com/download?id=1SvWTjg3JKd3-gA58_XU2aUAVbBsTlC_N&export=download&authuser=0&confirm=t&uuid=b97fa8d5-c617-447c-b6a2-270a37bd52b9&at=APZUnTXJK7axgoVfpy_5L03zyBlp:1692269869256";


    ESP_LOGI(TAG, "Downloading WAV file...");
    // download_wav_file(wavFileUrl); //commenting out after first download
    init_time();
    sync_time();

    /* NEW ALARM */
    static alarm_config test_alarm;
    test_alarm.audio_fptr = play_wav;  //play_wav function pointer
    time_t current_time_epoch = init_time();
    time_t start_time_epoch = current_time_epoch + 60;
    time_t end_time_epoch = start_time_epoch + 30;
    memcpy(&(test_alarm.file_name), WAV_FILE, sizeof(WAV_FILE));  //copying the file destination
    set_alarm_start_time(&test_alarm, start_time_epoch);  //setting alarm's start time 
    set_alarm_end_time(&test_alarm, end_time_epoch); //setting alarm's end time

    DIR* dir = opendir("/spiffs/audio");
    if (dir == NULL) {
        return;
    }

    while (true) {
        struct dirent* de = readdir(dir);
        if (!de) {
            break;
        }

        printf("Found file: %s\n", de->d_name);
    }

    pca9536_i2c_master_init();
    pca9536_set_pin_mode_output(PCA9536_IO1);
    pca9536_set_pin_high(PCA9536_IO1);
    pca9536_i2c_deinit();
    vTaskDelay(1000/ portTICK_PERIOD_MS);

    ESP_LOGI(TAG, "Initialize i2s");
    i2s_setup();

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

    esp_err_t ret;

    ret = es8388_init(&es8388_cfg, GPIO_NUM_40, GPIO_NUM_41);
    ESP_LOGI(TAG, "es8388_init = %s", esp_err_to_name(ret));
    es8388_config_i2s(es8388_cfg.codec_mode, &es8388_cfg.i2s_iface);
    es8388_ctrl_state(AUDIO_HAL_CODEC_MODE_DECODE, AUDIO_HAL_CTRL_START);
    es8388_set_voice_volume(0);

    size_t bytes_written = 0;
    ESP_LOGI(TAG, "Start to play music");
    xTaskCreate(alarm_task,"alarm trigger", 2*2048, &test_alarm, 10, NULL);
    while (1)
    {   
        ESP_LOGI(TAG, "Main Loop Task");
        vTaskDelay(1000/portTICK_PERIOD_MS);
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
    fclose(wav_file);
  wav_file = fopen(fp, "rb");
  if (wav_file == NULL)
  {
    ESP_LOGE(TAG, "Failed to open file");
    return ESP_ERR_INVALID_ARG;
  }

  // skip the header...
  fseek(wav_file, 44, SEEK_SET);

  // create a writer buffer
  int16_t *buf = calloc(AUDIO_BUFFER, sizeof(int16_t));
  size_t bytes_read = 0;
  size_t bytes_written = 0;

  bytes_read = fread(buf, sizeof(int16_t), AUDIO_BUFFER, wav_file);
  i2s_channel_enable(tx_handle);

  while (bytes_read > 0)
  {
    // write the buffer to the i2s
    i2s_channel_write(tx_handle, buf, bytes_read * sizeof(int16_t), &bytes_written, portMAX_DELAY);
    bytes_read = fread(buf, sizeof(int16_t), AUDIO_BUFFER, wav_file);
    ESP_LOGI(TAG, "Bytes read: %d", bytes_read);
  }

  i2s_channel_disable(tx_handle);
  free(buf);

  return ESP_OK;
}

void wifi_init_sta() {

    nvs_flash_init();
    wifi_event_group = xEventGroupCreate();
    esp_netif_init();
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = "eSpark Wifi",
            .password = "47356880",
        },
    };
    esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL);
    esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL);

    ESP_LOGI(TAG, "Setting WiFi configuration SSID %s...", wifi_config.sta.ssid);
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "Waiting for WiFi connection...");
    xEventGroupWaitBits(wifi_event_group, CONNECTED_BIT, false, false, portMAX_DELAY);
    ESP_LOGI(TAG, "Connected to WiFi");
}

/* .WAV Downloading Functions */
void download_wav_file(const char *url) {
    wav_file = fopen("/spiffs/audio/downloaded.wav", "wb");
    if (!wav_file) {
        ESP_LOGE(TAG, "Failed to open file for writing");
        return;
    }

    esp_http_client_config_t config = {
        .url = url,
        .event_handler = http_event_handler,
    };
    
    esp_http_client_handle_t client = esp_http_client_init(&config);
    esp_err_t err = esp_http_client_perform(client);

    if (err != ESP_OK) {
        ESP_LOGE(TAG, "HTTP request failed: %s", esp_err_to_name(err));
    }

    esp_http_client_cleanup(client);
}

esp_err_t wifi_event_handler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data) {
    static int retry = 0; 
    if(event_base == WIFI_EVENT){
        switch (event_id)
        {
            case WIFI_EVENT_STA_START:
                ESP_LOGI(TAG,"esp_wifi_connect");
                esp_wifi_connect();
                break;

            case WIFI_EVENT_STA_DISCONNECTED:
                ESP_LOGI(TAG, "Wi-Fi disconnected, Retry %d", retry);
                xEventGroupClearBits(wifi_event_group, CONNECTED_BIT);
                esp_wifi_connect();
                if(retry == 30)
                {  
                    // xTaskCreate(prov_reinit,"prov_reinit_task", 2*2048, NULL, 10, NULL);
                    retry=0;
                    break;
                }
                retry++;
                break;

            default:
                break;
        }
    }

    if(event_base == IP_EVENT){
        switch (event_id)
        {
        case IP_EVENT_STA_GOT_IP:
            xEventGroupSetBits(wifi_event_group, CONNECTED_BIT);
            ESP_LOGI(TAG, "Wi-Fi connected");
            retry=0;
            break;
        
        default:
            break;
        }
    }
    return ESP_OK;
}

esp_err_t http_event_handler(esp_http_client_event_t *evt) {
    static size_t file_size;
    switch(evt->event_id) {
        case HTTP_EVENT_ON_DATA:
            if (!wav_file) {
                ESP_LOGE(TAG, "File not opened for writing");
                return ESP_FAIL;
            }
            fwrite(evt->data, 1, evt->data_len, wav_file);
            file_size += evt->data_len;
            ESP_LOGI(TAG,"downloading : %d", file_size);
            break;
        case HTTP_EVENT_ON_FINISH:
            if (wav_file) {
                // fclose(wav_file);
                // wav_file = NULL;
                ESP_LOGI(TAG, "File download complete");
                ESP_LOGI(TAG, "File Size : %d bytes", file_size);
            }
            break;
        default:
            break;
    }
    return ESP_OK;
}

void alarm_task(void * alarm){
    alarm_config* this_alarm = (alarm_config*)alarm;
    while(1)
    {
        if(alarm_ring_time(alarm)){
            ESP_LOGI(TAG, "alarm started");
            es8388_set_voice_volume(80);
            ring_alarm(alarm);
            es8388_set_voice_volume(0);
            vTaskDelete(NULL);
            ESP_LOGI(TAG, "alarm ended");
        }
        else{
            es8388_set_voice_volume(0);
            ESP_LOGI(TAG, "waiting for alarm");
        }
        ESP_LOGI(TAG, "In Alarm Task");
        show_alarm_info(this_alarm);
        vTaskDelay(1000/portTICK_PERIOD_MS);
    }
}