#include "Utils.h"

#include <algorithm>
#include <vector>
#include <assert.h>

#include "Converters.h"
#include "GameTime.h"
#include "MersenneTwister.h"
#include "Actor.h"
#include "FeatureMob.h"

using namespace std;

namespace Rnd {

namespace {

MTRand mtRand;

int roll(const int ROLLS, const int SIDES) {
  if(SIDES <= 0) {return 0;}
  if(SIDES == 1) {return ROLLS * SIDES;}

  int result = 0;
  for(int i = 0; i < ROLLS; ++i) {
    result += mtRand.randInt(SIDES - 1) + 1;
  }
  return result;
}

} //Namespace

void seed(const unsigned long val) {mtRand = MTRand(val);}

int dice(const int ROLLS, const int SIDES) {return roll(ROLLS, SIDES);}

int dice(const DiceParam& p) {return roll(p.rolls, p.sides);}

bool coinToss() {return roll(1, 2) == 2;}

bool fraction(const int NUMERATOR, const int DENOMINATOR) {
  if(NUMERATOR <= 0) {return false;}
  return roll(1, DENOMINATOR) <= NUMERATOR;
}

bool fraction(const Fraction& fractionVals) {
  return fraction(fractionVals.numerator, fractionVals.denominator);
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

bool isClrEq(const Clr& clr1, const Clr& clr2) {
  return clr1.r == clr2.r && clr1.g == clr2.g && clr1.b == clr2.b;
}

void resetArray(int a[MAP_W][MAP_H]) {
  for(int x = 0; x < MAP_W; ++x) {
    for(int y = 0; y < MAP_H; ++y) {
      a[x][y] = 0;
    }
  }
}

void resetArray(bool a[MAP_W][MAP_H], const bool value) {
  for(int x = 0; x < MAP_W; ++x) {
    for(int y = 0; y < MAP_H; ++y) {
      a[x][y] = value;
    }
  }
}

void reverseBoolArray(bool array[MAP_W][MAP_H]) {
  for(int x = 0; x < MAP_W; ++x) {
    for(int y = 0; y < MAP_H; ++y) {
      array[x][y] = !array[x][y];
    }
  }
}

void copyBoolArray(const bool in[MAP_W][MAP_H], bool out[MAP_W][MAP_H]) {
  for(int x = 0; x < MAP_W; ++x) {
    for(int y = 0; y < MAP_H; ++y) {
      out[x][y] = in[x][y];
    }
  }
}

void mkVectorFromBoolMap(const bool VALUE_TO_STORE, const bool a[MAP_W][MAP_H],
                         vector<Pos>& out) {
  out.clear();
  for(int x = 0; x < MAP_W; ++x) {
    for(int y = 0; y < MAP_H; ++y) {
      if(a[x][y] == VALUE_TO_STORE) {
        out.push_back(Pos(x, y));
      }
    }
  }
}

void mkBoolMapFromVector(const vector<Pos>& positions, bool out[MAP_W][MAP_H]) {
  resetArray(out, false);
  for(const Pos& p : positions) {out[p.x][p.y] = true;}
}

void getActorCells(const vector<Actor*>& actors, vector<Pos>& out) {
  out.clear();
  for(const auto* const a : actors) {out.push_back(a->pos);}
}

Actor* getFirstActorAtPos(const Pos& pos, ActorState state) {
  for(auto* const actor : GameTime::actors_) {
    if(actor->pos == pos && actor->getState() == state) {
      return actor;
    }
  }
  return nullptr;
}

Mob* getFirstMobAtPos(const Pos& pos) {
  for(auto* const mob : GameTime::mobs_) {
    if(mob->getPos() == pos) {return mob;}
  }
  return nullptr;
}

void mkActorArray(Actor* a[MAP_W][MAP_H]) {
  resetArray(a);

  for(Actor* actor : GameTime::actors_) {
    const Pos& p = actor->pos;
    a[p.x][p.y] = actor;
  }
}

bool isPosInsideMap(const Pos& pos, const bool COUNT_EDGE_AS_INSIDE) {
  if(COUNT_EDGE_AS_INSIDE) {
    return pos.x >= 0 && pos.y >= 0 && pos.x < MAP_W && pos.y < MAP_H;
  } else {
    return pos.x > 0 && pos.y > 0 && pos.x < MAP_W - 1 && pos.y < MAP_H - 1;
  }
}

bool isPosInside(const Pos& pos, const Rect& area) {
  return
    pos.x >= area.p0.x &&
    pos.x <= area.p1.x &&
    pos.y >= area.p0.y &&
    pos.y <= area.p1.y;
}

bool isAreaInsideOther(const Rect& inner, const Rect& outer,
                       const bool COUNT_EQUAL_AS_INSIDE) {
  if(COUNT_EQUAL_AS_INSIDE) {
    return
      inner.p0.x >= outer.p0.x &&
      inner.p1.x <= outer.p1.x &&
      inner.p0.y >= outer.p0.y &&
      inner.p1.y <= outer.p1.y;
  } else {
    return
      inner.p0.x > outer.p0.x &&
      inner.p1.x < outer.p1.x &&
      inner.p0.y > outer.p0.y &&
      inner.p1.y < outer.p1.y;
  }
}

bool isAreaInsideMap(const Rect& area) {
  return isPosInsideMap(area.p0) && isPosInsideMap(area.p1);
}

int kingDist(const int X0, const int Y0, const int X1, const int Y1) {
  return max(abs(X1 - X0), abs(Y1 - Y0));
}

int kingDist(const Pos& p0, const Pos& p1) {
  return max(abs(p1.x - p0.x), abs(p1.y - p0.y));
}

int taxicabDist(const Pos& p0, const Pos& p1) {
  return abs(p1.x - p0.x) + abs(p1.y - p0.y);
}

Pos getClosestPos(const Pos& p, const vector<Pos>& positions) {
  int distToNearest = INT_MAX;
  Pos closestPos;
  for(Pos pCmp : positions) {
    const int CUR_DIST = kingDist(p, pCmp);
    if(CUR_DIST < distToNearest) {
      distToNearest = CUR_DIST;
      closestPos    = pCmp;
    }
  }

  return closestPos;
}

Actor* getRandomClosestActor(const Pos& c, const vector<Actor*>& actors) {
  if(actors.empty())      {return nullptr;}
  if(actors.size() == 1)  {return actors[0];}

  //Find distance to nearest actor(s)
  int distToNearest = INT_MAX;
  for(Actor* actor : actors) {
    const int CUR_DIST = kingDist(c, actor->pos);
    if(CUR_DIST < distToNearest) {
      distToNearest = CUR_DIST;
    }
  }

  assert(distToNearest != INT_MAX);

  //Store all actors with distance equal to the nearest distance
  vector<Actor*> closestActors;
  for(Actor* actor : actors) {
    if(kingDist(c, actor->pos) == distToNearest) {
      closestActors.push_back(actor);
    }
  }

  assert(!closestActors.empty());

  const int ELEMENT = Rnd::range(0, closestActors.size() - 1);

  return closestActors.at(ELEMENT);
}

bool isPosAdj(const Pos& pos1, const Pos& pos2, const bool COUNT_SAME_CELL_AS_ADJ) {
  if(pos1.x < pos2.x - 1)                   {return false;}
  if(pos1.x > pos2.x + 1)                   {return false;}
  if(pos1.y < pos2.y - 1)                   {return false;}
  if(pos1.y > pos2.y + 1)                   {return false;}
  if(pos1.x == pos2.x && pos1.y == pos2.y)  {return COUNT_SAME_CELL_AS_ADJ;}
  return true;
}

TimeData getCurTime() {
  time_t      t   = time(nullptr);
  struct tm*  now = localtime(&t);
  return TimeData(now->tm_year + 1900, now->tm_mon + 1, now->tm_mday,
                  now->tm_hour, now->tm_min, now->tm_sec);
}

bool isValInRange(const int VAL, const Range& range) {
  assert(range.upper >= range.lower);
  return (unsigned int)(VAL - range.lower) <= (unsigned int)(range.upper - range.lower);
}

} //Utils

//------------------------------------------------------ TIME DATA
string TimeData::getTimeStr(const TimeType lowest,
                            const bool ADD_SEPARATORS) const {
  string ret = toStr(year_);

  string monthStr   = (month_   < 10 ? "0" : "") + toStr(month_);
  string dayStr     = (day_     < 10 ? "0" : "") + toStr(day_);
  string hourStr    = (hour_    < 10 ? "0" : "") + toStr(hour_);
  string minuteStr  = (minute_  < 10 ? "0" : "") + toStr(minute_);
  string secondStr  = (second_  < 10 ? "0" : "") + toStr(second_);

  if(lowest >= time_month)  ret += (ADD_SEPARATORS ? "-" : "-") + monthStr;
  if(lowest >= time_day)    ret += (ADD_SEPARATORS ? "-" : "-") + dayStr;
  if(lowest >= time_hour)   ret += (ADD_SEPARATORS ? " " : "_") + hourStr;
  if(lowest >= time_minute) ret += (ADD_SEPARATORS ? ":" : "-") + minuteStr;
  if(lowest >= time_second) ret += (ADD_SEPARATORS ? ":" : "-") + secondStr;

  return ret;
}

namespace DirUtils {

namespace {

const string compassDirNames[3][3] = {
  {"NW", "N", "NE"},
  { "W",  "",  "E",},
  {"SW", "S", "SE"}
};

const double PI_DB            = 3.14159265;
const double ANGLE_45_DB      = 2 * PI_DB / 8;
const double ANGLE_45_HALF_DB = ANGLE_45_DB / 2.0;

const double edge[4] = {
  ANGLE_45_HALF_DB + (ANGLE_45_DB * 0),
  ANGLE_45_HALF_DB + (ANGLE_45_DB * 1),
  ANGLE_45_HALF_DB + (ANGLE_45_DB * 2),
  ANGLE_45_HALF_DB + (ANGLE_45_DB * 3)
};

} //namespace

Dir getDir(const Pos& offset) {
  assert(offset.x >= -1 && offset.y >= -1 && offset.x <= 1 && offset.y <= 1);

  if(offset.y == -1) {
    return offset.x == -1 ? Dir::upLeft :
           offset.x ==  0 ? Dir::up :
           offset.x ==  1 ? Dir::upRight :
           Dir::END;
  }

  if(offset.y == 0) {
    return offset.x == -1 ? Dir::left :
           offset.x ==  0 ? Dir::center :
           offset.x ==  1 ? Dir::right :
           Dir::END;
  }
  if(offset.y == 1) {
    return offset.x == -1 ? Dir::downLeft :
           offset.x ==  0 ? Dir::down :
           offset.x ==  1 ? Dir::downRight :
           Dir::END;
  }
  return Dir::END;
}

Pos getOffset(const Dir dir) {
  assert(dir != Dir::END);

  switch(dir) {
    case Dir::downLeft:   return Pos(-1, 1);
    case Dir::down:       return Pos(0, 1);
    case Dir::downRight:  return Pos(1, 1);
    case Dir::left:       return Pos(-1, 0);
    case Dir::center:     return Pos(0, 0);
    case Dir::right:      return Pos(1, 0);
    case Dir::upLeft:     return Pos(-1, -1);
    case Dir::up:         return Pos(0, -1);
    case Dir::upRight:    return Pos(1, -1);
    case Dir::END:        return Pos(0, 0);
  }
  return Pos(0, 0);
}

Pos getRndAdjPos(const Pos& origin, const bool IS_ORIGIN_ALLOWED) {
  if(IS_ORIGIN_ALLOWED) {
    const int ELEMENT = Rnd::range(0, dirList.size()); //Treat origin as extra element
    return ELEMENT == int(dirList.size()) ? origin : (origin + dirList[ELEMENT]);
  } else {
    return origin + dirList[Rnd::range(0, dirList.size() - 1)];
  }
}

void getCompassDirName(const Pos& fromPos, const Pos& toPos, string& strRef) {

  strRef = "";

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

void getCompassDirName(const Dir dir, string& strRef) {

  const Pos& offset = getOffset(dir);
  strRef = compassDirNames[offset.x + 1][offset.y + 1];
}

void getCompassDirName(const Pos& offset, string& strRef) {
  strRef = compassDirNames[offset.x + 1][offset.y + 1];
}

bool isCardinal(const Pos& d) {
  assert(d.x >= -1 && d.x <= 1 && d.y >= -1 && d.y <= 1);
  return d != 0 && (d.x == 0 || d.y == 0);
}

} //DirUtils
