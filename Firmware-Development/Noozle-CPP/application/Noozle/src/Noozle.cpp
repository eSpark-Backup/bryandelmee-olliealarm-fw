#include "Noozle.hpp"
#include "esp_check.h"

#define SPACE_LABEL "Device_Info"
#define BOOT_KEY "Boot"
#define PROV_STATUS_KEY "Prov_Status"
#define SLEEP_WAKEUP_OFFSET 15


static const char* TAG = "Noozle";
extern char* sleep_scene_string[];
RTC_DATA_ATTR bool alarmState = false;
extern app_status_t noozleStatus;

Noozle::Noozle()
{
    nvs = new NVS();
    wifi = new WIFI();
    ble = new BLEProv();
    rtc = new RTC();
    // currentAlarm = new alarm_config;
    wakeAlarm = new alarm_config;
    sleepAlarm = new alarm_config;
    nearestAlarm = new alarm_config;
}

esp_err_t Noozle::setup()
{
    ESP_RETURN_ON_ERROR(setupNVS(), TAG, "setup : Setting up NVS");
    /* These API must be called before using WIFI/BLE */
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    ESP_RETURN_ON_ERROR(wifi->setup(), TAG, "setup : Setting up Wi-Fi");
    ESP_RETURN_ON_ERROR(ble->setup(), TAG, "setup : Setting up BLE");
    rtc->initialize();
    spiff();
    return ESP_OK;
}

esp_err_t Noozle::setupNVS()
{
    ESP_RETURN_ON_ERROR(nvs->init(), TAG, "setupNVS: Initialize NVS");
    /*Setup Keys in the NVS with default values*/
    ESP_RETURN_ON_ERROR(nvs->setupKeyU32(SPACE_LABEL, BOOT_KEY), TAG, "setupNVS : Setting up U32 key in NVS");
    ESP_RETURN_ON_ERROR(nvs->setupKeyU8(SPACE_LABEL, PROV_STATUS_KEY), TAG, "setupNVS : Setting up U8 key in NVS");
    return ESP_OK;
}

esp_err_t Noozle::connectWifi()
{
    ESP_RETURN_ON_ERROR(wifi->run(), TAG, "connectWifi : Wi-Fi is started");
    ESP_RETURN_ON_ERROR(wifi->wait(), TAG, "connectWifi : Waiting For Wi-FI connect/fail");
    return ESP_OK;
}

esp_err_t Noozle::connectBLE()
{
    ESP_RETURN_ON_ERROR(ble->run(), TAG, "connectBLE: Ble is started");
    ble->wait();
    return ESP_OK;
}

esp_err_t Noozle::setProvStatus(uint8_t status)
{
    uint8_t prov_status = status;
    ESP_LOGI(TAG,"setProvStatus = %u", prov_status);
    ESP_RETURN_ON_ERROR(nvs->writeKeyU8(SPACE_LABEL, PROV_STATUS_KEY, prov_status), TAG, "setProvStatus: Writes provisioning status in NVS");
    return ESP_OK;
}

uint8_t Noozle::getProvStatus()
{
    uint8_t prov_status;
    ESP_RETURN_ON_ERROR(nvs->readKeyU8(SPACE_LABEL, PROV_STATUS_KEY, &prov_status), TAG, "getProvStatus : Reads provisioning status from NVS");
    ESP_LOGI(TAG,"getProvStatus = %u", prov_status);
    return prov_status;
}

esp_err_t Noozle::bootCountUpdate()
{
    uint32_t boot_count = 0;
    ESP_RETURN_ON_ERROR(nvs->readKeyU32(SPACE_LABEL, BOOT_KEY, &boot_count), TAG, "bootCountUpdate: Read boot-count from NVS");
    ESP_RETURN_ON_ERROR(nvs->writeKeyU32(SPACE_LABEL, BOOT_KEY, ++boot_count), TAG, "bootCountUpdate: Writes boot-count in NVS");
    ESP_LOGI(TAG, "boot count = %lu", boot_count);
    return ESP_OK;
}

void Noozle::checkProvStatusAndRunBleOrWifi()
{
    uint8_t ret = getProvStatus();
    if(ret == PROVISIONING_STATUS_FAIL)
    {
        connectBLE();
        setProvStatus(PROVISIONING_STATUS_OK); /*0 : Device Provisioned */
    }
    else
    {
        esp_err_t ret = connectWifi();
        if(ret != ESP_OK)
            setProvStatus(PROVISIONING_STATUS_FAIL);  /*1 : Device is not Provisioned */
    }
}

void Noozle::setAlarmsFromNvs(){
    uint32_t time;
    nvs->readKeyU32(sleep_scene_string[ALMOST_SLEEP], "time", &time);
    set_alarm_start_time(sleepAlarm, (time_t)time);
    ESP_LOGI(TAG, "Almost Asleep time NVS : %ld\n", time);

    nvs->readKeyU32(sleep_scene_string[TO_SLEEP], "time", &time);
    set_alarm_end_time(sleepAlarm, (time_t)time);
    ESP_LOGI(TAG, "To SLeep time NVS : %ld\n", time);

    nvs->readKeyU32(sleep_scene_string[ALMOST_GETTING_UP], "time", &time);
    set_alarm_start_time(wakeAlarm, (time_t)time);
    ESP_LOGI(TAG, "Almost Getting up time NVS : %ld\n", time);

    nvs->readKeyU32(sleep_scene_string[TO_GET_UP], "time", &time);
    set_alarm_end_time(wakeAlarm, (time_t)time);
    ESP_LOGI(TAG, "To get up time NVS : %ld\n", time);

    nvs->readKeyU8("weekdays", "weekday_bits", &weekday_bits);
    ESP_LOGI(TAG, "Week days in binary: %d\n", weekday_bits);
    bits_to_bool(weekday_bits);
}

void Noozle::bits_to_bool(uint8_t bits){
    for(int i=0; i<7; i++){
        if((bits & (1 << (6-i))) != 0){
            weekday[i] = true;
        }
        else{
            weekday[i] = false;
        }
    }

    for(int j = 0; j<7; j++){
        printf("WEEKDAY %d = %d\n", j, weekday[j]);
    }

}


void Noozle::updateTime(){
    rtc->updateTime();
}

void Noozle::setNearestAlarm(){
    /* CHECK DAY*/
    uint32_t wakeDiff = 0, sleepDiff = 0;
    int thisDay = rtc->realtimeinfo.tm_wday;
    int nextDay = (thisDay < 7) ? thisDay+1 : 0;
    if(weekday[thisDay] == 1){
        ESP_LOGI(TAG, "TODAY IS ALARM");

        sleepAlarm->start_timeinfo.tm_year = rtc->realtimeinfo.tm_year;
        sleepAlarm->start_timeinfo.tm_mon = rtc->realtimeinfo.tm_mon;
        sleepAlarm->start_timeinfo.tm_mday = rtc->realtimeinfo.tm_mday;

        sleepAlarm->end_timeinfo.tm_year = rtc->realtimeinfo.tm_year;
        sleepAlarm->end_timeinfo.tm_mon = rtc->realtimeinfo.tm_mon;
        sleepAlarm->end_timeinfo.tm_mday = rtc->realtimeinfo.tm_mday;

        wakeAlarm->start_timeinfo.tm_year = rtc->realtimeinfo.tm_year;
        wakeAlarm->start_timeinfo.tm_mon = rtc->realtimeinfo.tm_mon;
        wakeAlarm->start_timeinfo.tm_mday = rtc->realtimeinfo.tm_mday;

        wakeAlarm->end_timeinfo.tm_year = rtc->realtimeinfo.tm_year;
        wakeAlarm->end_timeinfo.tm_mon = rtc->realtimeinfo.tm_mon;
        wakeAlarm->end_timeinfo.tm_mday = rtc->realtimeinfo.tm_mday;

        set_alarm_start_time(sleepAlarm, mktime(&sleepAlarm->start_timeinfo));
        set_alarm_end_time(sleepAlarm, mktime(&sleepAlarm->end_timeinfo));

        set_alarm_start_time(wakeAlarm, mktime(&wakeAlarm->start_timeinfo));
        set_alarm_end_time(wakeAlarm, mktime(&wakeAlarm->end_timeinfo));
        
        wakeDiff = wakeAlarm->start_time - rtc->real_time;
        sleepDiff = sleepAlarm->start_time - rtc->real_time;
    }
    else if(weekday[nextDay] == 1){ //have check for saturday 7+1 = 8 and 8 is not a day
        ESP_LOGI(TAG, "TOMORROW IS ALARM");
        sleepAlarm->start_timeinfo.tm_year = rtc->realtimeinfo.tm_year;
        sleepAlarm->start_timeinfo.tm_mon = rtc->realtimeinfo.tm_mon;
        sleepAlarm->start_timeinfo.tm_mday = rtc->realtimeinfo.tm_mday;

        sleepAlarm->end_timeinfo.tm_year = rtc->realtimeinfo.tm_year;
        sleepAlarm->end_timeinfo.tm_mon = rtc->realtimeinfo.tm_mon;
        sleepAlarm->end_timeinfo.tm_mday = rtc->realtimeinfo.tm_mday;

        wakeAlarm->start_timeinfo.tm_year = rtc->realtimeinfo.tm_year;
        wakeAlarm->start_timeinfo.tm_mon = rtc->realtimeinfo.tm_mon;
        wakeAlarm->start_timeinfo.tm_mday = rtc->realtimeinfo.tm_mday;

        wakeAlarm->end_timeinfo.tm_year = rtc->realtimeinfo.tm_year;
        wakeAlarm->end_timeinfo.tm_mon = rtc->realtimeinfo.tm_mon;
        wakeAlarm->end_timeinfo.tm_mday = rtc->realtimeinfo.tm_mday;

        set_alarm_start_time(sleepAlarm, mktime(&sleepAlarm->start_timeinfo)+86400);
        set_alarm_end_time(sleepAlarm, mktime(&sleepAlarm->end_timeinfo)+86400);

        set_alarm_start_time(wakeAlarm, mktime(&wakeAlarm->start_timeinfo)+86400);
        set_alarm_end_time(wakeAlarm, mktime(&wakeAlarm->end_timeinfo)+86400);
        
        wakeDiff = wakeAlarm->start_time - rtc->real_time;
        sleepDiff = sleepAlarm->start_time - rtc->real_time;
    }
    

    if(wakeDiff < sleepDiff){
        nearestAlarm = wakeAlarm;
        ESP_LOGI(TAG, "NEAREST ALARM = WAKE ALARM");
    }
    else if( sleepDiff < wakeDiff){
        nearestAlarm = sleepAlarm;
        ESP_LOGI(TAG, "NEAREST ALARM = SLEEP ALARM");
    }
    else if(wakeDiff == sleepDiff){
        nearestAlarm = NULL;
        ESP_LOGI(TAG, "NEAREST ALARM = NULL ALARM");
    }
    ESP_LOGI(TAG, "DIFFS wake : %ld sleep : %ld", wakeDiff, sleepDiff);
}

void Noozle::setDeepSleep(){
    uint64_t calculatedSleepUs;
    if(nearestAlarm != NULL){
        // int8_t minuteDifference = rtc->minDifference(nearestAlarm->start_timeinfo, nearestAlarm->start_time);
        time_t epochDiff = rtc->epochDifference(nearestAlarm->start_time);
        // int8_t secondDifference = rtc->secDifference(nearestAlarm->start_timeinfo);
        ESP_LOGI(TAG, " EPOCH DIFFERNECE B/W Alarm and Current Time= %lld", epochDiff);
                if( epochDiff >= 3600)
                {
                    ESP_LOGI(TAG, "ESP CAN GO 1HR DEEP SLEEP");
                    calculatedSleepUs = (unsigned int)(60*60*1000*1000);
                    alarmState = false;
                    noozleStatus = DEEP_SLEEP;
                    deep_sleep(calculatedSleepUs);

                }
                else if(epochDiff < 3600 && epochDiff > 0)
                {
                    ESP_LOGI(TAG, "ESP CAN GO DEEP SLEEP FOR %lld Mins ideal ", epochDiff/60 );
                    // calculatedSleepUs = ((minuteDifference) + (secondDifference - SLEEP_WAKEUP_OFFSET)) * 1000000;
                    calculatedSleepUs = (epochDiff - SLEEP_WAKEUP_OFFSET)*1000000;
                    alarmState = true;
                    noozleStatus = RING_ALARM;
                    deep_sleep(calculatedSleepUs);
                }
                else if(epochDiff < 0){
                    ESP_LOGI(TAG, "Alarm Passed waiting for new alarm");
                    alarmState = false;
                }
    }
    else{  // go to deep sleep if no nearest alarm
        ESP_LOGI(TAG, "No Nearest Alarm");
        calculatedSleepUs = (unsigned int)(60*60*1000*1000);
        deep_sleep(calculatedSleepUs);
        alarmState = false;
    }

}


void deep_sleep(uint64_t sleepTimeUs){
    uint64_t timer = sleepTimeUs;
    ESP_LOGI(TAG, "Activating Deep Sleep for %llu = us", timer);
    esp_sleep_enable_timer_wakeup(timer);
    /* disabled ext0
    gpio_set_direction(GPIO_NUM_20, GPIO_MODE_INPUT);
    rtc_gpio_hold_en(GPIO_NUM_20);
    esp_sleep_enable_ext0_wakeup(GPIO_NUM_20, 0);
    */
    esp_deep_sleep_start();
}

bool Noozle::alarmStatus(){
    if(alarmState == true){
        return true;
    }
    return false;

}

void Noozle::checkAlarm(){
    if(alarmState == true){
        ring_alarm(nearestAlarm);
    }
    else if(alarmState == false){
        ESP_LOGI(TAG, "ITS NOT ALARM TIME");
    }
}

void Noozle::updateBatteryStatus(){
    ESP_LOGI(TAG, "UPDATE BATTERY STATUS");
}

void Noozle::spiff(){
    /*SPIFFS SETUP*/
    esp_vfs_spiffs_conf_t config = {
      .base_path = "/spiffs",
      .partition_label = NULL,
      .max_files = 5,
      .format_if_mount_failed = true,
    };
    esp_vfs_spiffs_register(&config);

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
}