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

#define TAG "Main"
void eventHandlerSIO(void *arg, esp_event_base_t eventBase, int32_t eventID, void *eventData);

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
            break;
    }
}