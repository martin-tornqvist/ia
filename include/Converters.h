#ifndef CONVERTERS_H
#define CONVERTERS_H

#include <string>

std::string toStr(const int IN);

int toInt(const std::string& in);

//Intended for enum class values, to retrieve the underlying type (e.g. int)
template <typename T>
typename std::underlying_type<T>::type toUnderlying(T t);

void constrInRange(const int MIN, int& val, const int MAX);
void constrInRange(const double MIN, double& val, const double MAX);

int getConstrInRange(const int MIN, const int VAL, const int MAX);
int getConstrInRange(const double MIN, const double VAL, const double MAX);

#endif
