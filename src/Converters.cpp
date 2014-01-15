#include "Converters.h"

#include <string>
#include <sstream>
#include <iostream>

std::string toString(int in) {
  std::ostringstream buffer;
  buffer << in;
  return buffer.str();
}

int toInt(std::string in) {
  int _nr;
  std::istringstream buffer(in);
  buffer >> _nr;
  return _nr;
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

