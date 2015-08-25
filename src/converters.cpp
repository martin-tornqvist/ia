#include "converters.hpp"

#include "init.hpp"

#include <string>
#include <sstream>

std::string to_str(const int IN)
{
    std::ostringstream buffer;
    buffer << IN;
    return buffer.str();
}

int to_int(const std::string& in)
{
    int _nr;
    std::istringstream buffer(in);
    buffer >> _nr;
    return _nr;
}

template <typename T>
typename std::underlying_type<T>::type to_underlying(T t)
{
    return static_cast<typename std::underlying_type<T>::type>(t);
}
