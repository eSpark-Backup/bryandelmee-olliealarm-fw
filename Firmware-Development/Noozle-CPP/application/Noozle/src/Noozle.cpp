#include "Noozle.hpp"
#include "esp_check.h"

#define SPACE_LABEL "Device_Info"
#define BOOT_KEY "Boot"
#define PROV_STATUS_KEY "Prov_Status"
#define SLEEP_WAKEUP_OFFSET 15
// #define DEBUG

/* FUNCTION PROTOTYPES*/
void display_task(void *pv);
void audio_task(void *pv);
void configureAlarm(alarm_config* alarm);

/* TASK HANDLES*/
TaskHandle_t displayhandle = NULL;
TaskHandle_t audiohandle = NULL;

static const char* TAG = "Noozle";
extern char* sleep_scene_string[];
RTC_DATA_ATTR bool alarmState = false;
extern app_status_t noozleStatus;
extern app_status_t lastStatus;

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

esp_err_t Noozle::setAlarmsFromNvs(){
    esp_err_t err = ESP_OK;
    //Check how many scenes and naps are saved in NVS
    int n, total_naps;

    err = readNoOfNaps(&total_naps);
    if(err != ESP_OK){
        numberOfNaps = 0;
        // nearestNapAlarm = 0; //SKIP MALLOC
    }
    else{
        numberOfNaps = total_naps;
        //allocate memory to  nap alarms
        napAlarm = (alarm_config*)malloc(numberOfNaps*sizeof(alarm_config));
        nearestNapAlarm = (alarm_config*)malloc(sizeof(alarm_config));
    }

    err = readNoOfScenes(&n);
    if(err != ESP_OK){
        // v_nearestAlarm = 0; /*SKIP MALLOC*/
        numberOfScenes = 0;
    }
    else{
        numberOfScenes = n;
        //Allocate memory to Alarm structures
        sleepAlarm = (alarm_config*)malloc(n*sizeof(alarm_config));
        wakeAlarm = (alarm_config*)malloc(n*sizeof(alarm_config));
        nearestAlarm = (alarm_config*)malloc(n*sizeof(alarm_config));
        v_nearestAlarm = (alarm_config*)malloc(sizeof(alarm_config));
        p_weekday = (bool*)calloc(n, sizeof(weekday));
        // return err;
    }

    //if both are not in nvs
    if((nearestAlarm == NULL) & (v_nearestAlarm == NULL)){
        return err;
    }

    //loop for no. of scenes and fetch scenenames for accessing namespces in NVS
    if(v_nearestAlarm != NULL){
        char *scene_name = "repeatedd";
        for(int i = 0; i < numberOfScenes; i++){
            readSceneName(scene_name, i);
            ESP_LOGI(TAG, "REPEATED : %s", scene_name);

            //Read weekdays of the particular scene first
            nvs->readKeyU8(scene_name, "weekday_bits", &weekday_bits);
            ESP_LOGI(TAG, "Week days in binary: %d\n", weekday_bits);
            bits_to_bool(weekday_bits, i);

            //Setting Sleep START and END of current scene
            uint32_t time;
            nvs->readKeyU32(scene_name, sleep_scene_string[ALMOST_SLEEP], &time);
            set_alarm_start_time(&(sleepAlarm[i]), (time_t)time);
            ESP_LOGI(TAG, "Almost Asleep time NVS : %ld\n", time);

            nvs->readKeyU32(scene_name, sleep_scene_string[TO_SLEEP], &time);
            set_alarm_end_time(&(sleepAlarm[i]), (time_t)time);
            ESP_LOGI(TAG, "To SLeep time NVS : %ld\n", time);

            //Setting Wake START and END of current scene
            nvs->readKeyU32(scene_name, sleep_scene_string[ALMOST_GETTING_UP], &time);
            set_alarm_start_time(&(wakeAlarm[i]), (time_t)time);
            ESP_LOGI(TAG, "Almost Getting time NVS : %ld\n", time);

            nvs->readKeyU32(scene_name, sleep_scene_string[TO_GET_UP], &time);
            set_alarm_end_time(&(wakeAlarm[i]), (time_t)time);
            ESP_LOGI(TAG, "To Getup time NVS : %ld\n", time);

            //show alarm info
            ESP_LOGI(TAG,"\nSLEEP ALARM SCENE %d\n", i);
            show_alarm_info(sleepAlarm+i);
            ESP_LOGI(TAG,"\nWAKE ALARM SCENE %d\n", i);
            show_alarm_info(wakeAlarm+i);
        }
    }

    if(nearestNapAlarm != NULL){
        char* current_nap_name = "Somenap";
        for(int k = 0; k < numberOfNaps; k++){
            readNapName(current_nap_name, k);
            ESP_LOGI(TAG, "NAP %d : %s", k, current_nap_name);

            //Setting NAP alarm time
            uint32_t time;
            nvs->readKeyU32(current_nap_name, "start_time", &time);
            set_alarm_start_time(&(napAlarm[k]), (time_t)time);

            nvs->readKeyU32(current_nap_name, "end_time", &time);
            set_alarm_end_time(&(napAlarm[k]), (time_t)time);

            ESP_LOGI(TAG, "Nap %d ALarm time Set to : %ld\n", k, time);
            //show alarm info
            show_alarm_info(napAlarm+k);
            
        }
    }
    return err;
}
esp_err_t Noozle::readNoOfScenes(int* n){
    esp_err_t ret = ESP_OK;
    ESP_LOGI(TAG, " Reading No Scenes from NVS"); 
    uint32_t number;
    ret |= nvs->readKeyU32("scene_names", "no_scenes", &number);
    *n = (int)number;
    ESP_LOGI(TAG, "read no of scenes : %d", *n);
    return ret;
}

esp_err_t Noozle::readNoOfNaps(int* n){
    esp_err_t ret = ESP_OK;
    ESP_LOGI(TAG, " Reading No Naps from NVS"); 
    uint8_t number;
    ret = nvs->readKeyU8("nap_info", "total_nap", &number);
    *n = (int)number;
    ESP_LOGI(TAG, "read no of naps : %d", *n);
    return ret;
}

void Noozle::readSceneName(char* p_scene_name, uint8_t scene_number){
    nvs_flash_init();
    nvs_handle_t handle;
    size_t required_size = 0;

    nvs_open("scene_names", NVS_READWRITE, &handle);
    char array[65] = "scene";
    snprintf(array, 18,"scene %d",scene_number);

    nvs_get_str(handle, array,NULL, &required_size);
    // char* scene_name = (char*)malloc(required_size);
    nvs_get_str(handle, array, p_scene_name,&required_size);
}

void Noozle::readNapName(char* p_nap_name, uint8_t nap_number){
    snprintf(p_nap_name, 18,"nap_%d", (int)nap_number);

}

void Noozle::bits_to_bool(uint8_t bits, int scene_no){
    for(int i=0; i<7; i++){ //i means the day of the week
        if((bits & (1 << (6-i))) != 0){
            *(p_weekday+(i+(scene_no*7))) = true;
        }
        else{
            *(p_weekday+(i+(scene_no*7))) = false;
        }
    }

    for(int j = 0; j<7; j++){
        printf("WEEKDAY %d = %d\n", j, *(p_weekday+(j+(scene_no*7))));
    }

}


void Noozle::updateTime(){
    rtc->updateTime();
}

esp_err_t Noozle::setNearestAlarm(){
    /* CHECK DAY*/
    uint32_t wakeDiff[numberOfScenes] = {0}, sleepDiff[numberOfScenes] = {0}; //set all differences to 0
    int thisDay = rtc->realtimeinfo.tm_wday;
    int nextDay = (thisDay < 7) ? thisDay+1 : 0;
    
    if(numberOfScenes > 0)
    {
        //get epoch difference from current real time. individual scene
        v_nearestAlarm->start_time = 4820714998; // set to max year = 2123
        for(int i = 0; i<numberOfScenes; i++)
            {
                //manually set the month and year and month day to current day so epoch difference is with in a day
                configureAlarm(sleepAlarm+i, false);
                configureAlarm(wakeAlarm+i, false);

                //now set difference in epoch of each alarm
                sleepDiff[i] = (sleepAlarm+i)->start_time - rtc->real_time;
                ESP_LOGI(TAG, "Sleep Diff %lu", sleepDiff[i]);
                wakeDiff[i] = (wakeAlarm+i)->start_time - rtc->real_time;
                ESP_LOGI(TAG, "Wake Diff %lu", wakeDiff[i]);

                //(check current day) && (check if both alarms are not passed)
                if(*(p_weekday+(thisDay+(i * 7))) == 1 && !((wakeDiff[i] & 1 << 31)  & (sleepDiff[i] & 1 << 31))){
                    ESP_LOGI(TAG, "ALARM FOUND at DAY :%d FOR Scene : %d",thisDay, i);
                    if(wakeDiff[i] < sleepDiff[i]){
                        nearestAlarm[i] = wakeAlarm[i];
                        ESP_LOGI(TAG, "NEAREST ALARM = WAKE ALARM");
                    }
                    else if( sleepDiff[i] < wakeDiff[i]){
                        nearestAlarm[i] = sleepAlarm[i];
                        ESP_LOGI(TAG, "NEAREST ALARM = SLEEP ALARM");
                    }
                }
                else if(*(p_weekday+(nextDay+(i * 7))) == 1){
                    ESP_LOGI(TAG, "ALARM FOUND at DAY :%d FOR Scene : %d",nextDay, i);

                    //reconfigure for next day
                    configureAlarm(sleepAlarm+i, true);
                    configureAlarm(wakeAlarm+i, true);
                    if(wakeDiff[i] < sleepDiff[i]){
                        nearestAlarm[i] = wakeAlarm[i];
                        ESP_LOGI(TAG, "NEAREST ALARM = WAKE ALARM");
                    }
                    else if( sleepDiff[i] < wakeDiff[i]){
                        nearestAlarm[i] = sleepAlarm[i];
                        ESP_LOGI(TAG, "NEAREST ALARM = SLEEP ALARM");
                    }
                }
                else{
                    //nearest alarm of the scene will be garbage
                    ESP_LOGI(TAG, "NO ALARM TODAY OR TOMORROW FOR SCENE %d", i);
                    
                    //change the garbage to 1970 or 0 epoch
                    set_alarm_start_time(nearestAlarm+i, 0);
                    set_alarm_end_time(nearestAlarm+i, 0);
                }

            //select the nearest of all alarms
            if((nearestAlarm[i].start_time > 0)  & (nearestAlarm[i].start_time <= v_nearestAlarm->start_time)){
                v_nearestAlarm = &(nearestAlarm[i]);
            }
        }

    }
    if(numberOfNaps > 0){
        ESP_LOGE(TAG, "NAPS ARE AVAILABLE");
        //TODO: set nearest nap from all available naps
        lowest = (alarm_config*)malloc(sizeof(alarm_config)); 
        lowest->start_time = 4820714998; //set it to max to find the nearest
        for(int j = 0; j < numberOfNaps; j++){
            if((napAlarm[j].start_time > 0) & (napAlarm[j].start_time <= lowest->start_time)){
                lowest = &(napAlarm[j]);
            }
        }
        // ESP_LOGI(TAG, "NEAREST NAP ALARM EPOCH %lld: ", lowest->start_time);
        show_alarm_info(lowest);
    }
    else{
        ESP_LOGE(TAG, "NO NAP AND SCENES AVAILABLE");
        return ESP_FAIL;
    }

    //setting very nearest from scenes and naps
    if((lowest->start_time > rtc->real_time) && (v_nearestAlarm->start_time < lowest->start_time)){
        ESP_LOGI(TAG, "REAPEATED IS THE NEAREST");
    }
    else if((lowest->start_time < rtc->real_time)){
        ESP_LOGI(TAG, "NAP TIME PASSED REPEATED IS NEAREST");
        // free(lowest);
    }
    else{
        ESP_LOGI(TAG,"NAP TIME IS THE NEAREST");
        v_nearestAlarm = lowest;
    }
    
    ESP_LOGI(TAG, "Alarm To Ring : ");
    rtc->printHumanTime(&(v_nearestAlarm->start_time));
    return ESP_OK;

}

/**
 * @brief Configure alarm : change month year and month day to current epoch so comparasion of weekday is easier
 * 
 * @param alarm : Pass the alarm of which you want to set the Start Time
 * 
 */
void Noozle::configureAlarm(alarm_config* alarm, bool next_day){
    
    ESP_LOGI(TAG, "CONFIGURING ALARM");
    alarm->start_timeinfo.tm_year = rtc->realtimeinfo.tm_year;
    alarm->start_timeinfo.tm_mon = rtc->realtimeinfo.tm_mon;
    alarm->start_timeinfo.tm_mday = rtc->realtimeinfo.tm_mday;


    alarm->end_timeinfo.tm_year = rtc->realtimeinfo.tm_year;
    alarm->end_timeinfo.tm_mon = rtc->realtimeinfo.tm_mon;
    alarm->end_timeinfo.tm_mday = rtc->realtimeinfo.tm_mday;

    if(next_day){
        set_alarm_start_time(alarm, mktime(&alarm->start_timeinfo)+86400);
        set_alarm_end_time(alarm, mktime(&alarm->end_timeinfo)+86400);
    }
    else{
        set_alarm_end_time(alarm, mktime(&alarm->end_timeinfo));
        set_alarm_start_time(alarm, mktime(&alarm->start_timeinfo));
    }


}

//TODO:
// esp_err_t Noozle::fetchTime(){

// }

// esp_err_t Noozle::fetchId(){

// }
esp_err_t Noozle::fetchRepeated(){
    esp_err_t err = ESP_OK;
    err |= noozle_cloud_service(NOOZLE_REPEATED_ALARM_API); 
    if(err == ESP_ERR_NOT_FOUND){
        numberOfScenes = 0;
    }
    return err;
}
//TODO:
// esp_err_t Noozle::fetchNap(){

// }

void Noozle::setDeepSleep(){
    uint64_t calculatedSleepUs;
    if(v_nearestAlarm != NULL){
        // int8_t minuteDifference = rtc->minDifference(nearestAlarm->start_timeinfo, nearestAlarm->start_time);
        time_t epochDiff = rtc->epochDifference(v_nearestAlarm->start_time);
        // int8_t secondDifference = rtc->secDifference(nearestAlarm->start_timeinfo);
        ESP_LOGI(TAG, " EPOCH DIFFERNECE B/W Alarm and Current Time= %lld", epochDiff);
                if( epochDiff >= 3600)  //Alarm after 60 mins or 1 hr
                {
                    ESP_LOGI(TAG, "ESP CAN GO 1HR DEEP SLEEP");
                    calculatedSleepUs = (unsigned int)(60*60*1000*1000);
                    alarmState = false;
                    noozleStatus = DEEP_SLEEP; //FIXME: UPDATE_BATTERY
                    deep_sleep(calculatedSleepUs);

                }
                else if(epochDiff < 3600 && epochDiff > 0)  //Alarm under 60 mins
                {
                    ESP_LOGI(TAG, "ESP CAN GO DEEP SLEEP FOR %lld Mins ideal ", epochDiff/60 );
                    // calculatedSleepUs = ((minuteDifference) + (secondDifference - SLEEP_WAKEUP_OFFSET)) * 1000000;
                    calculatedSleepUs = (epochDiff - SLEEP_WAKEUP_OFFSET)*1000000;
                    alarmState = true;
                    noozleStatus = SET_ALARMS;
                    deep_sleep(calculatedSleepUs);
                }
                else if(epochDiff < 0){  //negative epoch means alarm passed
                    ESP_LOGI(TAG, "Alarm Passed waiting for new alarm");
                    v_nearestAlarm = NULL;
                    noozleStatus = FETCH_ALARM;
                    alarmState = false;
                }
    }
    else{  // go to deep sleep if no nearest alarm
        ESP_LOGI(TAG, "No Nearest Alarm");
        calculatedSleepUs = (unsigned int)(60*60*1000*1000);
        alarmState = false;
        noozleStatus = SET_ALARMS;
        deep_sleep(calculatedSleepUs);
    }

}


void deep_sleep(uint64_t sleepTimeUs){
    uint64_t timer = sleepTimeUs;
    ESP_LOGI(TAG, "Activating Deep Sleep for %llu = us", timer);
    esp_sleep_enable_timer_wakeup(timer);
    // // enabled ext0
    // gpio_set_direction(GPIO_NUM_20, GPIO_MODE_INPUT);
    // rtc_gpio_hold_en(GPIO_NUM_20);
    // esp_sleep_enable_ext0_wakeup(GPIO_NUM_20, 0);

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
        // ring_alarm(nearestAlarm);
        ESP_LOGI(TAG, "WAITING FOR ALARM ");
        while(rtc->real_time < v_nearestAlarm->start_time){
            vTaskDelay(500/portTICK_PERIOD_MS);
            ESP_LOGI(TAG, ".");
            rtc->updateTime();
        }
        // xTaskCreatePinnedToCore(display_task,"Display task", 2*2048, NULL, 10, &displayhandle, 0);
        xTaskCreate(audio_task,"audio task", 2*2048, NULL, 10, &audiohandle);
        // vTaskDelay(10000/portTICK_PERIOD_MS);

        // codec audio;
        // audio.setup();
        // audio.setVol(90);

        while(rtc->real_time < nearestAlarm->end_time){
            ESP_LOGI(TAG, "PLaying Alarm");
            // ESP_LOGE(ALARM_TAG, "No playwav function");
            // alarm->audio_fptr(alarm->file_name); //causing guru meditation
            // playwav  
            // audio.play("/spiffs/audio/Piano.wav");
            vTaskDelay(10/portTICK_PERIOD_MS);
            rtc->updateTime();
        }
        // vTaskDelete(displayhandle);
        ESP_LOGI(TAG, "ALARM ENDED TASK DELETING");
        vTaskDelete(audiohandle);
        // audio.setVol(0);
        alarmState = false;
    }
    else if(alarmState == false){
        ESP_LOGI(TAG, "ITS NOT ALARM TIME");
    }

}

void Noozle::updateBatteryStatus(){
    ESP_LOGI(TAG, "UPDATE BATTERY STATUS");
    noozle_cloud_service(NOOZLE_BATTERY_UPDATE_API);
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

/*DISPLAY TASK FUNCTION*/
void display_task(void *pv){
    DISPLAY display;
    display.setup();
    display.update(DAY);
    display.timer(30);
    while(1){
        display.operate();
        vTaskDelay(1/portTICK_PERIOD_MS);
    }
}

void audio_task(void *pv){
    codec audio;
    audio.setup();
    while(1){
        audio.setVol(90);
        audio.play("/spiffs/audio/Piano.wav");   
        audio.setVol(0);
        vTaskDelay(500/portTICK_PERIOD_MS);
    }
}