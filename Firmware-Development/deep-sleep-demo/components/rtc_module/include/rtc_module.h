#ifndef RTC_MODULE_H
#define RTC_MODULE_H

#include <stdio.h>
#include "soc/rtc.h"
#include "esp_log.h"
#include <sys/time.h>
#include <time.h>
#include "esp_log.h"

struct tm getTimeStruct();
void getTime(char * s);
void getDateTime(char* s, bool mode);
void setTime(int sc, int mn, int hr, int dy, int mt, int yr, int ms);
void setEpochTime(uint64_t epoch, int ms);

#endif