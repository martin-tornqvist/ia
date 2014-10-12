#include "PopulateMonsters.h"

#include <algorithm>

#include "Init.h"
#include "Room.h"
#include "FeatureTrap.h"
#include "Map.h"
#include "ActorFactory.h"
#include "ActorMonster.h"
#include "ActorPlayer.h"
#include "MapParsing.h"
#include "Utils.h"

using namespace std;

namespace PopulateMonsters {

namespace {

int getRandomOutOfDepth() {
  if(Map::dlvl == 0)                  {return 0;}
  if(Rnd::oneIn(40) && Map::dlvl > 1) {return 5;}
  if(Rnd::oneIn(5))                   {return 2;}

  return 0;
}

void mkListOfMonstersCanAutoSpawn(const int NR_LVLS_OUT_OF_DEPTH,
                                  vector<ActorId>& listRef) {
  listRef.resize(0);

  const int EFFECTIVE_DLVL =
    max(1, min(LAST_CAVERN_LVL, Map::dlvl + NR_LVLS_OUT_OF_DEPTH));

  for(const auto& d : ActorData::data) {
    if(
      d.id != ActorId::player           &&
      d.isAutoSpawnAllowed              &&
      d.nrLeftAllowedToSpawn != 0       &&
      EFFECTIVE_DLVL >= d.spawnMinDLVL  &&
      EFFECTIVE_DLVL <= d.spawnMaxDLVL) {
      listRef.push_back(d.id);
    }
  }
}

void mkGroupOfRandomAt(const vector<Pos>& sortedFreeCellsVector,
                       bool blocked[MAP_W][MAP_H],
                       const int NR_LVLS_OUT_OF_DEPTH_ALLOWED,
                       const bool IS_ROAMING_ALLOWED) {
  vector<ActorId> idBucket;
  mkListOfMonstersCanAutoSpawn(NR_LVLS_OUT_OF_DEPTH_ALLOWED, idBucket);

  if(!idBucket.empty()) {
    const ActorId id = idBucket.at(Rnd::range(0, idBucket.size() - 1));
    mkGroupAt(id, sortedFreeCellsVector, blocked, IS_ROAMING_ALLOWED);
  }
}

bool mkGroupOfRandomNativeToRoomTypeAt(
  const RoomType roomType, const vector<Pos>& sortedFreeCellsVector,
  bool blocked[MAP_W][MAP_H], const bool IS_ROAMING_ALLOWED) {

  TRACE_FUNC_BEGIN;

  const int NR_LVLS_OUT_OF_DEPTH_ALLOWED = getRandomOutOfDepth();
  vector<ActorId> idBucket;
  mkListOfMonstersCanAutoSpawn(NR_LVLS_OUT_OF_DEPTH_ALLOWED, idBucket);

  for(size_t i = 0; i < idBucket.size(); ++i) {
    const ActorDataT& d = ActorData::data[int(idBucket.at(i))];
    bool isMonsterNativeToRoom = false;
    for(size_t iNative = 0; iNative < d.nativeRooms.size(); iNative++) {
      if(d.nativeRooms.at(iNative) == roomType) {
        isMonsterNativeToRoom = true;
        break;
      }
    }
    if(!isMonsterNativeToRoom) {
      idBucket.erase(idBucket.begin() + i);
      i--;
    }
  }

  if(idBucket.empty()) {
    TRACE << "Found no valid monsters to spawn at room theme ("
          << toStr(int(roomType)) + ")" << endl;
    TRACE_FUNC_END;
    return false;
  } else {
    const ActorId id = idBucket.at(Rnd::range(0, idBucket.size() - 1));
    mkGroupAt(id, sortedFreeCellsVector, blocked, IS_ROAMING_ALLOWED);
    TRACE_FUNC_END;
    return true;
  }
}

} //namespace

void trySpawnDueToTimePassed() {
  TRACE_FUNC_BEGIN;

  bool blocked[MAP_W][MAP_H];
  MapParse::parse(CellPred::BlocksMoveCmn(true), blocked);

  const int MIN_DIST_TO_PLAYER = FOV_STD_RADI_INT + 3;

  const Pos& playerPos = Map::player->pos;
  const int X0 = max(0, playerPos.x - MIN_DIST_TO_PLAYER);
  const int Y0 = max(0, playerPos.y - MIN_DIST_TO_PLAYER);
  const int X1 = min(MAP_W - 1, playerPos.x + MIN_DIST_TO_PLAYER);
  const int Y1 = min(MAP_H - 1, playerPos.y + MIN_DIST_TO_PLAYER);

  for(int x = X0; x <= X1; ++x) {
    for(int y = Y0; y <= Y1; ++y) {
      blocked[x][y] = true;
    }
  }

  vector<Pos> freeCellsVector;
  for(int y = 1; y < MAP_H - 1; ++y) {
    for(int x = 1; x < MAP_W - 1; ++x) {
      if(!blocked[x][y]) {freeCellsVector.push_back(Pos(x, y));}
    }
  }

  if(!freeCellsVector.empty()) {

    const int ELEMENT = Rnd::range(0, freeCellsVector.size() - 1);
    const Pos& origin = freeCellsVector.at(ELEMENT);

    mkSortedFreeCellsVector(origin, blocked, freeCellsVector);

    if(!freeCellsVector.empty()) {
      if(Map::cells[origin.x][origin.y].isExplored) {
        const int NR_OOD = getRandomOutOfDepth();
        mkGroupOfRandomAt(freeCellsVector, blocked, NR_OOD, true);
      }
    }
  }
  TRACE_FUNC_END;
}

void populateCaveLvl() {
  const int NR_GROUPS_ALLOWED = Rnd::range(6, 7);

  bool blocked[MAP_W][MAP_H];

  const int MIN_DIST_FROM_PLAYER = FOV_STD_RADI_INT - 2;
  MapParse::parse(CellPred::BlocksMoveCmn(true), blocked);

  const Pos& playerPos = Map::player->pos;

  const int X0 = max(0, playerPos.x - MIN_DIST_FROM_PLAYER);
  const int Y0 = max(0, playerPos.y - MIN_DIST_FROM_PLAYER);
  const int X1 = min(MAP_W - 1, playerPos.x + MIN_DIST_FROM_PLAYER) - 1;
  const int Y1 = min(MAP_H - 1, playerPos.y + MIN_DIST_FROM_PLAYER) - 1;

  for(int y = Y0; y <= Y1; ++y) {
    for(int x = X0; x <= X1; ++x) {
      blocked[x][y] = true;
    }
  }

  for(int i = 0; i < NR_GROUPS_ALLOWED; ++i) {
    vector<Pos> originBucket;
    for(int y = 1; y < MAP_H - 1; ++y) {
      for(int x = 1; x < MAP_W - 1; ++x) {
        if(!blocked[x][y]) {originBucket.push_back(Pos(x, y));}
      }
    }
    const int ELEMENT = Rnd::range(0, originBucket.size() - 1);
    const Pos origin = originBucket.at(ELEMENT);
    vector<Pos> sortedFreeCellsVector;
    mkSortedFreeCellsVector(origin, blocked, sortedFreeCellsVector);
    if(!sortedFreeCellsVector.empty()) {
      mkGroupOfRandomAt(sortedFreeCellsVector, blocked,
                        getRandomOutOfDepth(), true);
    }
  }
}

void populateIntroLvl() {
  const int NR_GROUPS_ALLOWED = 2; //Rnd::range(2, 3);

  bool blocked[MAP_W][MAP_H];

  const int MIN_DIST_FROM_PLAYER = FOV_STD_RADI_INT + 3;
  MapParse::parse(CellPred::BlocksMoveCmn(true), blocked);

  const Pos& playerPos = Map::player->pos;

  const int X0 = max(0, playerPos.x - MIN_DIST_FROM_PLAYER);
  const int Y0 = max(0, playerPos.y - MIN_DIST_FROM_PLAYER);
  const int X1 = min(MAP_W - 1, playerPos.x + MIN_DIST_FROM_PLAYER) - 1;
  const int Y1 = min(MAP_H - 1, playerPos.y + MIN_DIST_FROM_PLAYER) - 1;
  for(int y = Y0; y <= Y1; ++y) {
    for(int x = X0; x <= X1; ++x) {
      blocked[x][y] = true;
    }
  }

  for(int i = 0; i < NR_GROUPS_ALLOWED; ++i) {
    vector<Pos> originBucket;
    for(int y = 1; y < MAP_H - 1; ++y) {
      for(int x = 1; x < MAP_W - 1; ++x) {
        if(!blocked[x][y]) {originBucket.push_back(Pos(x, y));}
      }
    }
    const int ELEMENT = Rnd::range(0, originBucket.size() - 1);
    const Pos origin = originBucket.at(ELEMENT);
    vector<Pos> sortedFreeCellsVector;
    mkSortedFreeCellsVector(origin, blocked, sortedFreeCellsVector);
    if(!sortedFreeCellsVector.empty()) {
      mkGroupAt(ActorId::wolf, sortedFreeCellsVector, blocked, true);
    }
  }
}

void populateStdLvl() {
  const int NR_GROUPS_ALLOWED_ON_MAP = Rnd::range(5, 9);
  int nrGroupsSpawned = 0;

  bool blocked[MAP_W][MAP_H];

  const int MIN_DIST_FROM_PLAYER = FOV_STD_RADI_INT - 1;

  MapParse::parse(CellPred::BlocksMoveCmn(true), blocked);

  const Pos& playerPos = Map::player->pos;

  const int X0 = max(0, playerPos.x - MIN_DIST_FROM_PLAYER);
  const int Y0 = max(0, playerPos.y - MIN_DIST_FROM_PLAYER);
  const int X1 = min(MAP_W - 1, playerPos.x + MIN_DIST_FROM_PLAYER);
  const int Y1 = min(MAP_H - 1, playerPos.y + MIN_DIST_FROM_PLAYER);
  for(int y = Y0; y <= Y1; ++y) {
    for(int x = X0; x <= X1; ++x) {
      blocked[x][y] = true;
    }
  }

  //First, attempt to populate all non-plain themed rooms
  for(Room* const room : Map::roomList) {
    if(room->type_ != RoomType::plain) {

      //TODO This is not a good method to calculate the number of room cells
      //(the room may be irregularly shaped)
      const int ROOM_W = room->r_.p1.x - room->r_.p0.x + 1;
      const int ROOM_H = room->r_.p1.y - room->r_.p0.y + 1;
      const int NR_CELLS_IN_ROOM = ROOM_W * ROOM_H;

      const int MAX_NR_GROUPS_IN_ROOM = 2;
      for(int i = 0; i < MAX_NR_GROUPS_IN_ROOM; ++i) {
        //Randomly pick a free position inside the room
        vector<Pos> originBucket;
        for(int y = room->r_.p0.y; y <= room->r_.p1.y; ++y) {
          for(int x = room->r_.p0.x; x <= room->r_.p1.x; ++x) {
            if(Map::roomMap[x][y] == room && !blocked[x][y]) {
              originBucket.push_back(Pos(x, y));
            }
          }
        }

        //If room is too full (due to spawned monsters and features),
        //stop spawning in this room
        const int NR_ORIGIN_CANDIDATES = originBucket.size();
        if(NR_ORIGIN_CANDIDATES < (NR_CELLS_IN_ROOM / 3)) {break;}

        //Spawn monsters in room
        if(NR_ORIGIN_CANDIDATES > 0) {
          const int ELEMENT = Rnd::range(0, NR_ORIGIN_CANDIDATES - 1);
          const Pos& origin = originBucket.at(ELEMENT);
          vector<Pos> sortedFreeCellsVector;
          mkSortedFreeCellsVector(origin, blocked, sortedFreeCellsVector);

          if(mkGroupOfRandomNativeToRoomTypeAt(
                room->type_, sortedFreeCellsVector, blocked, false)) {
            nrGroupsSpawned++;
            if(nrGroupsSpawned >= NR_GROUPS_ALLOWED_ON_MAP) {return;}
          }
        }
      }

      //After attempting to populate a non-plain themed room,
      //mark that area as forbidden
      for(int y = room->r_.p0.y; y <= room->r_.p1.y; ++y) {
        for(int x = room->r_.p0.x; x <= room->r_.p1.x; ++x) {
          blocked[x][y] = true;
        }
      }
    }
  }

  //Second, place groups randomly in plain-themed areas until
  //no more groups to place
  while(nrGroupsSpawned < NR_GROUPS_ALLOWED_ON_MAP) {
    vector<Pos> originBucket;
    for(int y = 1; y < MAP_H - 1; ++y) {
      for(int x = 1; x < MAP_W - 1; ++x) {
        if(Map::roomMap[x][y]) {
          if(
            !blocked[x][y] &&
            Map::roomMap[x][y]->type_ == RoomType::plain) {
            originBucket.push_back(Pos(x, y));
          }
        }
      }
    }
    const int ELEMENT = Rnd::range(0, originBucket.size() - 1);
    const Pos origin  = originBucket.at(ELEMENT);
    vector<Pos> sortedFreeCellsVector;
    mkSortedFreeCellsVector(origin, blocked, sortedFreeCellsVector);
    if(mkGroupOfRandomNativeToRoomTypeAt(
          RoomType::plain, sortedFreeCellsVector, blocked, true)) {
      nrGroupsSpawned++;
    }
  }
}

void mkGroupAt(const ActorId id, const vector<Pos>& sortedFreeCellsVector,
               bool blocked[MAP_W][MAP_H], const bool IS_ROAMING_ALLOWED) {
  const ActorDataT& d = ActorData::data[int(id)];

  int maxNrInGroup = 1;

  switch(d.groupSize) {
    case monsterGroupSizeFew:    maxNrInGroup = Rnd::range(1, 2);    break;
    case monsterGroupSizeGroup:  maxNrInGroup = Rnd::range(3, 4);    break;
    case monsterGroupSizeHorde:  maxNrInGroup = Rnd::range(6, 7);    break;
    case monsterGroupSizeSwarm:  maxNrInGroup = Rnd::range(10, 12);  break;
    default: {} break;
  }

  Actor* originActor = nullptr;

  const int NR_FREE_CELLS = sortedFreeCellsVector.size();
  const int NR_CAN_BE_SPAWNED = min(NR_FREE_CELLS, maxNrInGroup);
  for(int i = 0; i < NR_CAN_BE_SPAWNED; ++i) {
    const Pos& pos = sortedFreeCellsVector.at(i);

    Actor* const actor = ActorFactory::mk(id, pos);
    Monster* const monster = static_cast<Monster*>(actor);
    monster->isRoamingAllowed_ = IS_ROAMING_ALLOWED;

    if(i == 0) {
      originActor = actor;
    } else {
      monster->leader = originActor;
    }

    blocked[pos.x][pos.y] = true;
  }
}

void mkSortedFreeCellsVector(const Pos& origin,
                             const bool blocked[MAP_W][MAP_H],
                             vector<Pos>& vectorRef) {
  vectorRef.resize(0);

  const int RADI = 10;
  const int X0 = getConstrInRange(1, origin.x - RADI, MAP_W - 2);
  const int Y0 = getConstrInRange(1, origin.y - RADI, MAP_H - 2);
  const int X1 = getConstrInRange(1, origin.x + RADI, MAP_W - 2);
  const int Y1 = getConstrInRange(1, origin.y + RADI, MAP_H - 2);

  for(int y = Y0; y <= Y1; ++y) {
    for(int x = X0; x <= X1; ++x) {
      if(!blocked[x][y]) {vectorRef.push_back(Pos(x, y));}
    }
  }

  IsCloserToPos sorter(origin);
  std::sort(vectorRef.begin(), vectorRef.end(), sorter);
}

} //PopulateMonsters
