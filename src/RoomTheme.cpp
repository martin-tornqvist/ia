#include "RoomTheme.h"

#include "Init.h"
#include "MapGen.h"
#include "FeatureData.h"
#include "FeatureFactory.h"
#include "Map.h"
#include "PopulateItems.h"
#include "PopulateMonsters.h"
#include "PopulateTraps.h"
#include "Gods.h"
#include "MapParsing.h"
#include "Utils.h"

using namespace std;

namespace RoomThemeMaking {

RoomThemeId themeMap[MAP_W][MAP_H];

namespace {

int getRandomNrFeaturesForTheme(const RoomThemeId theme) {
  switch(theme) {
    case RoomThemeId::plain:
      return Rnd::oneIn(14) ? 2 : Rnd::oneIn(5) ? 1 : 0;
    case RoomThemeId::human:
      return Rnd::range(3, 6);
    case RoomThemeId::ritual:
      return Rnd::range(1, 5);
    case RoomThemeId::spider:
      return Rnd::range(0, 3);
    case RoomThemeId::crypt:
      return Rnd::range(3, 6);
    case RoomThemeId::monster:
      return Rnd::range(0, 6);
    case RoomThemeId::flooded:
      return 0;
    case RoomThemeId::muddy:
      return 0;
    case RoomThemeId::endOfRoomThemes: {} break;
  }
  return -1;
}

int trySetFeatureToPlace(const FeatureDataT** def, Pos& pos,
                         vector<Pos>& nextToWalls,
                         vector<Pos>& awayFromWalls,
                         vector<const FeatureDataT*> themeFeatureData) {
  TRACE << "RoomThemeMaking::trySetFeatureToPlace()" << endl;

  if(themeFeatureData.empty()) {return -1;}

  const bool IS_NEXT_TO_WALL_AVAIL = nextToWalls.size() != 0;
  const bool IS_AWAY_FROM_WALLS_AVAIL = awayFromWalls.size() != 0;

  if(!IS_NEXT_TO_WALL_AVAIL && !IS_AWAY_FROM_WALLS_AVAIL) {
    TRACE << "RoomThemeMaking: Neither cells next to walls or away from ";
    TRACE << "walls found, returning" << endl;
    def = nullptr;
    return -1;
  }

  const int NR_ATTEMPTS_TO_FIND_POS = 100;
  for(int i = 0; i < NR_ATTEMPTS_TO_FIND_POS; i++) {
    const int NR_DATA       = themeFeatureData.size();
    const int ELEMENT       = Rnd::range(0, NR_DATA - 1);
    const auto* const dTmp  = themeFeatureData.at(ELEMENT);

    if(
      dTmp->featureThemeSpawnRules.getPlacementRule() ==
      PlacementRule::nextToWalls) {
      if(IS_NEXT_TO_WALL_AVAIL) {
        const int POS_ELEMENT =
          Rnd::range(0, nextToWalls.size() - 1);
        pos = nextToWalls.at(POS_ELEMENT);
        *def = dTmp;
        return ELEMENT;
      }
    }

    if(
      dTmp->featureThemeSpawnRules.getPlacementRule() ==
      PlacementRule::awayFromWalls) {
      if(IS_AWAY_FROM_WALLS_AVAIL) {
        const int POS_ELEMENT =
          Rnd::range(0, awayFromWalls.size() - 1);
        pos = awayFromWalls.at(POS_ELEMENT);
        *def = dTmp;
        return ELEMENT;
      }
    }

    if(
      dTmp->featureThemeSpawnRules.getPlacementRule() ==
      PlacementRule::nextToWallsOrAwayFromWalls) {
      if(Rnd::coinToss()) {
        if(IS_NEXT_TO_WALL_AVAIL) {
          const int POS_ELEMENT =
            Rnd::range(0, nextToWalls.size() - 1);
          pos = nextToWalls.at(POS_ELEMENT);
          *def = dTmp;
          return ELEMENT;
        }
      } else {
        if(IS_AWAY_FROM_WALLS_AVAIL) {
          const int POS_ELEMENT =
            Rnd::range(0, awayFromWalls.size() - 1);
          pos = awayFromWalls.at(POS_ELEMENT);
          *def = dTmp;
          return ELEMENT;
        }
      }
    }
  }
  return -1;
}

void eraseAdjacentCellsFromVectors(
  const Pos& pos,  vector<Pos>& nextToWalls, vector<Pos>& awayFromWalls) {
  TRACE_FUNC_BEGIN;
  for(unsigned int i = 0; i < nextToWalls.size(); i++) {
    if(Utils::isPosAdj(pos, nextToWalls.at(i), true)) {
      nextToWalls.erase(nextToWalls.begin() + i);
      i--;
    }
  }
  for(unsigned int i = 0; i < awayFromWalls.size(); i++) {
    if(Utils::isPosAdj(pos, awayFromWalls.at(i), true)) {
      awayFromWalls.erase(awayFromWalls.begin() + i);
      i--;
    }
  }
  TRACE_FUNC_END;
}

int placeThemeFeatures(Room& room) {
  TRACE << "RoomThemeMaking::placeThemeFeatures()" << endl;
  vector<const FeatureDataT*> themeFeatureData;

  for(int i = 0; i < int(FeatureId::endOfFeatureId); i++) {
    const auto* const d = FeatureData::getData((FeatureId)(i));
    if(d->featureThemeSpawnRules.isBelongingToTheme(room.theme_)) {
      themeFeatureData.push_back(d);
    }
  }

  vector<Pos> nextToWalls;
  vector<Pos> awayFromWalls;
  MapPatterns::setPositionsInArea(room.r_, nextToWalls, awayFromWalls);

  vector<int> featuresSpawnCount(themeFeatureData.size(), 0);

  int nrFeaturesLeftToPlace = getRandomNrFeaturesForTheme(room.theme_);

  int nrFeaturesPlaced = 0;

  while(true) {
    if(nrFeaturesLeftToPlace == 0) {
      TRACE << "RoomThemeMaking: Placed enough features, returning" << endl;
      return nrFeaturesPlaced;
    }

    const FeatureDataT* d = nullptr;
    Pos pos(-1, -1);
    const int FEATURE_CANDIDATE_ELEMENT =
      trySetFeatureToPlace(&d, pos, nextToWalls, awayFromWalls,
                           themeFeatureData);

    if(d == nullptr) {
      TRACE << "RoomThemeMaking: Could not find any more spots ";
      TRACE << "to place feature, returning" << endl;
      return nrFeaturesPlaced;
    } else {
      TRACE << "RoomThemeMaking: Placing " << d->name_a << endl;
      FeatureFactory::mk(d->id, pos);
      featuresSpawnCount.at(FEATURE_CANDIDATE_ELEMENT)++;

      nrFeaturesLeftToPlace--;
      nrFeaturesPlaced++;

      //Check if more of this feature can be spawned,
      //if not, delete it from feature candidates
      if(
        featuresSpawnCount.at(FEATURE_CANDIDATE_ELEMENT) >=
        d->featureThemeSpawnRules.getMaxNrInRoom()) {
        featuresSpawnCount.erase(
          featuresSpawnCount.begin() + FEATURE_CANDIDATE_ELEMENT);
        themeFeatureData.erase(
          themeFeatureData.begin() + FEATURE_CANDIDATE_ELEMENT);

        //Are there any more features left to place at all?
        if(themeFeatureData.empty()) {return nrFeaturesPlaced;}
      }

      eraseAdjacentCellsFromVectors(pos, nextToWalls, awayFromWalls);
    }
  }
}

void mkThemeSpecificRoomModifications(Room& room) {
  bool blocked[MAP_W][MAP_H];
  MapParse::parse(CellPred::BlocksMoveCmn(false), blocked);

  switch(room.theme_) {
    case RoomThemeId::flooded:
    case RoomThemeId::muddy: {
      const auto featureId = room.theme_ == RoomThemeId::flooded ?
                             FeatureId::shallowWater : FeatureId::shallowMud;
      for(int y = room.r_.p0.y; y <= room.r_.p1.y; y++) {
        for(int x = room.r_.p0.x; x <= room.r_.p1.x; x++) {
          if(!blocked[x][y]) {
            FeatureFactory::mk(featureId, Pos(x, y));
          }
        }
      }
    } break;

    case RoomThemeId::monster: {
      int nrBloodPut = 0;
      const int NR_TRIES = 1000; //TODO Hacky, needs improving
      for(int i = 0; i < NR_TRIES; i++) {
        for(int y = room.r_.p0.y; y <= room.r_.p1.y; y++) {
          for(int x = room.r_.p0.x; x <= room.r_.p1.x; x++) {
            if(!blocked[x][y]) {
              const int CHANCE_TO_PUT_BLOOD = 40;
              if(Rnd::percentile() < CHANCE_TO_PUT_BLOOD) {
                Map::mkGore(Pos(x, y));
                Map::mkBlood(Pos(x, y));
                nrBloodPut++;
              }
            }
          }
        }
        if(nrBloodPut > 0) {break;}
      }
    } break;

    //Ritual chamber, set a random god for this level, sometimes make gore
    //at altar (or at random pos if no altar)
    case RoomThemeId::ritual: {

      Gods::setRandomGod();

      const int CHANCE_FOR_BLOODY_CHAMBER = 60;
      if(Rnd::percentile() < CHANCE_FOR_BLOODY_CHAMBER) {

        Pos origin(-1, -1);
        vector<Pos> originBucket;
        for(int y = room.r_.p0.y; y <= room.r_.p1.y; y++) {
          for(int x = room.r_.p0.x; x <= room.r_.p1.x; x++) {
            if(Map::cells[x][y].featureStatic->getId() == FeatureId::altar) {
              origin = Pos(x, y);
              y = 999;
              x = 999;
            } else {
              if(!blocked[x][y]) {originBucket.push_back(Pos(x, y));}
            }
          }
        }
        if(!originBucket.empty()) {
          if(origin.x == -1) {
            const int ELEMENT = Rnd::range(0, originBucket.size() - 1);
            origin = originBucket.at(ELEMENT);
          }
          for(int dy = -1; dy <= 1; dy++) {
            for(int dx = -1; dx <= 1; dx++) {
              if(
                (dx == 0 && dy == 0) ||
                (Rnd::percentile() < CHANCE_FOR_BLOODY_CHAMBER / 2)) {
                const Pos pos = origin + Pos(dx, dy);
                if(!blocked[pos.x][pos.y]) {
                  Map::mkGore(pos);
                  Map::mkBlood(pos);
                }
              }
            }
          }
        }
      }
    } break;

    case RoomThemeId::plain:            {} break;
    case RoomThemeId::human:            {} break;
    case RoomThemeId::spider:           {} break;
    case RoomThemeId::crypt:            {} break;
    case RoomThemeId::endOfRoomThemes:  {} break;
  }
}

void applyThemeToRoom(Room& room) {
  placeThemeFeatures(room);

  mkThemeSpecificRoomModifications(room);

  switch(room.theme_) {
    case RoomThemeId::plain:   {room.descr_ = "";} break;
    case RoomThemeId::human:   {room.descr_ = "";} break;
    case RoomThemeId::ritual:  {room.descr_ = "";} break;
    case RoomThemeId::spider:  {room.descr_ = "";} break;
    case RoomThemeId::crypt:   {room.descr_ = "";} break;
    case RoomThemeId::monster: {room.descr_ = "";} break;
    case RoomThemeId::flooded: {room.descr_ = "";} break;
    case RoomThemeId::muddy:   {room.descr_ = "";} break;
    case RoomThemeId::endOfRoomThemes: {} break;
  }
}

int nrThemeInMap(const RoomThemeId theme) {
  int nr = 0;
  for(Room* r : Map::roomList) {if(r->theme_ == theme) nr++;}
  return nr;
}

bool isThemeAllowed(const Room* const room, const RoomThemeId theme,
                    const bool blocked[MAP_W][MAP_H]) {
  (void)blocked;

  const int ROOM_W  = room->r_.p1.x - room->r_.p0.x + 1;
  const int ROOM_H  = room->r_.p1.y - room->r_.p0.y + 1;
  const int MIN_DIM = min(ROOM_W, ROOM_H);
  const int MAX_DIM = max(ROOM_W, ROOM_H);

  switch(theme) {
    case RoomThemeId::plain: {return true;} break;

    case RoomThemeId::human: {
      return MIN_DIM >= 4 && MAX_DIM <= 8 &&
             nrThemeInMap(RoomThemeId::human) < 3;
    } break;

    case RoomThemeId::ritual: {
      return MIN_DIM >= 4 && MAX_DIM <= 8 &&
             nrThemeInMap(RoomThemeId::ritual) == 0;
    } break;

    case RoomThemeId::spider: {
      return MIN_DIM >= 3 && MAX_DIM <= 8;
    } break;

    case RoomThemeId::crypt: {
      return MIN_DIM >= 3 && MAX_DIM <= 12 &&
             nrThemeInMap(RoomThemeId::crypt) < 3;
    } break;

    case RoomThemeId::monster: {
      return MIN_DIM >= 4 && MAX_DIM <= 8 &&
             nrThemeInMap(RoomThemeId::monster) < 3;
    } break;

    default: {} break;
  }
  return true;
}

void mkRoomDarkWithChance(const Room& room) {
  const int ROOM_W = room.r_.p1.x - room.r_.p0.x + 1;
  const int ROOM_H = room.r_.p1.y - room.r_.p0.y + 1;
  if(ROOM_W >= 4 && ROOM_H >= 4) {
    int chanceToMkDark = 0;

    switch(room.theme_) {
      case RoomThemeId::plain:     chanceToMkDark = 5;   break;
      case RoomThemeId::human:     chanceToMkDark = 10;  break;
      case RoomThemeId::ritual:    chanceToMkDark = 15;  break;
      case RoomThemeId::spider:    chanceToMkDark = 33;  break;
      case RoomThemeId::crypt:     chanceToMkDark = 75;  break;
      case RoomThemeId::monster:   chanceToMkDark = 75;  break;
      case RoomThemeId::flooded:   chanceToMkDark = 50;  break;
      case RoomThemeId::muddy:     chanceToMkDark = 50;  break;
      default: break;
    }

    chanceToMkDark += Map::dlvl - 1;

    if(Rnd::range(1, 100) < chanceToMkDark) {
      for(int y = room.r_.p0.y; y <= room.r_.p1.y; y++) {
        for(int x = room.r_.p0.x; x <= room.r_.p1.x; x++) {
          Map::cells[x][y].isDark = true;
        }
      }
    }
  }
}

void assignRoomThemes() {
  TRACE_FUNC_BEGIN;

  for(int y = 0; y < MAP_H; y++) {
    for(int x = 0; x < MAP_W; x++) {
      themeMap[x][y] = RoomThemeId::plain;
    }
  }

  const int MIN_DIM = 3;
  const int MAX_DIM = 12;
  const int NR_NON_PLAIN_THEMED = Rnd::range(1, 4);

  const int NR_ROOMS = Map::roomList.size();

  vector<bool> isAssigned(NR_ROOMS, false);

  TRACE << "RoomThemeMaking: Assigning plain theme to rooms with ";
  TRACE << "wrong dimensions" << endl;
  for(int i = 0; i < NR_ROOMS; i++) {
    Room* const room = Map::roomList.at(i);

    // Check dimensions, assign plain if too small or too big
    if(!isAssigned.at(i)) {
      const int W = room->r_.p1.x - room->r_.p0.x + 1;
      const int H = room->r_.p1.y - room->r_.p0.y + 1;
      if(W < MIN_DIM || W > MAX_DIM || H < MIN_DIM || H > MAX_DIM) {
        room->theme_ = RoomThemeId::plain;
        isAssigned.at(i) = true;
        continue;
      }
    }
  }

  TRACE << "RoomThemeMaking: Trying to set non-plain themes ";
  TRACE << "for some rooms" << endl;
  bool blocked[MAP_W][MAP_H];
  MapParse::parse(CellPred::BlocksMoveCmn(false), blocked);
  const int NR_TRIES_TO_ASSIGN = 100;
  for(int i = 0; i < NR_NON_PLAIN_THEMED; i++) {
    for(int ii = 0; ii < NR_TRIES_TO_ASSIGN; ii++) {
      const int ELEMENT = Rnd::range(0, NR_ROOMS - 1);
      if(!isAssigned.at(ELEMENT)) {
        const RoomThemeId theme =
          (RoomThemeId)(Rnd::range(1, int(RoomThemeId::endOfRoomThemes) - 1));
        Room* const room = Map::roomList.at(ELEMENT);

        if(isThemeAllowed(room, theme, blocked)) {
          room->theme_ = theme;
          TRACE << "RoomThemeMaking: Assigned non-plain theme";
          TRACE << "(" << int(theme) << ") to room" << endl;
          isAssigned.at(ELEMENT) = true;
          for(int y = room->r_.p0.y; y < room->r_.p1.y; y++) {
            for(int x = room->r_.p0.x; x < room->r_.p1.x; x++) {
              themeMap[x][y] = theme;
            }
          }
          break;
        }
      }
    }
  }

  TRACE << "RoomThemeMaking: Assigning plain theme to remaining rooms" << endl;
  for(int i = 0; i < NR_ROOMS; i++) {
    if(!isAssigned.at(i)) {
      Map::roomList.at(i)->theme_ = RoomThemeId::plain;
      isAssigned.at(i) = true;
    }
  }

  TRACE_FUNC_END;
}

} //namespace

void run() {
  TRACE_FUNC_BEGIN;

  Gods::setNoGod();

  assignRoomThemes();

  for(Room* const room : Map::roomList) {
    applyThemeToRoom(*room);
    mkRoomDarkWithChance(*room);
  }

  TRACE_FUNC_END;
}

} //RoomThemeMaking
