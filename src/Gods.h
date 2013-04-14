#ifndef GODS_H
#define GODS_H

#include <string>
#include <vector>

using namespace std;

class Engine;

struct God {
public:
  God(const string& name, const string& descr) : name_(name), descr_(descr) {}

  inline string getName() const {return name_;}
  inline string getDescr() const {return descr_;}

private:
  string name_;
  string descr_;
};

class Gods {
public:
  Gods(Engine* const engine) : eng(engine) {initGodList();}
  ~Gods() {}

  inline bool isGodLevel() const {return currentGodElem_ >= 0;}

  inline const God* getCurrentGod() const {
    if(currentGodElem_ >= 0) {
      return &(godList.at(static_cast<unsigned int>(currentGodElem_)));
    }
    return NULL;
  }

  void setRandomGod();

  void setNoGod();

private:
  vector<God> godList;

  int currentGodElem_;

  void initGodList();

  Engine* const eng;
};

#endif
