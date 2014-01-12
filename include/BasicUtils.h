#ifndef BASIC_UTILS_H
#define BASIC_UTILS_H

#include <vector>
//#include <math.h>

#include "CommonData.h"
#include "Actor.h"
#include "MersenneTwister.h"

using namespace std;

class Engine;

enum Time_t {
  time_year,
  time_month,
  time_day,
  time_hour,
  time_minute,
  time_second
};

struct TimeData {
  TimeData() :
    year_(0), month_(0), day_(0), hour_(0), minute_(0), second_(0) {
  }

  TimeData(int year, int month, int day, int hour, int minute, int second) :
    year_(year), month_(month), day_(day), hour_(hour), minute_(minute),
    second_(second) {}

  string getTimeStr(const Time_t lowest, const bool ADD_SEPARATORS) const;

  int year_, month_, day_, hour_, minute_, second_;
};

class BasicUtils {
public:
  BasicUtils(Engine& engine) : eng(engine) {}

  inline bool isClrEq(const SDL_Color& clr1, const SDL_Color& clr2) {
    return clr1.r == clr2.r && clr1.g == clr2.g && clr1.b == clr2.b;
  }

  void resetArray(int a[MAP_W][MAP_H]) {
    for(int y = 0; y < MAP_H; y++) {
      for(int x = 0; x < MAP_W; x++) {
        a[x][y] = 0;
      }
    }
  }

  void resetArray(Actor* a[MAP_W][MAP_H]) {
    for(int y = 0; y < MAP_H; y++) {
      for(int x = 0; x < MAP_W; x++) {
        a[x][y] = NULL;
      }
    }
  }

  void resetArray(bool a[MAP_W][MAP_H], const bool value) {
    for(int y = 0; y < MAP_H; y++) {
      for(int x = 0; x < MAP_W; x++) {
        a[x][y] = value;
      }
    }
  }

  void reverseBoolArray(bool array[MAP_W][MAP_H]) {
    for(int y = 0; y < MAP_H; y++) {
      for(int x = 0; x < MAP_W; x++) {
        array[x][y] = !array[x][y];
      }
    }
  }

  void makeVectorFromBoolMap(const bool VALUE_TO_STORE,
                             bool a[MAP_W][MAP_H],
                             vector<Pos>& vectorRef);

  Actor* getActorAtPos(const Pos& pos) const;

  void getActorPositions(const vector<Actor*>& actors,
                         vector<Pos>& vectorRef);

  void makeActorArray(Actor* a[MAP_W][MAP_H]);

  inline bool isPosInsideMap(const Pos& pos) const {
    return pos.x >= 0 && pos.y >= 0 && pos.x < MAP_W && pos.y < MAP_H;
  }

  inline bool isPosInside(const Pos& pos, const Rect& area) const {
    return
      pos.x >= area.x0y0.x &&
      pos.x <= area.x1y1.x &&
      pos.y >= area.x0y0.y &&
      pos.y <= area.x1y1.y;
  }

  inline bool isAreaInsideOther(const Rect& inner, const Rect& outer,
                                const bool COUNT_EQUAL_AS_INSIDE) const {
    if(COUNT_EQUAL_AS_INSIDE) {
      return
        inner.x0y0.x >= outer.x0y0.x &&
        inner.x1y1.x <= outer.x1y1.x &&
        inner.x0y0.y >= outer.x0y0.y &&
        inner.x1y1.y <= outer.x1y1.y;
    } else {
      return
        inner.x0y0.x > outer.x0y0.x &&
        inner.x1y1.x < outer.x1y1.x &&
        inner.x0y0.y > outer.x0y0.y &&
        inner.x1y1.y < outer.x1y1.y;
    }
  }

  inline bool isAreaInsideMap(const Rect& area) {
    if(
      area.x0y0.x < 0 || area.x0y0.y < 0 ||
      area.x1y1.x >= MAP_W || area.x1y1.y >= MAP_H) {
      return false;
    }
    return true;
  }


  bool isPosAdj(const Pos& pos1, const Pos& pos2,
                const bool COUNT_SAME_CELL_AS_NEIGHBOUR) const;

  Pos getClosestPos(const Pos& c, const vector<Pos>& positions) const;

  Actor* getClosestActor(const Pos& c, const vector<Actor*>& actors) const;

  inline int chebyshevDist(const int X0, const int Y0,
                           const int X1, const int Y1) const {
    if(X0 == X1 && Y0 == Y1) {
      return 0;
    }
    return max(abs(X1 - X0), abs(Y1 - Y0));
  }

  inline int chebyshevDist(const Pos& c1, const Pos& c2) const {
    return chebyshevDist(c1.x, c1.y, c2.x, c2.y);
  }

  TimeData getCurrentTime() const;

private:
  Engine& eng;
};

class Dice {
public:
  int operator()(const int ROLLS, const int SIDES) {
    return roll(ROLLS, SIDES);
  }

  int operator()(const DiceParam& p) {
    return roll(p.rolls, p.sides);
  }

  //Note: If MTRand is not provided any parameters to the constructor,
  //it will instead seed with current time. So seeding it manually is
  //not necessary for normal gameplay purposes - only if seed should be
  //controlled for testing purposes, or recreating a certain level, etc.
  void seed(const unsigned long val) {
    mtRand_ = MTRand(val);
  }

  bool coinToss() {
    return roll(1, 2) == 2;
  }

  inline bool fraction(const int NUMERATOR, const int DENOMINATOR) {
    return roll(1, DENOMINATOR) <= NUMERATOR;
  }

  inline bool oneIn(const int N) {
    return fraction(1, N);
  }

  inline int range(const int MIN, const int MAX) {
    return MIN + roll(1, MAX - MIN + 1) - 1;
  }

  inline int range(const Range& valueRange) {
    return range(valueRange.lower, valueRange.upper);
  }

  inline int percentile() {return roll(1, 100);}

private:
  int roll(const int ROLLS, const int SIDES) {
    if(SIDES <= 0) {return 0;}
    if(SIDES == 1) {return ROLLS * SIDES;}

    int result = 0;
    for(int i = 0; i < ROLLS; i++) {
      result += mtRand_.randInt(SIDES - 1) + 1;
    }
    return result;
  }

  MTRand mtRand_;
};

class DirConverter {
public:
  DirConverter();

  ~DirConverter() {}

  Dir_t getDir(const Pos& offset) const;

  Pos getOffset(const Dir_t dir) const;

  void getCompassDirName(
    const Pos& fromPos, const Pos& toPos, string& strRef) const;

  void getCompassDirName(const Dir_t dir, string& strRef) const;

  void getCompassDirName(const Pos& offset, string& strRef) const;

private:
  string compassDirNames[3][3];
};

#endif
