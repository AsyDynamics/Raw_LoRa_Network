// NTP real time
#include "time.h"
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 3600;
const int   daylightOffset_sec = 3600;

void setup(){
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
}

void realTime(byte &sync_H, byte &sync_M, byte &sync_S){
  struct tm timeinfo;
  if (getLocalTime(&timeinfo)) {
    sync_H = timeinfo.tm_hour;
    sync_M = timeinfo.tm_min;
    sync_S = timeinfo.tm_sec;
  }
}
