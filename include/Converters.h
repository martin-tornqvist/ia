#ifndef CONVERTERS_H
#define CONVERTERS_H

#include <string>

std::string toString(int in);

int toInt(std::string in);

void constrInRange(const int MIN, int& val, const int MAX);
void constrInRange(const double MIN, double& val, const double MAX);

int getConstrInRange(const int MIN, const int VAL, const int MAX);
int getConstrInRange(const double MIN, const double VAL, const double MAX);

#endif
