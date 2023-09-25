#ifndef rtc_H
#define rtc_H


#include <stdio.h>
#include "soc/rtc.h"
#include "esp_log.h"
#include <sys/time.h>
#include <time.h>
#include "esp_log.h"

enum WEEKDAY {SUN, MON, TUE, WED, THU, FRI, SAT};



class RTC{
        public: 
            /* functions */
            struct tm getTimeStruct();
            void getTime(char * s);
            void getDateTime(char* s, bool mode);
            void setTime(int sc, int mn, int hr, int dy, int mt, int yr, int ms);
            void setEpochTime(uint64_t epoch);
            time_t initialize();
            time_t updateTime();
            void syncRtc();
            bool checkDay(tm alarmStartTime);
            bool alarmHour(tm alarmStartTime);
            int8_t minDifference(tm alramStartTime);
            int8_t secDifference(tm alramStartTime);


            /* variables */
            time_t real_time; 
            struct tm realtimeinfo;
            struct tm *realtimeinfo_ptr = &realtimeinfo;
            char realtime_buf[64];
            struct tm start_timeinfo;
            time_t *time_ptr = &real_time;
};


#endif