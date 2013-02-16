#include "BasicUtils.h"

#include <stdlib.h>
#include <iostream>
#include <ctime>
#include <algorithm>

#include "Engine.h"

#include "Converters.h"

// Function for lexicographically comparing two StringAndColor structs
struct LexicograhicalCompareStringAndColor {
public:
  LexicograhicalCompareStringAndColor() {
  }
  bool operator()(const StringAndColor& entry1, const StringAndColor& entry2) {
    return std::lexicographical_compare(entry1.str.begin(), entry1.str.end(), entry2.str.begin(), entry2.str.end());
  }
};

void BasicUtils::lexicographicalSortStringAndColorVector(vector<StringAndColor>& vect) {
  LexicograhicalCompareStringAndColor cmp;
  std::sort(vect.begin(), vect.end(), cmp);
}

string TimeData::getTimeStr(const Time_t lowest, const bool ADD_SEPARATORS) const {
  string ret = intToString(year_);

  string monthStr   = (month_   < 10 ? "0" : "") + intToString(month_);
  string dayStr     = (day_     < 10 ? "0" : "") + intToString(day_);
  string hourStr    = (hour_    < 10 ? "0" : "") + intToString(hour_);
  string minuteStr  = (minute_  < 10 ? "0" : "") + intToString(minute_);
  string secondStr  = (second_  < 10 ? "0" : "") + intToString(second_);

  if(lowest >= time_month)  ret += (ADD_SEPARATORS ? "-" : "") + monthStr;
  if(lowest >= time_day)    ret += (ADD_SEPARATORS ? "-" : "") + dayStr;
  if(lowest >= time_hour)   ret += (ADD_SEPARATORS ? " " : "") + hourStr;
  if(lowest >= time_minute) ret += (ADD_SEPARATORS ? ":" : "") + minuteStr;
  if(lowest >= time_second) ret += (ADD_SEPARATORS ? ":" : "") + secondStr;

  return ret;
}

TimeData BasicUtils::getCurrentTime() const {
  time_t t = time(0);
  struct tm* now = localtime(&t);
  return TimeData(now->tm_year + 1900, now->tm_mon + 1, now->tm_mday,
                  now->tm_hour, now->tm_min, now->tm_sec);
}
