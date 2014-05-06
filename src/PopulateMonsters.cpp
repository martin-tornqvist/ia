#include "PopulateMonsters.h"

#include <algorithm>

#include "Init.h"
#include "FeatureTrap.h"
#include "Map.h"
#include "ActorFactory.h"
#include "ActorMonster.h"
#include "ActorPlayer.h"
#include "FeatureFactory.h"
#include "MapGen.h"
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

void makeListOfMonstersCanAutoSpawn(const int NR_LVLS_OUT_OF_DEPTH,
                                    vector<ActorId>& listRef) {
  listRef.resize(0);

  const int EFFECTIVE_DLVL =
    max(1, min(LAST_CAVERN_LEVEL, Map::dlvl + NR_LVLS_OUT_OF_DEPTH));

  for(unsigned int i = actor_player + 1; i < endOfActorIds; i++) {
    const ActorDataT& d = ActorData::data[i];
    if(
      d.isAutoSpawnAllowed &&
      d.nrLeftAllowedToSpawn != 0 &&
      EFFECTIVE_DLVL >= d.spawnMinDLVL &&
      EFFECTIVE_DLVL <= d.spawnMaxDLVL) {
      listRef.push_back((ActorId)(i));
    }
  }
}

void spawnGroupOfRandomAt(const vector<Pos>& sortedFreeCellsVector,
                          bool blockers[MAP_W][MAP_H],
                          const int NR_LVLS_OUT_OF_DEPTH_ALLOWED,
                          const bool IS_ROAMING_ALLOWED) {
  vector<ActorId> idBucket;
  makeListOfMonstersCanAutoSpawn(NR_LVLS_OUT_OF_DEPTH_ALLOWED, idBucket);

  if(idBucket.empty() == false) {
    const ActorId id = idBucket.at(Rnd::range(0, idBucket.size() - 1));
    spawnGroupAt(id, sortedFreeCellsVector, blockers, IS_ROAMING_ALLOWED);
  }
}

bool spawnGroupOfRandomNativeToRoomThemeAt(
  const RoomThemeId roomTheme, const vector<Pos>& sortedFreeCellsVector,
  bool blockers[MAP_W][MAP_H], const bool IS_ROAMING_ALLOWED) {

  trace << "PopulateMonsters::spawnGroupOfRandomNativeToRoomThemeAt()" << endl;
  const int NR_LEVELS_OUT_OF_DEPTH_ALLOWED = getRandomOutOfDepth();
  vector<ActorId> idBucket;
  makeListOfMonstersCanAutoSpawn(NR_LEVELS_OUT_OF_DEPTH_ALLOWED, idBucket);

  for(size_t i = 0; i < idBucket.size(); i++) {
    const ActorDataT& d = ActorData::data[idBucket.at(i)];
    bool isMonsterNativeToRoom = false;
    for(size_t iNative = 0; iNative < d.nativeRooms.size(); iNative++) {
      if(d.nativeRooms.at(iNative) == roomTheme) {
        isMonsterNativeToRoom = true;
        break;
      }
    }
    if(isMonsterNativeToRoom == false) {
      idBucket.erase(idBucket.begin() + i);
      i--;
    }
  }

  if(idBucket.empty()) {
    trace << "PopulateMonsters: Found no valid monsters to spawn "
          "at room theme (" + toStr(int(roomTheme)) + ")" << endl;
    return false;
  } else {
    const int ELEMENT = Rnd::range(0, idBucket.size() - 1);
    const ActorId id = idBucket.at(ELEMENT);
    spawnGroupAt(id, sortedFreeCellsVector, blockers,
                 IS_ROAMING_ALLOWED);
    return true;
  }
}

} //namespace

void trySpawnDueToTimePassed() {
  trace << "PopulateMonsters::trySpawnDueToTimePassed()..." << endl;

  bool blockers[MAP_W][MAP_H];
  MapParse::parse(CellPred::BlocksMoveCmn(true), blockers);

  const int MIN_DIST_TO_PLAYER = FOV_STD_RADI_INT + 3;

  const Pos& playerPos = Map::player->pos;
  const int X0 = max(0, playerPos.x - MIN_DIST_TO_PLAYER);
  const int Y0 = max(0, playerPos.y - MIN_DIST_TO_PLAYER);
  const int X1 = min(MAP_W - 1, playerPos.x + MIN_DIST_TO_PLAYER);
  const int Y1 = min(MAP_H - 1, playerPos.y + MIN_DIST_TO_PLAYER);

  for(int x = X0; x <= X1; x++) {
    for(int y = Y0; y <= Y1; y++) {
      blockers[x][y] = true;
    }
  }

  vector<Pos> freeCellsVector;
  for(int y = 1; y < MAP_H - 1; y++) {
    for(int x = 1; x < MAP_W - 1; x++) {
      if(blockers[x][y] == false) {
        freeCellsVector.push_back(Pos(x, y));
      }
    }
  }

  if(freeCellsVector.empty() == false) {

    const int ELEMENT = Rnd::range(0, freeCellsVector.size() - 1);
    const Pos& origin = freeCellsVector.at(ELEMENT);

    makeSortedFreeCellsVector(origin, blockers, freeCellsVector);

    if(freeCellsVector.empty() == false) {
      if(Map::cells[origin.x][origin.y].isExplored) {
        const int NR_OOD = getRandomOutOfDepth();
        spawnGroupOfRandomAt(freeCellsVector, blockers, NR_OOD, true);
      }
    }
  }
  trace << "PopulateMonsters::trySpawnDueToTimePassed() [DONE]" << endl;
}

void populateCaveLevel() {
  const int NR_GROUPS_ALLOWED = Rnd::range(6, 7);

  bool blockers[MAP_W][MAP_H];

  const int MIN_DIST_FROM_PLAYER = FOV_STD_RADI_INT - 2;
  MapParse::parse(CellPred::BlocksMoveCmn(true), blockers);

  const Pos& playerPos = Map::player->pos;

  const int X0 = max(0, playerPos.x - MIN_DIST_FROM_PLAYER);
  const int Y0 = max(0, playerPos.y - MIN_DIST_FROM_PLAYER);
  const int X1 = min(MAP_W - 1, playerPos.x + MIN_DIST_FROM_PLAYER) - 1;
  const int Y1 = min(MAP_H - 1, playerPos.y + MIN_DIST_FROM_PLAYER) - 1;

  for(int y = Y0; y <= Y1; y++) {
    for(int x = X0; x <= X1; x++) {
      blockers[x][y] = true;
    }
  }

  for(int i = 0; i < NR_GROUPS_ALLOWED; i++) {
    vector<Pos> originBucket;
    for(int y = 1; y < MAP_H - 1; y++) {
      for(int x = 1; x < MAP_W - 1; x++) {
        if(blockers[x][y] == false) {
          originBucket.push_back(Pos(x, y));
        }
      }
    }
    const int ELEMENT = Rnd::range(0, originBucket.size() - 1);
    const Pos origin = originBucket.at(ELEMENT);
    vector<Pos> sortedFreeCellsVector;
    makeSortedFreeCellsVector(origin, blockers, sortedFreeCellsVector);
    if(sortedFreeCellsVector.empty() == false) {
      spawnGroupOfRandomAt(sortedFreeCellsVector, blockers,
                           getRandomOutOfDepth(), true);
    }
  }
}

void populateIntroLevel() {
  const int NR_GROUPS_ALLOWED = 2; //Rnd::range(2, 3);

  bool blockers[MAP_W][MAP_H];

  const int MIN_DIST_FROM_PLAYER = FOV_STD_RADI_INT + 3;
  MapParse::parse(CellPred::BlocksMoveCmn(true), blockers);

  const Pos& playerPos = Map::player->pos;

  const int X0 = max(0, playerPos.x - MIN_DIST_FROM_PLAYER);
  const int Y0 = max(0, playerPos.y - MIN_DIST_FROM_PLAYER);
  const int X1 = min(MAP_W - 1, playerPos.x + MIN_DIST_FROM_PLAYER) - 1;
  const int Y1 = min(MAP_H - 1, playerPos.y + MIN_DIST_FROM_PLAYER) - 1;
  for(int y = Y0; y <= Y1; y++) {
    for(int x = X0; x <= X1; x++) {
      blockers[x][y] = true;
    }
  }

  for(int i = 0; i < NR_GROUPS_ALLOWED; i++) {
    vector<Pos> originBucket;
    for(int y = 1; y < MAP_H - 1; y++) {
      for(int x = 1; x < MAP_W - 1; x++) {
        if(blockers[x][y] == false) {originBucket.push_back(Pos(x, y));}
      }
    }
    const int ELEMENT = Rnd::range(0, originBucket.size() - 1);
    const Pos origin = originBucket.at(ELEMENT);
    vector<Pos> sortedFreeCellsVector;
    makeSortedFreeCellsVector(origin, blockers, sortedFreeCellsVector);
    if(sortedFreeCellsVector.empty() == false) {
      spawnGroupAt(actor_wolf, sortedFreeCellsVector, blockers, true);
    }
  }
}

void populateRoomAndCorridorLevel() {
  const int NR_GROUPS_ALLOWED_ON_MAP = Rnd::range(5, 9);
  int nrGroupsSpawned = 0;

  bool blockers[MAP_W][MAP_H];

  const int MIN_DIST_FROM_PLAYER = FOV_STD_RADI_INT - 1;

  MapParse::parse(CellPred::BlocksMoveCmn(true), blockers);

  const Pos& playerPos = Map::player->pos;

  const int X0 = max(0, playerPos.x - MIN_DIST_FROM_PLAYER);
  const int Y0 = max(0, playerPos.y - MIN_DIST_FROM_PLAYER);
  const int X1 = min(MAP_W - 1, playerPos.x + MIN_DIST_FROM_PLAYER);
  const int Y1 = min(MAP_H - 1, playerPos.y + MIN_DIST_FROM_PLAYER);
  for(int y = Y0; y <= Y1; y++) {
    for(int x = X0; x <= X1; x++) {
      blockers[x][y] = true;
    }
  }

  //First, attempt to populate all non-plain themed rooms
  for(Room * const room : Map::rooms) {
    if(room->roomTheme != RoomThemeId::plain) {

      const int ROOM_W = room->getX1() - room->getX0() + 1;
      const int ROOM_H = room->getY1() - room->getY0() + 1;
      const int NR_CELLS_IN_ROOM = ROOM_W * ROOM_H;

      const int MAX_NR_GROUPS_IN_ROOM = 2;
      for(int i = 0; i < MAX_NR_GROUPS_IN_ROOM; i++) {
        //Randomly pick a free position inside the room
        vector<Pos> originBucket;
        for(int y = room->getY0(); y <= room->getY1(); y++) {
          for(int x = room->getX0(); x <= room->getX1(); x++) {
            if(
              blockers[x][y] == false &&
              RoomThemeMaking::themeMap[x][y] == room->roomTheme) {
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
          makeSortedFreeCellsVector(origin, blockers, sortedFreeCellsVector);

          if(spawnGroupOfRandomNativeToRoomThemeAt(
                room->roomTheme, sortedFreeCellsVector, blockers, false)) {
            nrGroupsSpawned++;
            if(nrGroupsSpawned >= NR_GROUPS_ALLOWED_ON_MAP) {
              return;
            }
          }
        }
      }

      //After attempting to populate a non-plain themed room,
      //mark that area as forbidden
      for(int y = room->getY0(); y <= room->getY1(); y++) {
        for(int x = room->getX0(); x <= room->getX1(); x++) {
          blockers[x][y] = true;
        }
      }
    }
  }

  //Second, place groups randomly in plain-themed areas until
  //no more groups to place
  while(nrGroupsSpawned < NR_GROUPS_ALLOWED_ON_MAP) {
    vector<Pos> originBucket;
    for(int y = 1; y < MAP_H - 1; y++) {
      for(int x = 1; x < MAP_W - 1; x++) {
        if(
          blockers[x][y] == false &&
          RoomThemeMaking::themeMap[x][y] == RoomThemeId::plain) {
          originBucket.push_back(Pos(x, y));
        }
      }
    }
    const int ELEMENT = Rnd::range(0, originBucket.size() - 1);
    const Pos origin  = originBucket.at(ELEMENT);
    vector<Pos> sortedFreeCellsVector;
    makeSortedFreeCellsVector(origin, blockers, sortedFreeCellsVector);
    if(spawnGroupOfRandomNativeToRoomThemeAt(
          RoomThemeId::plain, sortedFreeCellsVector, blockers, true)) {
      nrGroupsSpawned++;
    }
  }
}

void spawnGroupAt(const ActorId id, const vector<Pos>& sortedFreeCellsVector,
                  bool blockers[MAP_W][MAP_H], const bool IS_ROAMING_ALLOWED) {
  const ActorDataT& d = ActorData::data[id];

  int maxNrInGroup = 1;

  switch(d.groupSize) {
    case monsterGroupSizeFew:    maxNrInGroup = Rnd::range(1, 2);    break;
    case monsterGroupSizeGroup:  maxNrInGroup = Rnd::range(3, 4);    break;
    case monsterGroupSizeHorde:  maxNrInGroup = Rnd::range(6, 7);    break;
    case monsterGroupSizeSwarm:  maxNrInGroup = Rnd::range(10, 12);  break;
    default: {} break;
  }

  Actor* originActor = NULL;

  const int NR_FREE_CELLS = sortedFreeCellsVector.size();
  const int NR_CAN_BE_SPAWNED = min(NR_FREE_CELLS, maxNrInGroup);
  for(int i = 0; i < NR_CAN_BE_SPAWNED; i++) {
    const Pos& pos = sortedFreeCellsVector.at(i);

    Actor* const actor = ActorFactory::spawn(id, pos);
    Monster* const monster = dynamic_cast<Monster*>(actor);
    monster->isRoamingAllowed_ = IS_ROAMING_ALLOWED;

    if(i == 0) {
      originActor = actor;
    } else {
      monster->leader = originActor;
    }

    blockers[pos.x][pos.y] = true;
  }
}

void makeSortedFreeCellsVector(const Pos& origin,
                               const bool blockers[MAP_W][MAP_H],
                               vector<Pos>& vectorRef) {
  vectorRef.resize(0);

  const int RADI = 10;
  const int X0 = getConstrInRange(1, origin.x - RADI, MAP_W - 2);
  const int Y0 = getConstrInRange(1, origin.y - RADI, MAP_H - 2);
  const int X1 = getConstrInRange(1, origin.x + RADI, MAP_W - 2);
  const int Y1 = getConstrInRange(1, origin.y + RADI, MAP_H - 2);

  for(int y = Y0; y <= Y1; y++) {
    for(int x = X0; x <= X1; x++) {
      if(blockers[x][y] == false) {
        vectorRef.push_back(Pos(x, y));
      }
    }
  }

  IsCloserToOrigin sorter(origin);
  std::sort(vectorRef.begin(), vectorRef.end(), sorter);
}

} //PopulateMonsters
