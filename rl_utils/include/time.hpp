#ifndef RL_UTILS_TIME_HPP
#define RL_UTILS_TIME_HPP

enum class TimeType
{
    year,
    month,
    day,
    hour,
    minute,
    second
};

struct TimeData
{
    TimeData() :
        year_   (0),
        month_  (0),
        day_    (0),
        hour_   (0),
        minute_ (0),
        second_ (0) {}

    TimeData(int year,
             int month,
             int day,
             int hour,
             int minute,
             int second) :
        year_   (year),
        month_  (month),
        day_    (day),
        hour_   (hour),
        minute_ (minute),
        second_ (second) {}

    std::string time_str(const TimeType lowest,
                         const bool add_separators) const;

    int year_, month_, day_, hour_, minute_, second_;
};

TimeData current_time();

#endif // RL_UTILS_TIME_HPP
