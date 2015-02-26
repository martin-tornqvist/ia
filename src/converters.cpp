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

//The following functions will first raise val to at least MIN,
//then lower val to at most MAX
void constr_in_range(const int MIN, int& val, const int MAX)
{
    if (MAX >= MIN)
    {
        val = std::min(MAX, std::max(val, MIN));
    }
}

void constr_in_range(const double MIN, double& val, const double MAX)
{
    if (MAX > MIN)
    {
        val = std::min(MAX, std::max(val, MIN));
    }
}

int get_constr_in_range(const int MIN, const int VAL, const int MAX)
{
    if (MAX < MIN)
    {
        return -1;
    }

    return std::min(MAX, std::max(VAL, MIN));
}

int get_constr_in_range(const double MIN, const double VAL, const double MAX)
{
    if (MAX < MIN)
    {
        return -1;
    }

    return std::min(MAX, std::max(VAL, MIN));
}

