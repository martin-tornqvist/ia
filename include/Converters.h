#ifndef CONVERTERS_H
#define CONVERTERS_H

#include <string>

std::string toString(int in);

int toInt(std::string in);

void constrInRange(const int MIN, int& val, const int MAX);

int getValConstrInRange(const int MIN, const int VAL, const int MAX);

#endif
