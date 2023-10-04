#include "plug.hpp"

// Map to associate the strings with the enum values
static std::map<std::string, event_name> map_event_name;
float socket_brightness = 40;
static const char* TAG = "Plug";

esp_err_t plug::set_handler(){
    esp_err_t ret = ESP_OK;
    ret = esp_event_handler_instance_register(SIO_EVENT, ESP_EVENT_ANY_ID, &eventHandlerSIO, NULL, NULL);
    return ret;
}

esp_err_t plug::configure(){
    sio_client_config_t sioConfig;
    sioConfig.eio_version = 4;
    sioConfig.transport = SIO_TRANSPORT_POLLING;
    sioConfig.server_address = "dev.noozle.nl:3000";
    sioConfig.sio_url_path = "/socket.io";
    sioConfig.nspc = "/";
    sioConfig.alloc_auth_body_cb = NULL;

    sio_client_id_t sioID = sio_client_init(&sioConfig);
    esp_err_t ret = sio_client_begin(sioID);  //internally runs xTaskCreate for polling
    ESP_LOGI(TAG, "%d", ret);
    return ret;
}


/* SOCKET IO EVENT HANDLER*/

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
            cJSON *brightness_value = NULL, *user_id = NULL;
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
                            printf("brightness value : %d\n", brightness_value->valueint);
                            if(brightness_value->valueint > 100){
                                brightness_value->valueint = 100;
                            }
                            if(brightness_value->valueint < 0){
                                brightness_value->valueint = 0;
                            }
                            socket_brightness = (float)brightness_value->valueint/100 * 1023;
                            printf("brightness value : %d\n", (int)socket_brightness);
                            ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, socket_brightness);
                            ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
                        }
                        else{
                            printf("ROOT2 IS NULL\n");
                        }

                        break;
                    case ALARM_ADDED:
                        ESP_LOGI(TAG, "IN ALARM_ADDED CASE");

                        if(root2 != NULL){
                            user_id = cJSON_GetObjectItem(root2, "User_id"); 
                            printf("User ID : %s\n", user_id->valuestring);

                        }
                        else{
                            printf("ROOT2 IS NULL\n");
                        }

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