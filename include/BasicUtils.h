#ifndef BASIC_UTILS_H
#define BASIC_UTILS_H

#include <vector>
#include <math.h>

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
  BasicUtils(Engine* engine) {
    eng = engine;
  }

  inline bool isClrEq(const SDL_Color& clr1, const SDL_Color& clr2) {
    return clr1.r == clr2.r && clr1.g == clr2.g && clr1.b == clr2.b;
  }

  void lexicographicalSortStringAndClrVector(vector<StringAndClr>& vect);

  void resetArray(int array[MAP_X_CELLS][MAP_Y_CELLS]) {
    for(int y = 0; y < MAP_Y_CELLS; y++) {
      for(int x = 0; x < MAP_X_CELLS; x++) {
        array[x][y] = 0;
      }
    }
  }

  void resetBoolArray(bool array[MAP_X_CELLS][MAP_Y_CELLS], const bool value) {
    for(int y = 0; y < MAP_Y_CELLS; y++) {
      for(int x = 0; x < MAP_X_CELLS; x++) {
        array[x][y] = value;
      }
    }
  }

  void reverseBoolArray(bool array[MAP_X_CELLS][MAP_Y_CELLS]) {
    for(int y = 0; y < MAP_Y_CELLS; y++) {
      for(int x = 0; x < MAP_X_CELLS; x++) {
        array[x][y] = !array[x][y];
      }
    }
  }

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

//  inline int manhattanDist(const int X0, const int Y0,
//                               const int X1, const int Y1) const {
//    return abs(X1 - X0) + abs(Y1 - Y0);
//  }

//  inline double pointDist(const int x1, const int y1,
//                              const int x2, const int y2) const {
//    if(x1 == x2 && y1 == y2)
//      return 0.0;
//
//    const double xSqr = double((x2 - x1) * (x2 - x1));
//    const double ySqr = double((y2 - y1) * (y2 - y1));
//    return sqrt(xSqr + ySqr);
//  }

  TimeData getCurrentTime() const;

private:
  Engine* eng;
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

class DirectionConverter {
public:
  DirectionConverter(Engine* const engine) : eng(engine) {
    compassDirectonNames[0][0] = "NW";
    compassDirectonNames[0][1] = "W";
    compassDirectonNames[0][2] = "SW";
    compassDirectonNames[1][0] = "N";
    compassDirectonNames[1][1] = "";
    compassDirectonNames[1][2] = "S";
    compassDirectonNames[2][0] = "NE";
    compassDirectonNames[2][1] = "E";
    compassDirectonNames[2][2] = "SE";
  }

  ~DirectionConverter() {}

  Direction_t getDirection(const Pos& offset) const {
    if(offset.y == -1) {
      return offset.x == -1 ? directionUpLeft :
             offset.x == 0 ? directionUp :
             offset.x == 1 ? directionUpRight :
             endOfDirections;
    }

    if(offset.y == 0) {
      return offset.x == -1 ? directionLeft :
             offset.x == 0 ? directionCenter :
             offset.x == 1 ? directionRight :
             endOfDirections;
    }
    if(offset.y == 1) {
      return offset.x == -1 ? directionDownLeft :
             offset.x == 0 ? directionDown :
             offset.x == 1 ? directionDownRight :
             endOfDirections;
    }
    return endOfDirections;
  }

  Pos getOffset(const Direction_t direction) const {
    switch(direction) {
      case directionDownLeft:   return Pos(-1, 1);
      case directionDown:       return Pos(0, 1);
      case directionDownRight:  return Pos(1, 1);
      case directionLeft:       return Pos(-1, 0);
      case directionCenter:     return Pos(0, 0);
      case directionRight:      return Pos(1, 0);
      case directionUpLeft:     return Pos(-1, -1);
      case directionUp:         return Pos(0, -1);
      case directionUpRight:    return Pos(1, -1);
      case endOfDirections:     return Pos(999, 999);
    }
    return Pos(999, 999);
  }

  string getCompassDirectionName(const Direction_t direction) const {
    const Pos& offset = getOffset(direction);
    return compassDirectonNames[offset.x + 1][offset.y + 1];
  }

  string getCompassDirectionName(const Pos& offset) const {
    return compassDirectonNames[offset.x + 1][offset.y + 1];
  }

private:
  DirectionConverter() : eng(NULL) {}

  Engine* const eng;

  string compassDirectonNames[3][3];
};

#endif
