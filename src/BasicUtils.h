#ifndef BASIC_UTILS_H
#define BASIC_UTILS_H

#include <vector>
#include <math.h>

#include "ConstTypes.h"
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
    year_(year), month_(month), day_(day), hour_(hour), minute_(minute), second_(second) {
  }

  string getTimeStr(const Time_t lowest, const bool ADD_SEPARATORS) const;

  int year_, month_, day_, hour_, minute_, second_;
};

class BasicUtils {
public:
  BasicUtils(Engine* engine) {
    eng = engine;
  }

  void lexicographicalSortStringAndColorVector(vector<StringAndColor>& vect);

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

  int chebyshevDistance(const int X0, const int Y0, const int X1, const int Y1) const {
    if(X0 == X1 && Y0 == Y1) {
      return 0;
    }
    return max(abs(X1 - X0), abs(Y1 - Y0));
  }

  int chebyshevDistance(const coord& c1, const coord& c2) const {
    return chebyshevDistance(c1.x, c1.y, c2.x, c2.y);
  }

  int manhattanDistance(const int X0, const int Y0, const int X1, const int Y1) const {
    return abs(X1 - X0) + abs(Y1 - Y0);
  }

  double pointDistance(const int x1, const int y1, const int x2, const int y2) const {
    if(x1 == x2 && y1 == y2)
      return 0.0;

    const double xSqr = static_cast<double>((x2 - x1) * (x2 - x1));
    const double ySqr = static_cast<double>((y2 - y1) * (y2 - y1));
    return sqrt(xSqr + ySqr);
  }

  TimeData getCurrentTime() const;

private:
  Engine* eng;
};

class Dice {
public:
  int operator()(const int ROLLS, const int SIDES) {return roll(ROLLS, SIDES);}
  int operator()(const DiceParam& p) {return roll(p.rolls, p.sides);}
  bool coinToss() {return roll(1, 2) == 2;}

  int getInRange(const int MIN, const int MAX) {
    const int ROLL = roll(1, MAX - MIN + 1);
    return MIN + ROLL - 1;
  }

  int percentile() {
    return roll(1, 100);
  }

private:
  int roll(const int ROLLS, const int SIDES) {
    if(SIDES <= 0) {
      return 0;
    }
    if(SIDES == 1) {
      return ROLLS * SIDES;
    }

    int result = 0;
    for(int i = 0; i < ROLLS; i++) {
      result += m_MTRand.randInt(SIDES - 1) + 1;
    }
    return result;
  }

  MTRand m_MTRand;
};

#endif
