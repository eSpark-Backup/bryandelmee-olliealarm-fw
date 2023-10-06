#include "rtc.hpp"
#include "esp_netif_sntp.h"
#include "esp_sntp.h"

static const char* TAG = "RTC";
const int CONNECTED_BIT = BIT0;
enum time_of_day_t : uint8_t { DAY =1, DAWN =2, DUSK =3, NIGHT =4};

bool overflow = false;

unsigned long offset = 0; 

time_t RTC::initialize()
{
    time(time_ptr);
    setenv("TZ", "PKT-5", 1);
    tzset();
    // time_t current_time = now;
    localtime_r(time_ptr, realtimeinfo_ptr);
    strftime(realtime_buf, sizeof(realtime_buf), "%c", realtimeinfo_ptr);
    ESP_LOGI(TAG, "The current date/time in Pakistan is: %s", realtime_buf);
    return *time_ptr;
}

void RTC::syncRtc(){   //syncs the time from internet
    //time synchronization
    esp_sntp_config_t time_config = ESP_NETIF_SNTP_DEFAULT_CONFIG("pool.ntp.org");
    esp_sntp_setoperatingmode(ESP_SNTP_OPMODE_LISTENONLY);
    esp_netif_sntp_init(&time_config);
    if (esp_netif_sntp_sync_wait(pdMS_TO_TICKS(10000)) != ESP_OK) {
        printf("Failed to update system time within 10s timeout");
    }
    ESP_LOGI(TAG, " TIME SYNC ");
    vTaskDelay(2000/portTICK_PERIOD_MS);

}

/* THIS FUNCTION NEEDS TO BE CALLED ON EVERY LOOP ITERATION*/
time_t* RTC::updateTime(){
    time(time_ptr);  //update epoch time
    localtime_r(time_ptr, realtimeinfo_ptr);  //convert current epoch to tm struct
    strftime(realtime_buf, sizeof(realtime_buf), "%c", realtimeinfo_ptr);  //updates the human time char array
    ESP_LOGI(TAG, "The current date/time in Pakistan is: %s", realtime_buf);
    return time_ptr;
}

void RTC::printHumanTime(time_t *time){
    struct tm timeinfo;
    char time_buf[64];
    localtime_r(time, &timeinfo);
    strftime(time_buf, sizeof(time_buf), "%c", &timeinfo);
    ESP_LOGI(TAG, "HUMAN TIME for ALARMS: %s", time_buf);
}

void RTC::getTime(char * s){
	struct tm timeinfo = getTimeStruct();
	strftime(s, sizeof(s), "%H:%M:%S", &timeinfo);
}

struct tm RTC::getTimeStruct(){
  struct tm timeinfo;
  time_t now;
  time(&now);
  localtime_r(&now, &timeinfo);
  time_t tt = mktime (&timeinfo);
    
  if (overflow){
	  tt += 63071999;
  }
  tt += offset;
  struct tm * tn = localtime(&tt);
  if (overflow){
	  tn->tm_year += 64;
  }
  return *tn;
}

void RTC::setTime(int sc, int mn, int hr, int dy, int mt, int yr, int ms) {
  // seconds, minute, hour, day, month, year $ microseconds(optional)
  // ie setTime(20, 34, 8, 1, 4, 2021) = 8:34:20 1/4/2021
  struct tm t = {0};        // Initalize to all 0's
  t.tm_year = yr - 1900;    // This is year-1900, so 121 = 2021
  t.tm_mon = mt - 1;
  t.tm_mday = dy;
  t.tm_hour = hr;
  t.tm_min = mn;
  t.tm_sec = sc;
  time_t timeSinceEpoch = mktime(&t);
  ESP_LOGI("RTC","%lld",timeSinceEpoch);
  setEpochTime(timeSinceEpoch);
}

void RTC::setEpochTime(uint64_t epoch) {
    struct timeval tv;
    epoch = epoch*1000;
    tv.tv_sec = epoch/1000;
    tv.tv_usec = (epoch % 1000) * 1000;  // Convert remaining milliseconds to microseconds
    settimeofday(&tv, NULL);
}


void RTC::getDateTime(char* s, bool mode)
{
	struct tm timeinfo = getTimeStruct();
	if (mode)
	{
		strftime(s, 50, "%A, %B %d %Y %H:%M:%S", &timeinfo);
	}
	else
	{
		strftime(s, 50, "%a, %b %d %Y %H:%M:%S", &timeinfo);
	}
}

bool RTC::checkDay(tm alarmStartTime){
    if(realtimeinfo.tm_wday == alarmStartTime.tm_wday){
        // ESP_LOGI(TAG, "Alarm Day");
        return true;
    }
    else{
        // ESP_LOGI(TAG, "No Alarm Day");
    }
    return false;
}

bool RTC::alarmHour(tm alarmStartTime){
    if(realtimeinfo.tm_hour == alarmStartTime.tm_hour){
        // ESP_LOGI(TAG, "Alarm Hour");
        return true;
    }
    else{
        // ESP_LOGI(TAG, "No Alarm Hour");
    }
    return false;
}

time_t RTC::epochDifference(time_t currentAlarmEpoch){
    time_t diff = currentAlarmEpoch - real_time;
    return diff;
}

int8_t RTC::minDifference(tm alramStartTime, time_t currentAlarmEpoch){
    if((currentAlarmEpoch - real_time) < 3600){   //3600 epoch  = 60 mins
        if((alramStartTime.tm_min - realtimeinfo.tm_min) < 60){
        return (alramStartTime.tm_min - realtimeinfo.tm_min);
        }
        else{
            return 80;  //epochs greater than 1 hr
        }
    }
    return 80;
    
}

int8_t RTC::secDifference(tm alramStartTime){
        if((alramStartTime.tm_sec - realtimeinfo.tm_sec) <= 60){
            return (alramStartTime.tm_sec - realtimeinfo.tm_sec);
        }
        return 80;  //epochs greater than 1 hr
}