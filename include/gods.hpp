#ifndef GODS_H
#define GODS_H

#include <string>
#include <vector>

struct God
{
public:
    God(const std::string& god_name, const std::string& god_descr) :
        name_(god_name), descr_(god_descr) {}

    std::string name()   const
    {
        return name_;
    }
    std::string descr()  const
    {
        return descr_;
    }

private:
    std::string name_;
    std::string descr_;
};

namespace gods
{

void init();

bool is_god_lvl();

const God* cur_god();

void set_random_god();

void set_no_god();

} //Gods

#endif
