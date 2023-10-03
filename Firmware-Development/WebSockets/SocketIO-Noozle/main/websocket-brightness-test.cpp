#include <stdio.h>
#include "esp_log.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "nvs_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_websocket_client.h"
#include "wifi.hpp"
#include "sio_client.h"
#include "driver/ledc.h"
#include <cJSON.h>


#define TAG "Main"
void eventHandlerSIO(void *arg, esp_event_base_t eventBase, int32_t eventID, void *eventData);
uint32_t socket_brightness = 1023;

extern "C" void app_main(void)
{
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    WIFI wifi = WIFI();
    wifi.setup();
    wifi.setConfig("eSpark Wifi", "47356880");
    wifi.run();
    wifi.wait();
    
    esp_event_handler_instance_register(SIO_EVENT, ESP_EVENT_ANY_ID, &eventHandlerSIO, NULL, NULL);
    sio_client_config_t sioConfig;
    sioConfig.eio_version = 4;
    sioConfig.transport = SIO_TRANSPORT_POLLING;
    sioConfig.server_address = "dev.noozle.nl:3000";
    sioConfig.sio_url_path = "/socket.io";
    sioConfig.nspc = "/";
    sioConfig.alloc_auth_body_cb = NULL;

    sio_client_id_t sioID = sio_client_init(&sioConfig);
    esp_err_t ret = sio_client_begin(sioID);
    ESP_LOGI(TAG, "%d", ret);

    /*setup gpio for brightnes*/
    ledc_timer_config_t led_config = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .duty_resolution =  LEDC_TIMER_10_BIT,
        .timer_num = LEDC_TIMER_0,
        .freq_hz = 20000

    };
    ledc_timer_config(&led_config);
    ledc_channel_config_t led_chan_config = {
        .gpio_num = 38,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = LEDC_CHANNEL_0

    };
    ledc_channel_config(&led_chan_config);
    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, 1023);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
    while(true){
        ESP_LOGI(TAG,"WHILE LOOP");
        ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, socket_brightness);
        ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
        vTaskDelay(1000/portTICK_PERIOD_MS);
    }
}

void eventHandlerSIO(void *arg, esp_event_base_t eventBase, int32_t eventID, void *eventData)
{
    sio_event_data_t* sioData = (sio_event_data_t*)eventData;

    switch (eventID)
    {
        case SIO_PACKET_CONNECT:
            ESP_LOGI(TAG,"SIO_PACKET_CONNECT");
            break;

        case SIO_PACKET_DISCONNECT:
            ESP_LOGI(TAG,"SIO_PACKET_DISCONNECT");
            break;

        default:
            ESP_LOGI(TAG,"SIO : %ld", eventID);
            PacketPointerArray_t packet = sioData->packets_pointer;
            ESP_LOGI(TAG,"SIO : %s", (*packet)->data);
            parse_packet(*packet);
            ESP_LOGI(TAG,"json recieved : %s", (*packet)->json_start);
            
            /*parse json packet*/
            cJSON *topic;
            cJSON *data;
            char json_string[80] ;

            strcpy(json_string, (*packet)->json_start);
            ESP_LOGI(TAG,"\njson_string copied : %s\n", json_string);

            cJSON *root = cJSON_Parse(json_string);
            char *out = cJSON_Print(root);
            topic = cJSON_GetObjectItem(root, "ollie_id");
            data = cJSON_GetObjectItem(root, "brightness_value");
            if(topic != NULL && data != NULL){
                printf("Olli id : %d\n", topic->valueint);
                printf("brightness value : %f\n", data->valuedouble);
                socket_brightness = data->valuedouble * 1023;
            }


            break;
    }
}