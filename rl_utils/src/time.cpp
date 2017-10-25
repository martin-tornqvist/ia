#include "rl_utils.hpp"

#include <time.h>

std::string TimeData::time_str(const TimeType lowest,
                               const bool add_separators) const
{
    std::string ret = std::to_string(year_);

    const std::string month_str =
        (month_ < 10 ? "0" : "") + std::to_string(month_);

    const std::string day_str =
        (day_ < 10 ? "0" : "") + std::to_string(day_);

    const std::string hour_str =
        (hour_ < 10 ? "0" : "") + std::to_string(hour_);

    const std::string minute_str =
        (minute_ < 10 ? "0" : "") + std::to_string(minute_);

    const std::string second_str =
        (second_ < 10 ? "0" : "") + std::to_string(second_);

    if (lowest >= TimeType::month)
    {
        ret += "-" + month_str;
    }

    if (lowest >= TimeType::day)
    {
        ret += "-" + day_str;
    }

    if (lowest >= TimeType::hour)
    {
        ret += (add_separators ? " " : "_") + hour_str;
    }

    if (lowest >= TimeType::minute)
    {
        ret += (add_separators ? ":" : "-") + minute_str;
    }

    if (lowest >= TimeType::second)
    {
        ret += (add_separators ? ":" : "-") + second_str;
    }

    return ret;
}

TimeData current_time()
{
    time_t t        = time(nullptr);
    struct tm* now  = localtime(&t);

    return TimeData(now->tm_year + 1900,
                     now->tm_mon + 1,
                     now->tm_mday,
                     now->tm_hour,
                     now->tm_min,
                     now->tm_sec);
}
