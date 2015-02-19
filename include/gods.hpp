#ifndef GODS_H
#define GODS_H

#include <string>
#include <vector>

struct God
{
public:
    God(const std::string& name, const std::string& descr) :
        name_(name), descr_(descr) {}

    std::string get_name()   const {return name_;}
    std::string get_descr()  const {return descr_;}

private:
    std::string name_;
    std::string descr_;
};

namespace Gods
{

void init();

bool is_god_lvl();

const God* get_cur_god();

void set_random_god();

void set_no_god();

} //Gods

#endif
