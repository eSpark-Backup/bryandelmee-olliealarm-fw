#ifndef alarm_config_H
#define alarm_config_H

#include <time.h>
#include "esp_netif_sntp.h"
#include "esp_log.h"
#include "freertos/task.h"
#include "esp_sntp.h"

// #include "esp_sntp.h"
#define TWO_MIN_EPOCH 120
#define ONE_MIN_EPOCH 60
#define HALF_MIN_EPOCH 30
#define FIFTEEN_SEC_EPOCH 15

#ifdef __cplusplus
extern "C" {
#endif

typedef esp_err_t (*func)(char file_location[] ) ; 

// /* time */
// extern time_t now;
// extern char strftime_buf[64];
// extern struct tm timeinfo;

/* alarm config structure*/
typedef struct{
    time_t start_time;
    char start_strftime_buf[64];
    struct tm start_timeinfo;
    time_t end_time;
    char end_strftime_buf[64];
    struct tm end_timeinfo;
    func audio_fptr;
    char file_name[24];
    
}alarm_config;

time_t init_time();
void sync_time();
void set_alarm_start_time(alarm_config* alarm, time_t time);
void set_alarm_end_time(alarm_config* alarm, time_t time);
bool alarm_ring_time(alarm_config* alarm);
void ring_alarm(alarm_config* alarm);
void show_alarm_info(alarm_config* alarm);
time_t return_current_time();

#ifdef __cplusplus
}
#endif

#endif  //alarm_config_H