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
#include <map>
#include <string>

#define TAG "Main"
void eventHandlerSIO(void *arg, esp_event_base_t eventBase, int32_t eventID, void *eventData);
uint32_t socket_brightness = 1023;

enum event_name {
    BRIGHTNESS_UPDATE, 
    ALARM_ADDED, 
    UPDATE_ALARM, 
    TESTING_MODE, 
    VOLUME_UPDATE, 
    NEW_NAP_TIME, 
    UPDATE_NAP_TIME, 
    DISCONNECT_DEVICE, 
    UNKNOWN
};
// Map to associate the strings with the enum values
static std::map<std::string, event_name> map_event_name;

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


}

void eventHandlerSIO(void *arg, esp_event_base_t eventBase, int32_t eventID, void *eventData)
{
    sio_event_data_t* sioData = (sio_event_data_t*)eventData;

    //Switch depending upon packet recieved
    switch (eventID)
    {
        case SIO_PACKET_CONNECT:
            ESP_LOGI(TAG,"SIO_PACKET_CONNECT");
            break;

        case SIO_PACKET_DISCONNECT:
            ESP_LOGI(TAG,"SIO_PACKET_DISCONNECT");
            break;

        case SIO_PACKET_CONNECT_ERROR:
            ESP_LOGI(TAG, "SIO_PACKET_CONNECT_ERROR");
            break;
            
        default:
            ESP_LOGI(TAG,"SIO : %ld", eventID);
            PacketPointerArray_t packet = sioData->packets_pointer;
            ESP_LOGI(TAG,"SIO : %s", (*packet)->data);
            parse_packet(*packet);
            ESP_LOGI(TAG,"json recieved : %s", (*packet)->json_start);
            
            /*parse json packet*/
            cJSON *ollie_id = NULL;
            cJSON *brightness_value = NULL;
            cJSON *topic, *root2;
            char json_string[100] ;

            strcpy(json_string, (*packet)->json_start);
            ESP_LOGI(TAG,"\njson_string copied : %s\n", json_string);

            cJSON *root = cJSON_Parse(json_string);
            char *topic_string ="Unknown";

            //Check if the root recieved array
            if(cJSON_IsArray(root)){
                topic = cJSON_GetArrayItem(root, 0);
                root2 = cJSON_GetArrayItem(root, 1);
            }
            else{
                topic = NULL;
                root2 = NULL;
            }
            
            //if recieved json is array
            if(topic != NULL && root2 != NULL){
                topic_string = cJSON_PrintUnformatted(topic);
                printf("TOPIC : %s\n", topic_string);

                //Remove Inverted Commas from the Topic
                for(int i = 0; i<strlen(topic_string); i++){
                    topic_string[i] = topic_string[i+1];
                }
                topic_string[strlen(topic_string)-1] = '\0';
                printf("TOPIC : %s\n", topic_string);

                //check Ollie ID
                ollie_id = cJSON_GetObjectItem(root2, "Ollie_id");
                printf("Olli id Recieved : %s\n", ollie_id->valuestring);

            }
            
            //Map the corresponding event name with the enum to be used in switch case
            map_event_name["brightness_update"] = BRIGHTNESS_UPDATE;
            map_event_name["Alarm_Added"] = ALARM_ADDED;
            map_event_name["Update_Alarm"] = UPDATE_ALARM;
            map_event_name["testing_mode"] = TESTING_MODE;
            map_event_name["volume_update"] = VOLUME_UPDATE;
            map_event_name["new_nap_time"] = NEW_NAP_TIME;
            map_event_name["update_nap_time"] = UPDATE_NAP_TIME;
            map_event_name["disconnect_device"] = DISCONNECT_DEVICE;
            map_event_name["Unknown"] = UNKNOWN;

            //check if Ollie_id json is not Null and ollie id matches with the device own ollie id
            if(ollie_id != NULL && strcmp(ollie_id->valuestring, "651ab72aa928e0a9e87be8dd") == 0 )
            {
                //This switch case respond according to the topic or event name recieved
                switch (map_event_name[topic_string])
                {
                    case BRIGHTNESS_UPDATE:
                        ESP_LOGI(TAG, "IN BRIGHTUPDATE CASE");

                        if(root2 != NULL){
                            brightness_value = cJSON_GetObjectItem(root2, "Brightness_value"); 
                            printf("brightness value : %f\n", brightness_value->valuedouble);
                            if(brightness_value->valuedouble > 1){
                                brightness_value->valuedouble = 1;
                            }
                            if(brightness_value->valuedouble < 0){
                                brightness_value->valuedouble = 0;
                            }
                            socket_brightness = brightness_value->valuedouble * 1023;
                            ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, socket_brightness);
                            ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
                        }
                        else{
                            printf("ROOT2 IS NULL\n");
                        }

                        break;
                    case ALARM_ADDED:
                        ESP_LOGI(TAG, "IN ALARM_ADDED CASE");
                        break;
                    case UPDATE_ALARM:
                        ESP_LOGI(TAG, "IN UPDATE_ALARMCASE");
                        break;
                    case TESTING_MODE:
                        ESP_LOGI(TAG, "IN TESTING_MODE CASE");
                        break;
                    case VOLUME_UPDATE:
                        ESP_LOGI(TAG, "IN VOLUME_UPDATE CASE");
                        break;
                    case NEW_NAP_TIME:
                        ESP_LOGI(TAG, "IN NEW_NAP_TIME CASE");
                        break;
                    case UPDATE_NAP_TIME:
                        ESP_LOGI(TAG, "IN UPDATE_NAP_TIME CASE");
                        break;
                    case DISCONNECT_DEVICE:
                        ESP_LOGI(TAG, "IN DISCONNECT_DEVICE CASE");
                        break;
                    case UNKNOWN:
                        ESP_LOGI(TAG, "IN UNKNOWN CASE");
                        break;
                    default:
                        ESP_LOGW(TAG, "Topic Unrecognized\n");
                        break;
                }
                break;
            }
            else if(ollie_id == NULL){ 
                ESP_LOGW(TAG, "No Event Occured");
                break;
            }
            else if(strcmp(ollie_id->valuestring, "651ab72aa928e0a9e87be8dd") != 0){
                ESP_LOGW(TAG, "Wrong Ollie ID");
                break;
            }
            break;
    }
}