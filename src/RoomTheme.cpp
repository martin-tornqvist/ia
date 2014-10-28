#include "RoomTheme.h"

#include "Init.h"
#include "MapGen.h"
#include "Room.h"
#include "FeatureData.h"
#include "Map.h"
#include "PopulateItems.h"
#include "PopulateMonsters.h"
#include "PopulateTraps.h"
#include "Gods.h"
#include "MapParsing.h"
#include "Utils.h"

using namespace std;

namespace RoomThemeMaking {

namespace {

int getRndNrFeaturesForRoomType(const RoomType type) {
  switch(type) {
    case RoomType::plain:   return Rnd::oneIn(14) ? 2 : Rnd::oneIn(5) ? 1 : 0;
    case RoomType::human:   return Rnd::range(3, 6);
    case RoomType::ritual:  return Rnd::range(1, 5);
    case RoomType::spider:  return Rnd::range(0, 3);
    case RoomType::crypt:   return Rnd::range(3, 6);
    case RoomType::monster: return Rnd::range(0, 6);
    case RoomType::flooded: return 0;
    case RoomType::muddy:   return 0;
    case RoomType::END_OF_STD_ROOMS:
    case RoomType::river:
    case RoomType::cave:
    case RoomType::corridorJunction:
    case RoomType::crumbleRoom: break;
  }
  return -1;
}

int trySetFeatureToPlace(const FeatureDataT** def, Pos& pos,
                         vector<Pos>& nextToWalls,
                         vector<Pos>& awayFromWalls,
                         vector<const FeatureDataT*> typeFeatureData) {
  TRACE_FUNC_BEGIN;

  if(typeFeatureData.empty()) {return -1;}

  const bool IS_NEXT_TO_WALL_AVAIL    = nextToWalls.size() != 0;
  const bool IS_AWAY_FROM_WALLS_AVAIL = awayFromWalls.size() != 0;

  if(!IS_NEXT_TO_WALL_AVAIL && !IS_AWAY_FROM_WALLS_AVAIL) {
    def = nullptr;
    TRACE_FUNC_END << "No eligible cells found" << endl;
    return -1;
  }

  const int NR_ATTEMPTS_TO_FIND_POS = 100;
  for(int i = 0; i < NR_ATTEMPTS_TO_FIND_POS; ++i) {
    const int NR_DATA       = typeFeatureData.size();
    const int ELEMENT       = Rnd::range(0, NR_DATA - 1);
    const auto* const dTmp  = typeFeatureData.at(ELEMENT);

    if(
      dTmp->themeSpawnRules.getPlacementRule() ==
      PlacementRule::nextToWalls) {
      if(IS_NEXT_TO_WALL_AVAIL) {
        pos   = nextToWalls.at(Rnd::range(0, nextToWalls.size() - 1));
        *def  = dTmp;
        TRACE_FUNC_END;
        return ELEMENT;
      }
    }

    if(
      dTmp->themeSpawnRules.getPlacementRule() ==
      PlacementRule::awayFromWalls) {
      if(IS_AWAY_FROM_WALLS_AVAIL) {
        pos   = awayFromWalls.at(Rnd::range(0, awayFromWalls.size() - 1));
        *def  = dTmp;
        TRACE_FUNC_END;
        return ELEMENT;
      }
    }

    if(
      dTmp->themeSpawnRules.getPlacementRule() == PlacementRule::either) {
      if(Rnd::coinToss()) {
        if(IS_NEXT_TO_WALL_AVAIL) {
          pos   = nextToWalls.at(Rnd::range(0, nextToWalls.size() - 1));
          *def  = dTmp;
          TRACE_FUNC_END;
          return ELEMENT;
        }
      } else {
        if(IS_AWAY_FROM_WALLS_AVAIL) {
          pos   = awayFromWalls.at(Rnd::range(0, awayFromWalls.size() - 1));
          *def  = dTmp;
          TRACE_FUNC_END;
          return ELEMENT;
        }
      }
    }
  }
  TRACE_FUNC_END;
  return -1;
}

void eraseAdjacentCellsFromVectors(const Pos& pos,
                                   vector<Pos>& nextToWalls,
                                   vector<Pos>& awayFromWalls) {
  TRACE_FUNC_BEGIN;
  for(int i = 0; i < int(nextToWalls.size()); ++i) {
    if(Utils::isPosAdj(pos, nextToWalls.at(i), true)) {
      nextToWalls.erase(nextToWalls.begin() + i);
      i--;
    }
  }
  for(int i = 0; i < int(awayFromWalls.size()); ++i) {
    if(Utils::isPosAdj(pos, awayFromWalls.at(i), true)) {
      awayFromWalls.erase(awayFromWalls.begin() + i);
      i--;
    }
  }
  TRACE_FUNC_END;
}

int placeThemeFeatures(Room& room) {
  TRACE_FUNC_BEGIN;
  vector<const FeatureDataT*> featureBucket;

  for(int i = 0; i < int(FeatureId::END); ++i) {
    const auto* const d = &FeatureData::getData((FeatureId)(i));
    if(d->themeSpawnRules.isBelongingToRoomType(room.type_)) {
      featureBucket.push_back(d);
    }
  }

  vector<Pos> nextToWalls;
  vector<Pos> awayFromWalls;
  MapPatterns::setCellsInArea(room.r_, nextToWalls, awayFromWalls);

  vector<int> spawnCount(featureBucket.size(), 0);

  int nrFeaturesLeftToPlace = getRndNrFeaturesForRoomType(room.type_);

  int nrFeaturesPlaced = 0;

  while(true) {
    if(nrFeaturesLeftToPlace == 0) {
      TRACE_FUNC_END << "Placed enough features" << endl;
      return nrFeaturesPlaced;
    }

    const FeatureDataT* d = nullptr;
    Pos pos(-1, -1);
    const int FEATURE_ELEMENT =
      trySetFeatureToPlace(&d, pos, nextToWalls, awayFromWalls, featureBucket);

    if(d) {
      TRACE << "Placing feature" << endl;
      Map::put(static_cast<Rigid*>(d->mkObj(pos)));
      spawnCount.at(FEATURE_ELEMENT)++;

      nrFeaturesLeftToPlace--;
      nrFeaturesPlaced++;

      //Check if more of this feature can be spawned. If not, remote it.
      if(
        spawnCount.at(FEATURE_ELEMENT) >= d->themeSpawnRules.getMaxNrInRoom()) {
        spawnCount   .erase(spawnCount   .begin() + FEATURE_ELEMENT);
        featureBucket.erase(featureBucket.begin() + FEATURE_ELEMENT);

        if(featureBucket.empty()) {
          TRACE_FUNC_END << "No more features to place" << endl;
          return nrFeaturesPlaced;
        }
      }
      eraseAdjacentCellsFromVectors(pos, nextToWalls, awayFromWalls);
    } else {
      TRACE_FUNC_END << "No remaining positions to place feature" << endl;
      return nrFeaturesPlaced;
    }
  }
}

void mkThemeSpecificRoomModifications(Room& room) {
  bool blocked[MAP_W][MAP_H];
  MapParse::parse(CellPred::BlocksMoveCmn(false), blocked);

  switch(room.type_) {
    case RoomType::flooded:
    case RoomType::muddy: {
      for(int y = room.r_.p0.y; y <= room.r_.p1.y; ++y) {
        for(int x = room.r_.p0.x; x <= room.r_.p1.x; ++x) {
          if(!blocked[x][y]) {
            LiquidShallow* const liquid = new LiquidShallow(Pos(x, y));
            liquid->type_ =
              room.type_ == RoomType::flooded ? LiquidType::water : LiquidType::mud;
            Map::put(liquid);
          }
        }
      }
    } break;

    case RoomType::monster: {
      int nrBloodPut = 0;
      const int NR_TRIES = 1000; //TODO Hacky, needs improving
      for(int i = 0; i < NR_TRIES; ++i) {
        for(int y = room.r_.p0.y; y <= room.r_.p1.y; ++y) {
          for(int x = room.r_.p0.x; x <= room.r_.p1.x; ++x) {
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
    case RoomType::ritual: {

      Gods::setRandomGod();

      const int CHANCE_FOR_BLOODY_CHAMBER = 60;
      if(Rnd::percentile() < CHANCE_FOR_BLOODY_CHAMBER) {

        Pos origin(-1, -1);
        vector<Pos> originBucket;
        for(int y = room.r_.p0.y; y <= room.r_.p1.y; ++y) {
          for(int x = room.r_.p0.x; x <= room.r_.p1.x; ++x) {
            if(Map::cells[x][y].rigid->getId() == FeatureId::altar) {
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
          for(int dx = -1; dx <= 1; ++dx) {
            for(int dy = -1; dy <= 1; ++dy) {
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

    case RoomType::plain:
    case RoomType::human:
    case RoomType::spider:
    case RoomType::crypt:
    case RoomType::END_OF_STD_ROOMS:
    case RoomType::cave:
    case RoomType::river:
    case RoomType::corridorJunction:
    case RoomType::crumbleRoom: break;
  }
}

void applyThemeToRoom(Room& room) {
  placeThemeFeatures(room);

  mkThemeSpecificRoomModifications(room);

  switch(room.type_) {
    case RoomType::plain:   {room.descr_ = "";} break;
    case RoomType::human:   {room.descr_ = "";} break;
    case RoomType::ritual:  {room.descr_ = "";} break;
    case RoomType::spider:  {room.descr_ = "";} break;
    case RoomType::crypt:   {room.descr_ = "";} break;
    case RoomType::monster: {room.descr_ = "";} break;
    case RoomType::flooded: {room.descr_ = "";} break;
    case RoomType::muddy:   {room.descr_ = "";} break;
    case RoomType::END_OF_STD_ROOMS:
    case RoomType::cave:
    case RoomType::river:
    case RoomType::corridorJunction:
    case RoomType::crumbleRoom: break;
  }
}

int nrOfRoomTypeInMap(const RoomType type) {
  int nr = 0;
  for(Room* r : Map::roomList) {if(r->type_ == type) nr++;}
  return nr;
}

bool isRoomTypeAllowed(const Room& room, const RoomType type,
                       const bool blocked[MAP_W][MAP_H]) {
  (void)blocked;

  //Note: This is a valid method to calculate the room size, since at this
  //point we are only dealing with rectangular rooms.
  const int R_W  = room.r_.p1.x - room.r_.p0.x + 1;
  const int R_H  = room.r_.p1.y - room.r_.p0.y + 1;
  const int MIN_DIM = min(R_W, R_H);
  const int MAX_DIM = max(R_W, R_H);

  switch(type) {
    case RoomType::plain:
    case RoomType::flooded:
    case RoomType::muddy: {return true;} break;

    case RoomType::human: {
      return MIN_DIM >= 4 && MAX_DIM <= 8 &&
             nrOfRoomTypeInMap(RoomType::human) < 3;
    } break;

    case RoomType::ritual: {
      return MIN_DIM >= 4 && MAX_DIM <= 8 &&
             nrOfRoomTypeInMap(RoomType::ritual) == 0;
    } break;

    case RoomType::spider: {
      return MIN_DIM >= 3 && MAX_DIM <= 8;
    } break;

    case RoomType::crypt: {
      return MIN_DIM >= 3 && MAX_DIM <= 12 &&
             nrOfRoomTypeInMap(RoomType::crypt) < 3;
    } break;

    case RoomType::monster: {
      return MIN_DIM >= 4 && MAX_DIM <= 8 &&
             nrOfRoomTypeInMap(RoomType::monster) < 3;
    } break;

    case RoomType::END_OF_STD_ROOMS:
    case RoomType::cave:
    case RoomType::river:
    case RoomType::corridorJunction:
    case RoomType::crumbleRoom: break;
  }
  return false;
}

void mkRoomDarkWithChance(const Room& room) {
  const int ROOM_W = room.r_.p1.x - room.r_.p0.x + 1;
  const int ROOM_H = room.r_.p1.y - room.r_.p0.y + 1;
  if(ROOM_W >= 4 && ROOM_H >= 4) {
    int chanceToMkDark = 0;

    switch(room.type_) {
      case RoomType::plain:     chanceToMkDark = 5;   break;
      case RoomType::human:     chanceToMkDark = 10;  break;
      case RoomType::ritual:    chanceToMkDark = 15;  break;
      case RoomType::spider:    chanceToMkDark = 33;  break;
      case RoomType::crypt:     chanceToMkDark = 75;  break;
      case RoomType::monster:   chanceToMkDark = 75;  break;
      case RoomType::flooded:   chanceToMkDark = 50;  break;
      case RoomType::muddy:     chanceToMkDark = 50;  break;
      default: break;
    }

    chanceToMkDark += Map::dlvl - 1;

    if(Rnd::range(1, 100) < chanceToMkDark) {
      for(int y = room.r_.p0.y; y <= room.r_.p1.y; ++y) {
        for(int x = room.r_.p0.x; x <= room.r_.p1.x; ++x) {
          Map::cells[x][y].isDark = true;
        }
      }
    }
  }
}

void assignRoomThemes() {
  TRACE_FUNC_BEGIN;

  TRACE << "Resetting all standard rooms to plain";
  for(Room* r : Map::roomList) {
    if(int(r->type_) < int(RoomType::END_OF_STD_ROOMS)) {
      r->type_ = RoomType::plain;
    }
  }

  const int MIN_DIM = 3;
  const int MAX_DIM = 12;
  const int NR_NON_PLAIN_THEMED = Rnd::range(1, 4);

  const int NR_ROOMS = Map::roomList.size();

  vector<bool> isAssigned(NR_ROOMS, false);

  TRACE << "Marking non-standard rooms as already assigned, and standard "
        << "rooms with wrong size permanently as plain" << endl;
  for(int i = 0; i < NR_ROOMS; ++i) {
    Room* const room = Map::roomList.at(i);
    if(int(room->type_) > int(RoomType::END_OF_STD_ROOMS)) {
      isAssigned.at(i) = true;
    } else if(int(room->type_) < int(RoomType::END_OF_STD_ROOMS)) {
      // Check dimensions, keep room as plain if too small or too big
      if(!isAssigned.at(i)) {
        const int W = room->r_.p1.x - room->r_.p0.x + 1;
        const int H = room->r_.p1.y - room->r_.p0.y + 1;
        if(W < MIN_DIM || W > MAX_DIM || H < MIN_DIM || H > MAX_DIM) {
          isAssigned.at(i) = true;
          continue;
        }
      }
    }
  }

  TRACE << "Trying to set non-plain types for some rooms" << endl;
  bool blocked[MAP_W][MAP_H];
  MapParse::parse(CellPred::BlocksMoveCmn(false), blocked);
  const int NR_TRIES_TO_ASSIGN = 100;
  for(int i = 0; i < NR_NON_PLAIN_THEMED; ++i) {
    for(int ii = 0; ii < NR_TRIES_TO_ASSIGN; ++ii) {
      const int ELEMENT = Rnd::range(0, NR_ROOMS - 1);
      if(!isAssigned.at(ELEMENT)) {
        const RoomType type =
          (RoomType)(Rnd::range(1, int(RoomType::END_OF_STD_ROOMS) - 1));
        Room* const room = Map::roomList.at(ELEMENT);

        if(isRoomTypeAllowed(*room, type, blocked)) {
          room->type_ = type;
          TRACE << "Assigned non-plain type ("
                << int(type) << ") to room" << endl;
          isAssigned.at(ELEMENT) = true;
          break;
        }
      }
    }
  }

  TRACE << "Assigning plain theme to remaining rooms" << endl;
  for(int i = 0; i < NR_ROOMS; ++i) {
    Room* const room = Map::roomList.at(i);
    if(!isAssigned.at(i) && int(room->type_) < int(RoomType::END_OF_STD_ROOMS)) {
      room->type_       = RoomType::plain;
      isAssigned.at(i)  = true;
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
    if(int(room->type_) < int(RoomType::END_OF_STD_ROOMS)) {
      applyThemeToRoom(*room);
      mkRoomDarkWithChance(*room);
    }
  }

  TRACE_FUNC_END;
}

} //RoomThemeMaking
