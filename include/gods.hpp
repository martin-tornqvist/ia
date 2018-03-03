#ifndef GODS_HPP
#define GODS_HPP

#include <string>
#include <vector>

struct God
{
        std::string name_;
        std::string descr_;
};

namespace gods
{

const God& current_god();

void set_random_god();

} // gods

#endif // GODS_HPP
