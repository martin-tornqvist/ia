#include "Utils.h"

#include <algorithm>
#include <assert.h>

#include "Engine.h"

#include "Converters.h"
#include "GameTime.h"
#include "MersenneTwister.h"

namespace Rnd {

//---------------------------------------------------------------- LOCAL
namespace {

MTRand mtRand;

int roll(const int ROLLS, const int SIDES) {
  if(SIDES <= 0) {return 0;}
  if(SIDES == 1) {return ROLLS * SIDES;}

  int result = 0;
  for(int i = 0; i < ROLLS; i++) {
    result += mtRand.randInt(SIDES - 1) + 1;
  }
  return result;
}

} //Namespace

//---------------------------------------------------------------- GLOBAL
void seed(const unsigned long val) {mtRand = MTRand(val);}

int dice(const int ROLLS, const int SIDES) {return roll(ROLLS, SIDES);}

int dice(const DiceParam& p) {return roll(p.rolls, p.sides);}

bool coinToss() {return roll(1, 2) == 2;}

bool fraction(const int NUMERATOR, const int DENOMINATOR) {
  return roll(1, DENOMINATOR) <= NUMERATOR;
}

bool oneIn(const int N) {return fraction(1, N);}

int range(const int MIN, const int MAX) {
  return MIN + roll(1, MAX - MIN + 1) - 1;
}

int range(const Range& valueRange) {
  return range(valueRange.lower, valueRange.upper);
}

int percentile() {return roll(1, 100);}

} //Rnd

namespace Utils {

//---------------------------------------------------------------- GLOBAL
bool isClrEq(const SDL_Color& clr1, const SDL_Color& clr2) {
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
                           vector<Pos>& vectorRef) {
  vectorRef.resize(0);
  for(int y = 0; y < MAP_H; y++) {
    for(int x = 0; x < MAP_W; x++) {
      if(a[x][y] == VALUE_TO_STORE) {
        vectorRef.push_back(Pos(x, y));
      }
    }
  }
}

void getActorPositions(const vector<Actor*>& actors, vector<Pos>& vectorRef) {
  vectorRef.resize(0);
  const unsigned int NR_ACTORS = actors.size();
  for(unsigned int i = 0; i < NR_ACTORS; i++) {
    vectorRef.push_back(actors.at(i)->pos);
  }
}


Actor* getActorAtPos(const Pos& pos, Engine& eng) {
  for(Actor * actor : eng.gameTime->actors_) {
    if(actor->pos == pos && actor->deadState == actorDeadState_alive) {
      return actor;
    }
  }
  return NULL;
}

void makeActorArray(Actor* a[MAP_W][MAP_H], Engine& eng) {
  resetArray(a);

  for(Actor * actor : eng.gameTime->actors_) {
    const Pos& p = actor->pos;
    a[p.x][p.y] = actor;
  }
}

bool isPosInsideMap(const Pos& pos) {
  return pos.x >= 0 && pos.y >= 0 && pos.x < MAP_W && pos.y < MAP_H;
}

bool isPosInside(const Pos& pos, const Rect& area) {
  return
    pos.x >= area.x0y0.x &&
    pos.x <= area.x1y1.x &&
    pos.y >= area.x0y0.y &&
    pos.y <= area.x1y1.y;
}

bool isAreaInsideOther(const Rect& inner, const Rect& outer,
                       const bool COUNT_EQUAL_AS_INSIDE) {
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

bool isAreaInsideMap(const Rect& area) {
  if(
    area.x0y0.x < 0 || area.x0y0.y < 0 ||
    area.x1y1.x >= MAP_W || area.x1y1.y >= MAP_H) {
    return false;
  }
  return true;
}

int chebyshevDist(const int X0, const int Y0, const int X1, const int Y1) {
  return max(std::abs(X1 - X0), std::abs(Y1 - Y0));
}

int chebyshevDist(const Pos& c1, const Pos& c2) {
  return max(std::abs(c2.x - c1.x), std::abs(c2.y - c1.y));
}

Pos getClosestPos(const Pos& c, const vector<Pos>& positions) {
  int distToNearest = INT_MAX;
  int closestElement = 0;
  const int NR_POSITIONS = positions.size();
  for(int i = 0; i < NR_POSITIONS; i++) {
    const int CUR_DIST = chebyshevDist(c, positions.at(i));
    if(CUR_DIST < distToNearest) {
      distToNearest = CUR_DIST;
      closestElement = i;
    }
  }

  return positions.at(closestElement);
}

Actor* getRandomClosestActor(const Pos& c, const vector<Actor*>& actors) {
  if(actors.empty()) return NULL;

  //Find distance to nearest actor(s)
  int distToNearest = INT_MAX;
  for(Actor * actor : actors) {
    const int CUR_DIST = chebyshevDist(c, actor->pos);
    if(CUR_DIST < distToNearest) {
      distToNearest = CUR_DIST;
    }
  }

  assert(distToNearest != INT_MAX);

  //Store all actors with distance equal to the nearest distance
  vector<Actor*> closestActors;
  for(Actor * actor : actors) {
    if(chebyshevDist(c, actor->pos) == distToNearest) {
      closestActors.push_back(actor);
    }
  }

  assert(closestActors.empty() == false);

  const int ELEMENT = Rnd::range(0, closestActors.size() - 1);

  return closestActors.at(ELEMENT);
}

bool isPosAdj(const Pos& pos1, const Pos& pos2,
              const bool COUNT_SAME_CELL_AS_NEIGHBOUR) {

  if(pos1.x == pos2.x && pos1.y == pos2.y) {
    return COUNT_SAME_CELL_AS_NEIGHBOUR;
  }
  if(pos1.x < pos2.x - 1) {
    return false;
  }
  if(pos1.x > pos2.x + 1) {
    return false;
  }
  if(pos1.y < pos2.y - 1) {
    return false;
  }
  if(pos1.y > pos2.y + 1) {
    return false;
  }
  return true;
}

TimeData getCurrentTime() {
  time_t t = time(0);
  struct tm* now = localtime(&t);
  return TimeData(now->tm_year + 1900, now->tm_mon + 1, now->tm_mday,
                  now->tm_hour, now->tm_min, now->tm_sec);
}

} //Utils

//------------------------------------------------------ TIME DATA
string TimeData::getTimeStr(const TimeType lowest,
                            const bool ADD_SEPARATORS) const {
  string ret = toString(year_);

  string monthStr   = (month_   < 10 ? "0" : "") + toString(month_);
  string dayStr     = (day_     < 10 ? "0" : "") + toString(day_);
  string hourStr    = (hour_    < 10 ? "0" : "") + toString(hour_);
  string minuteStr  = (minute_  < 10 ? "0" : "") + toString(minute_);
  string secondStr  = (second_  < 10 ? "0" : "") + toString(second_);

  if(lowest >= time_month)  ret += (ADD_SEPARATORS ? "-" : "-") + monthStr;
  if(lowest >= time_day)    ret += (ADD_SEPARATORS ? "-" : "-") + dayStr;
  if(lowest >= time_hour)   ret += (ADD_SEPARATORS ? " " : "_") + hourStr;
  if(lowest >= time_minute) ret += (ADD_SEPARATORS ? ":" : "-") + minuteStr;
  if(lowest >= time_second) ret += (ADD_SEPARATORS ? ":" : "-") + secondStr;

  return ret;
}

DirConverter::DirConverter() {
  compassDirNames[0][0] = "NW";
  compassDirNames[0][1] = "W";
  compassDirNames[0][2] = "SW";
  compassDirNames[1][0] = "N";
  compassDirNames[1][1] = "";
  compassDirNames[1][2] = "S";
  compassDirNames[2][0] = "NE";
  compassDirNames[2][1] = "E";
  compassDirNames[2][2] = "SE";
}

Dir DirConverter::getDir(const Pos& offset) const {
  assert(offset.x >= -1 && offset.y >= -1 && offset.x <= 1 && offset.y <= 1);

  if(offset.y == -1) {
    return offset.x == -1 ? dirUpLeft :
           offset.x == 0 ? dirUp :
           offset.x == 1 ? dirUpRight :
           endOfDirs;
  }

  if(offset.y == 0) {
    return offset.x == -1 ? dirLeft :
           offset.x == 0 ? dirCenter :
           offset.x == 1 ? dirRight :
           endOfDirs;
  }
  if(offset.y == 1) {
    return offset.x == -1 ? dirDownLeft :
           offset.x == 0 ? dirDown :
           offset.x == 1 ? dirDownRight :
           endOfDirs;
  }
  return endOfDirs;
}

Pos DirConverter::getOffset(const Dir dir) const {
  assert(dir != endOfDirs);

  switch(dir) {
    case dirDownLeft:   return Pos(-1, 1);
    case dirDown:       return Pos(0, 1);
    case dirDownRight:  return Pos(1, 1);
    case dirLeft:       return Pos(-1, 0);
    case dirCenter:     return Pos(0, 0);
    case dirRight:      return Pos(1, 0);
    case dirUpLeft:     return Pos(-1, -1);
    case dirUp:         return Pos(0, -1);
    case dirUpRight:    return Pos(1, -1);
    case endOfDirs:     return Pos(0, 0);
  }
  return Pos(0, 0);
}

void DirConverter::getCompassDirName(
  const Pos& fromPos, const Pos& toPos, string& strRef) const {

  strRef = "";

  const double PI_DB        = 3.14159265;
  const double ANGLE_45_DB  = 2 * PI_DB / 8;

  double edge[4];

  for(int i = 0; i < 4; i++) {
    edge[i] = (ANGLE_45_DB / 2) + (ANGLE_45_DB * i);
  }

  const Pos offset(toPos - fromPos);
  const double ANGLE_DB = atan2(-offset.y, offset.x);

  if(ANGLE_DB        <  -edge[2] && ANGLE_DB >  -edge[3]) {
    strRef = "SW";
  } else if(ANGLE_DB <= -edge[1] && ANGLE_DB >= -edge[2]) {
    strRef = "S";
  } else if(ANGLE_DB <  -edge[0] && ANGLE_DB >  -edge[1]) {
    strRef = "SE";
  } else if(ANGLE_DB >= -edge[0] && ANGLE_DB <=  edge[0]) {
    strRef = "E";
  } else if(ANGLE_DB >   edge[0] && ANGLE_DB <   edge[1]) {
    strRef = "NE";
  } else if(ANGLE_DB >=  edge[1] && ANGLE_DB <=  edge[2]) {
    strRef = "N";
  } else if(ANGLE_DB >   edge[2] && ANGLE_DB <   edge[3]) {
    strRef = "NW";
  } else {
    strRef = "W";
  }
}

void DirConverter::getCompassDirName(
  const Dir dir, string& strRef) const {

  const Pos& offset = getOffset(dir);
  strRef = compassDirNames[offset.x + 1][offset.y + 1];
}

void DirConverter::getCompassDirName(
  const Pos& offset, string& strRef) const {

  strRef = compassDirNames[offset.x + 1][offset.y + 1];
}

