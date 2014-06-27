#include "Converters.h"

#include <string>
#include <sstream>
#include <iostream>

std::string toStr(const int IN) {
  std::ostringstream buffer;
  buffer << IN;
  return buffer.str();
}

int toInt(const std::string& in) {
  int _nr;
  std::istringstream buffer(in);
  buffer >> _nr;
  return _nr;
}

template <typename T>
typename std::underlying_type<T>::type toUnderlying(T t) {
  return static_cast<typename std::underlying_type<T>::type>(t);
}

//The following functions will first raise val to at least MIN,
//then lower val to at most MAX
void constrInRange(const int MIN, int& val, const int MAX) {
  if(MAX >= MIN) {
    val = std::min(MAX, std::max(val, MIN));
  }
}

void constrInRange(const double MIN, double& val, const double MAX) {
  if(MAX > MIN) {
    val = std::min(MAX, std::max(val, MIN));
  }
}

int getConstrInRange(const int MIN, const int VAL, const int MAX) {
  if(MAX < MIN) {
    return -1;
  }
  return std::min(MAX, std::max(VAL, MIN));
}

int getConstrInRange(const double MIN, const double VAL, const double MAX) {
  if(MAX < MIN) {
    return -1;
  }
  return std::min(MAX, std::max(VAL, MIN));
}

