#ifndef GODS_H
#define GODS_H

#include <string>
#include <vector>

struct God
{
public:
  God(const std::string& name, const std::string& descr) :
    name_(name), descr_(descr) {}

  inline std::string getName()   const {return name_;}
  inline std::string getDescr()  const {return descr_;}

private:
  std::string name_;
  std::string descr_;
};

namespace Gods
{

void init();

bool isGodLvl();

const God* getCurGod();

void setRandomGod();

void setNoGod();

} //Gods

#endif
