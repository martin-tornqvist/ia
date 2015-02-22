#ifndef CONVERTERS_H
#define CONVERTERS_H

#include <string>

std::string to_str(const int IN);

int to_int(const std::string& in);

//Intended for enum class Values, to retrieve the underlying type (e.g. int)
template <typename T>
typename std::underlying_type<T>::type to_underlying(T t);

void constr_in_range(const int MIN, int& val, const int MAX);
void constr_in_range(const double MIN, double& val, const double MAX);

int get_constr_in_range(const int MIN, const int VAL, const int MAX);
int get_constr_in_range(const double MIN, const double VAL, const double MAX);

#endif
