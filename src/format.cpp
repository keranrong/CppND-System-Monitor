#include <string>

#include "format.h"


using std::string;

// Reframe the time to double digit number string
// INPUT: int unit time in sec, min or hours
// OUTPUT: double digit number string
string Format::TimeFormat(int time) {
  if (time < 10) return "0" + std::to_string(time);
  return std::to_string(time);
}

// INPUT: Long int measuring seconds
// OUTPUT: HH:MM:SS
string Format::ElapsedTime(long seconds) {
  int sec = seconds % 60;
  seconds -= sec;
  int min = seconds % 3600 / 60;
  seconds -= min * 60;
  int hr = seconds / 3600;
  return string(Format::TimeFormat(hr) + ':' + Format::TimeFormat(min) + ':' +
                Format::TimeFormat(sec));
}