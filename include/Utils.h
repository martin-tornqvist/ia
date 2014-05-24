#ifndef UTILS_H
#define UTILS_H

#include <vector>

#include "CmnData.h"
#include "Actor.h"
#include "MersenneTwister.h"

namespace Rnd {

//Note: If MTRand is not provided any parameters to the constructor, it will be
//seeded with current time. So seeding it manually is not necessary for normal
//gameplay purposes - only if seed should be controlled for testing purposes,
//or recreating a certain level, etc.
void seed(const unsigned long val);

int dice(const int ROLLS, const int SIDES);

int dice(const DiceParam& p);

bool coinToss();

bool fraction(const int NUMERATOR, const int DENOMINATOR);

bool oneIn(const int N);

int range(const int MIN, const int MAX);

int range(const Range& valueRange);

int percentile();

} //Rnd

enum TimeType {
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

  std::string getTimeStr(const TimeType lowest,
                         const bool ADD_SEPARATORS) const;

  int year_, month_, day_, hour_, minute_, second_;
};

namespace Utils {

bool isClrEq(const SDL_Color& clr1, const SDL_Color& clr2);

void resetArray(int a[MAP_W][MAP_H]);
void resetArray(Actor* a[MAP_W][MAP_H]);
void resetArray(bool a[MAP_W][MAP_H], const bool value);

void reverseBoolArray(bool array[MAP_W][MAP_H]);

void copyBoolArray(const bool in[MAP_W][MAP_H], bool out[MAP_W][MAP_H]);

void mkVectorFromBoolMap(const bool VALUE_TO_STORE,
                         bool a[MAP_W][MAP_H],
                         std::vector<Pos>& vectorRef);

Actor* getActorAtPos(const Pos& pos,
                     ActorDeadState deadState = ActorDeadState::alive);

void getActorPositions(const std::vector<Actor*>& actors,
                       std::vector<Pos>& vectorRef);

void mkActorArray(Actor* a[MAP_W][MAP_H]);

bool isPosInsideMap(const Pos& pos);

bool isPosInside(const Pos& pos, const Rect& area);

bool isAreaInsideOther(const Rect& inner, const Rect& outer,
                       const bool COUNT_EQUAL_AS_INSIDE);

bool isAreaInsideMap(const Rect& area);

bool isPosAdj(const Pos& pos1, const Pos& pos2,
              const bool COUNT_SAME_CELL_AS_NEIGHBOUR);

Pos getClosestPos(const Pos& c, const std::vector<Pos>& positions);

Actor* getRandomClosestActor(const Pos& c, const std::vector<Actor*>& actors);

int kingDist(const int X0, const int Y0, const int X1, const int Y1);
int kingDist(const Pos& c1, const Pos& c2);

TimeData getCurTime();

} //Utils

namespace DirUtils {

//Useful lists to iterate over (e.g. in map generation)
const Pos cardinalOffsets[] = {Pos(1, 0), Pos(-1, 0), Pos(0, 1), Pos(0, -1)};
const Pos diagonalOffsets[] = {Pos(1, 1), Pos(-1, 1), Pos(1, -1), Pos(-1, -1)};

Dir getDir(const Pos& offset);

Pos getOffset(const Dir dir);

void getCompassDirName(const Pos& fromPos, const Pos& toPos,
                       std::string& strRef);

void getCompassDirName(const Dir dir, std::string& strRef);

void getCompassDirName(const Pos& offset, std::string& strRef);

} //DirUtils

#endif
