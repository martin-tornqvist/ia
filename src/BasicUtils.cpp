#include "BasicUtils.h"

#include <algorithm>
#include <assert.h>

#include "Engine.h"

#include "Converters.h"
#include "GameTime.h"

//------------------------------------------------------ BASIC UTILS
void BasicUtils::makeVectorFromBoolMap(const bool VALUE_TO_STORE,
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

void BasicUtils::getActorPositions(const vector<Actor*>& actors,
                                   vector<Pos>& vectorRef) {
  vectorRef.resize(0);
  const unsigned int NR_ACTORS = actors.size();
  for(unsigned int i = 0; i < NR_ACTORS; i++) {
    vectorRef.push_back(actors.at(i)->pos);
  }
}


Actor* BasicUtils::getActorAtPos(const Pos& pos) const {
  for(Actor * actor : eng.gameTime->actors_) {
    if(actor->pos == pos && actor->deadState == actorDeadState_alive) {
      return actor;
    }
  }
  return NULL;
}

void BasicUtils::makeActorArray(Actor* a[MAP_W][MAP_H]) {
  resetArray(a);

  for(Actor * actor : eng.gameTime->actors_) {
    const Pos& p = actor->pos;
    a[p.x][p.y] = actor;
  }
}

Pos BasicUtils::getClosestPos(
  const Pos& c, const vector<Pos>& positions) const {
  int distToNearest = INT_MAX;
  int closestElement = 0;
  const int NR_POSITIONS = positions.size();
  for(int i = 0; i < NR_POSITIONS; i++) {
    const int CUR_DIST = eng.basicUtils->chebyshevDist(c, positions.at(i));
    if(CUR_DIST < distToNearest) {
      distToNearest = CUR_DIST;
      closestElement = i;
    }
  }

  return positions.at(closestElement);
}


Actor* BasicUtils::getRandomClosestActor(
  const Pos& c, const vector<Actor*>& actors) const {

  if(actors.empty()) return NULL;

  //Find distance to nearest actor(s)
  int distToNearest = INT_MAX;
  for(Actor * actor : actors) {
    const int CUR_DIST = chebyshevDist(c, actor->pos);
    if(CUR_DIST < distToNearest) {
      distToNearest = CUR_DIST;
    }
  }

  assert(distToNearest > 0);
  assert(distToNearest != INT_MAX);

  //Store all actors with distance equal to the nearest distance
  vector<Actor*> closestActors;
  for(Actor * actor : actors) {
    if(chebyshevDist(c, actor->pos) == distToNearest) {
      closestActors.push_back(actor);
    }
  }

  assert(closestActors.empty() == false);

  const int ELEMENT = eng.dice.range(0, closestActors.size() - 1);

  return closestActors.at(ELEMENT);
}

bool BasicUtils::isPosAdj(
  const Pos& pos1, const Pos& pos2,
  const bool COUNT_SAME_CELL_AS_NEIGHBOUR) const {

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

TimeData BasicUtils::getCurrentTime() const {
  time_t t = time(0);
  struct tm* now = localtime(&t);
  return TimeData(now->tm_year + 1900, now->tm_mon + 1, now->tm_mday,
                  now->tm_hour, now->tm_min, now->tm_sec);
}

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

//------------------------------------------------------ DIRECTION CONVERTER
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

