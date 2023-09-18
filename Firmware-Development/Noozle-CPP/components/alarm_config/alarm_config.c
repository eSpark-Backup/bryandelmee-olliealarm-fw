#include "alarm_config.h"

time_t now;
char strftime_buf[64];
struct tm timeinfo;

static const char *ALARM_TAG = "TEST_ALARM_CONFIG";


time_t init_time(){
    time(&now);
    setenv("TZ", "PKT-5", 1);
    tzset();
    // time_t current_time = now;
    localtime_r(&now, &timeinfo);
    strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
    ESP_LOGI(ALARM_TAG, "The current date/time in Pakistan is: %s", strftime_buf);
    return now;
}

/**
 * @brief To initialize a particular SNTP server and also start the SNTP service, simply create a default SNTP server configuration with a particular server name, 
 * then call esp_netif_sntp_init() to register that server and start the SNTP service.
 */
void sync_time(){   //syncs the time from internet
    //time synchronization
    esp_sntp_config_t time_config = ESP_NETIF_SNTP_DEFAULT_CONFIG("pool.ntp.org");
    esp_sntp_setoperatingmode(ESP_SNTP_OPMODE_LISTENONLY);
    esp_netif_sntp_init(&time_config);
    if (esp_netif_sntp_sync_wait(pdMS_TO_TICKS(10000)) != ESP_OK) {
        printf("Failed to update system time within 10s timeout");
    }
    ESP_LOGI(ALARM_TAG, " TIME SYNC ");
    vTaskDelay(2000/portTICK_PERIOD_MS);

}

/**
 * @brief Set Alarm start time
 * 
 * @param alarm : Pass the alarm of which you want to set the Start Time
 * @param time : Time you want to set
 * .
 */
void set_alarm_start_time(alarm_config* alarm, time_t time){
    alarm->start_time = time;
    localtime_r(&(alarm->start_time), &(alarm->start_timeinfo));
    strftime(alarm->start_strftime_buf, sizeof(alarm->start_strftime_buf), "%c", &(alarm->start_timeinfo));
    ESP_LOGI(ALARM_TAG, "Setting Alarm Start Time to : %llu", alarm->start_time);
    ESP_LOGI(ALARM_TAG, "Alarm Start Human time is: %s", alarm->start_strftime_buf);

}

void set_alarm_end_time(alarm_config* alarm, time_t time){
    alarm->end_time = time;
    localtime_r(&(alarm->end_time), &(alarm->end_timeinfo));
    strftime(alarm->end_strftime_buf, sizeof(alarm->end_strftime_buf), "%c", &(alarm->end_timeinfo));
    ESP_LOGI(ALARM_TAG, "Setting Alarm End Time to : %llu", alarm->end_time);
    ESP_LOGI(ALARM_TAG, "Alarm End Human time is: %s", alarm->end_strftime_buf);
}

bool alarm_ring_time(alarm_config* alarm){
    time_t current_time = init_time();
    if(current_time >= alarm->start_time && current_time <= alarm->end_time){
        return true;
    }
    else{
        return false;
    }
}

void ring_alarm(alarm_config* alarm){
    while(init_time() < alarm->end_time){
        ESP_LOGI(ALARM_TAG, "PLaying Alarm");
        // ESP_LOGE(ALARM_TAG, "No playwav function");
        alarm->audio_fptr(alarm->file_name);
        // playwav
        vTaskDelay(1000/portTICK_PERIOD_MS);
    }
}

void show_alarm_info(alarm_config* alarm){
    time(&now);
    localtime_r(&now, &timeinfo);
    strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
    ESP_LOGI(ALARM_TAG, "The current date/time in Pakistan is: %s", strftime_buf);
    ESP_LOGI(ALARM_TAG, " The current date/time in Pakistan in epochs: %llu", now);
    
    localtime_r(&(alarm->start_time), &(alarm->start_timeinfo));
    strftime(alarm->start_strftime_buf, sizeof(alarm->start_strftime_buf), "%c", &(alarm->start_timeinfo));
    ESP_LOGI(ALARM_TAG, "Alarm start time is: %s", alarm->start_strftime_buf);
    ESP_LOGI(ALARM_TAG, "The alarm start time in epochs: %llu", alarm->start_time);

    localtime_r(&(alarm->end_time), &timeinfo);
    strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
    ESP_LOGI(ALARM_TAG, "Alarm end time is: %s", strftime_buf);
    ESP_LOGI(ALARM_TAG, "The alarm end time in epochs: %llu", alarm->end_time);
}

time_t return_current_time(){
    return now;
}