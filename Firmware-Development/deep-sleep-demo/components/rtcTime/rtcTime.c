#include "rtcTime.h"

unsigned long offset = 0; /**/
bool overflow = false;

void getTime(char * s){
	struct tm timeinfo = getTimeStruct();
	strftime(s, 50, "%H:%M:%S", &timeinfo);
}

struct tm getTimeStruct(){
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

void setTime(int sc, int mn, int hr, int dy, int mt, int yr, int ms) {
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
  setEpochTime(timeSinceEpoch, ms);
}

void setEpochTime(unsigned long epoch, int ms) {
  struct timeval tv;
  if (epoch > 2082758399){
	  overflow = true;
	  tv.tv_sec = epoch - 2082758399;  // epoch time (seconds)
  } else {
	  tv.tv_sec = epoch;  // epoch time (seconds)
  }
  tv.tv_usec = ms;    // microseconds
  settimeofday(&tv, NULL);
}

void getDateTime(char* s, bool mode)
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