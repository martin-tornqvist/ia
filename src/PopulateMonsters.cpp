#include "PopulateMonsters.h"

#include <algorithm>

#include "Engine.h"
#include "FeatureTrap.h"
#include "Map.h"
#include "ActorFactory.h"
#include "ActorMonster.h"
#include "ActorPlayer.h"
#include "FeatureFactory.h"
#include "MapGen.h"
#include "MapParsing.h"

using namespace std;

void PopulateMonsters::makeListOfMonstersEligibleForAutoSpawning(
  const int NR_LVLS_OUT_OF_DEPTH, vector<ActorId_t>& listToSet) const {

  listToSet.resize(0);

  const int DLVL = eng.map->getDlvl();
  const int EFFECTIVE_DLVL =
    max(1, min(LAST_CAVERN_LEVEL, DLVL + NR_LVLS_OUT_OF_DEPTH));

  for(unsigned int i = actor_player + 1; i < endOfActorIds; i++) {
    const ActorData& d = eng.actorDataHandler->dataList[i];
    if(
      d.isAutoSpawnAllowed &&
      d.nrLeftAllowedToSpawn != 0 &&
      EFFECTIVE_DLVL >= d.spawnMinDLVL &&
      EFFECTIVE_DLVL <= d.spawnMaxDLVL) {
      listToSet.push_back((ActorId_t)(i));
    }
  }
}

void PopulateMonsters::spawnGroupOfRandomAt(
  const vector<Pos>& sortedFreeCellsVector,
  bool blockers[MAP_X_CELLS][MAP_Y_CELLS],
  const int NR_LVLS_OUT_OF_DEPTH_ALLOWED,
  const bool IS_ROAMING_ALLOWED) const {

  vector<ActorId_t> idCandidates;
  makeListOfMonstersEligibleForAutoSpawning(
    NR_LVLS_OUT_OF_DEPTH_ALLOWED, idCandidates);

  if(idCandidates.empty() == false) {
    const ActorId_t id =
      idCandidates.at(eng.dice.range(0, idCandidates.size() - 1));

    spawnGroupAt(id, sortedFreeCellsVector, blockers,
                 IS_ROAMING_ALLOWED);
  }
}

void PopulateMonsters::trySpawnDueToTimePassed() const {
  trace << "PopulateMonsters::trySpawnDueToTimePassed()..." << endl;

  bool blockers[MAP_X_CELLS][MAP_Y_CELLS];
  MapParser::parse(CellPredBlocksBodyType(bodyType_normal, true, eng),
                   blockers);

  const int MIN_DIST_TO_PLAYER = FOV_STANDARD_RADI_INT + 3;

  const Pos& playerPos = eng.player->pos;
  const int X0 = max(0, playerPos.x - MIN_DIST_TO_PLAYER);
  const int Y0 = max(0, playerPos.y - MIN_DIST_TO_PLAYER);
  const int X1 = min(MAP_X_CELLS - 1, playerPos.x + MIN_DIST_TO_PLAYER);
  const int Y1 = min(MAP_Y_CELLS - 1, playerPos.y + MIN_DIST_TO_PLAYER);

  for(int x = X0; x <= X1; x++) {
    for(int y = Y0; y <= Y1; y++) {
      blockers[x][y] = true;
    }
  }

  vector<Pos> freeCellsVector;
  for(int y = 1; y < MAP_Y_CELLS - 1; y++) {
    for(int x = 1; x < MAP_X_CELLS - 1; x++) {
      if(blockers[x][y] == false) {
        freeCellsVector.push_back(Pos(x, y));
      }
    }
  }
  if(freeCellsVector.empty() == false) {

    const Pos& origin =
      freeCellsVector.at(eng.dice.range(0, freeCellsVector.size() - 1));

    makeSortedFreeCellsVector(origin, blockers, freeCellsVector);

    if(freeCellsVector.empty() == false) {
      const Pos& pos =
        freeCellsVector.at(eng.dice.range(0, freeCellsVector.size() - 1));

      if(eng.map->cells[pos.x][pos.y].isExplored) {
        spawnGroupOfRandomAt(freeCellsVector, blockers,
                             getRandomOutOfDepth(), true);
      }
    }
  }
  trace << "PopulateMonsters::trySpawnDueToTimePassed() [DONE]" << endl;
}

void PopulateMonsters::populateCaveLevel() const {
  const int NR_GROUPS_ALLOWED = eng.dice.range(6, 7);

  bool blockers[MAP_X_CELLS][MAP_Y_CELLS];

  const int MIN_DIST_FROM_PLAYER = FOV_STANDARD_RADI_INT - 2;
  MapParser::parse(CellPredBlocksBodyType(bodyType_normal, true, eng),
                   blockers);

  const Pos& playerPos = eng.player->pos;

  const int X0 = max(0, playerPos.x - MIN_DIST_FROM_PLAYER);
  const int Y0 = max(0, playerPos.y - MIN_DIST_FROM_PLAYER);
  const int X1 = min(MAP_X_CELLS - 1, playerPos.x + MIN_DIST_FROM_PLAYER) - 1;
  const int Y1 = min(MAP_Y_CELLS - 1, playerPos.y + MIN_DIST_FROM_PLAYER) - 1;

  for(int y = Y0; y <= Y1; y++) {
    for(int x = X0; x <= X1; x++) {
      blockers[x][y] = true;
    }
  }

  for(int i = 0; i < NR_GROUPS_ALLOWED; i++) {
    vector<Pos> originCandidates;
    for(int y = 1; y < MAP_Y_CELLS - 1; y++) {
      for(int x = 1; x < MAP_X_CELLS - 1; x++) {
        if(blockers[x][y] == false) {
          originCandidates.push_back(Pos(x, y));
        }
      }
    }
    const int ELEMENT = eng.dice.range(0, originCandidates.size() - 1);
    const Pos origin = originCandidates.at(ELEMENT);
    vector<Pos> sortedFreeCellsVector;
    makeSortedFreeCellsVector(origin, blockers, sortedFreeCellsVector);
    if(sortedFreeCellsVector.empty() == false) {
      spawnGroupOfRandomAt(sortedFreeCellsVector, blockers,
                           getRandomOutOfDepth(), true);
    }
  }
}

void PopulateMonsters::populateIntroLevel() {
  const int NR_GROUPS_ALLOWED = 2; //eng.dice.range(2, 3);

  bool blockers[MAP_X_CELLS][MAP_Y_CELLS];

  const int MIN_DIST_FROM_PLAYER = FOV_STANDARD_RADI_INT + 3;
  MapParser::parse(CellPredBlocksBodyType(bodyType_normal, true, eng),
                   blockers);

  const Pos& playerPos = eng.player->pos;

  const int X0 = max(0, playerPos.x - MIN_DIST_FROM_PLAYER);
  const int Y0 = max(0, playerPos.y - MIN_DIST_FROM_PLAYER);
  const int X1 = min(MAP_X_CELLS - 1, playerPos.x + MIN_DIST_FROM_PLAYER) - 1;
  const int Y1 = min(MAP_Y_CELLS - 1, playerPos.y + MIN_DIST_FROM_PLAYER) - 1;
  for(int y = Y0; y <= Y1; y++) {
    for(int x = X0; x <= X1; x++) {
      blockers[x][y] = true;
    }
  }

  for(int i = 0; i < NR_GROUPS_ALLOWED; i++) {
    vector<Pos> originCandidates;
    for(int y = 1; y < MAP_Y_CELLS - 1; y++) {
      for(int x = 1; x < MAP_X_CELLS - 1; x++) {
        if(blockers[x][y] == false) {
          originCandidates.push_back(Pos(x, y));
        }
      }
    }
    const int ELEMENT = eng.dice.range(0, originCandidates.size() - 1);
    const Pos origin = originCandidates.at(ELEMENT);
    vector<Pos> sortedFreeCellsVector;
    makeSortedFreeCellsVector(origin, blockers, sortedFreeCellsVector);
    if(sortedFreeCellsVector.empty() == false) {
      spawnGroupAt(actor_wolf, sortedFreeCellsVector, blockers, true);
    }
  }
}

void PopulateMonsters::populateRoomAndCorridorLevel(
  RoomTheme_t themeMap[MAP_X_CELLS][MAP_Y_CELLS],
  const vector<Room*>& rooms) const {

  const int NR_GROUPS_ALLOWED = eng.dice.range(5, 9);
  int nrGroupsSpawned = 0;

  bool blockers[MAP_X_CELLS][MAP_Y_CELLS];

  const int MIN_DIST_FROM_PLAYER = FOV_STANDARD_RADI_INT - 2;

  MapParser::parse(CellPredBlocksBodyType(bodyType_normal, true, eng),
                   blockers);

  const Pos& playerPos = eng.player->pos;

  const int X0 = max(0, playerPos.x - MIN_DIST_FROM_PLAYER);
  const int Y0 = max(0, playerPos.y - MIN_DIST_FROM_PLAYER);
  const int X1 = min(MAP_X_CELLS - 1, playerPos.x + MIN_DIST_FROM_PLAYER) - 1;
  const int Y1 = min(MAP_Y_CELLS - 1, playerPos.y + MIN_DIST_FROM_PLAYER) - 1;
  for(int y = Y0; y <= Y1; y++) {
    for(int x = X0; x <= X1; x++) {
      blockers[x][y] = true;
    }
  }

  //First, attempt to populate all non-plain themed rooms
  for(unsigned int i = 0; i < rooms.size(); i++) {
    Room* const room = rooms.at(i);
    if(room->roomTheme != roomTheme_plain) {

      const unsigned int MAX_NR_GROUPS_IN_ROOM = 2;
      for(
        unsigned int i_groupsInRoom = 0;
        i_groupsInRoom < MAX_NR_GROUPS_IN_ROOM;
        i_groupsInRoom++) {

        //Randomly pick a free position inside the room
        vector<Pos> originCandidates;
        for(int y = room->getY0(); y <= room->getY1(); y++) {
          for(int x = room->getX0(); x <= room->getX1(); x++) {
            if(
              blockers[x][y] == false && themeMap[x][y] ==
              room->roomTheme) {
              originCandidates.push_back(Pos(x, y));
            }
          }
        }

        //If room is too full (due to spawned monsters and features),
        //stop spawning in this room
        const int ROOM_W = room->getX1() - room->getX0() + 1;
        const int ROOM_H = room->getY1() - room->getY0() + 1;
        const unsigned int NR_CELLS_IN_ROOM = ROOM_W * ROOM_H;

        if(originCandidates.size() < NR_CELLS_IN_ROOM / 3) {
          continue;
        }

        //Spawn monsters in room
        if(originCandidates.empty() == false) {
          const int ELEMENT =
            eng.dice.range(0, originCandidates.size() - 1);
          const Pos& origin = originCandidates.at(ELEMENT);
          vector<Pos> sortedFreeCellsVector;
          makeSortedFreeCellsVector(
            origin, blockers, sortedFreeCellsVector);

          if(spawnGroupOfRandomNativeToRoomThemeAt(
                room->roomTheme, sortedFreeCellsVector,
                blockers, false)) {
            nrGroupsSpawned++;
            if(nrGroupsSpawned >= NR_GROUPS_ALLOWED) {
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
  while(nrGroupsSpawned < NR_GROUPS_ALLOWED) {
    vector<Pos> originCandidates;
    for(int y = 1; y < MAP_Y_CELLS - 1; y++) {
      for(int x = 1; x < MAP_X_CELLS - 1; x++) {
        if(
          blockers[x][y] == false && themeMap[x][y] ==
          roomTheme_plain) {
          originCandidates.push_back(Pos(x, y));
        }
      }
    }
    const int ELEMENT = eng.dice.range(0, originCandidates.size() - 1);
    const Pos origin = originCandidates.at(ELEMENT);
    vector<Pos> sortedFreeCellsVector;
    makeSortedFreeCellsVector(origin, blockers, sortedFreeCellsVector);
    if(spawnGroupOfRandomNativeToRoomThemeAt(
          roomTheme_plain, sortedFreeCellsVector,
          blockers, true)) {
      nrGroupsSpawned++;
    }
  }
}

bool PopulateMonsters::spawnGroupOfRandomNativeToRoomThemeAt(
  const RoomTheme_t roomTheme, const vector<Pos>& sortedFreeCellsVector,
  bool blockers[MAP_X_CELLS][MAP_Y_CELLS],
  const bool IS_ROAMING_ALLOWED) const {

  trace << "PopulateMonsters::spawnGroupOfRandomNativeToRoomThemeAt()" << endl;
  const int NR_LEVELS_OUT_OF_DEPTH_ALLOWED = getRandomOutOfDepth();
  vector<ActorId_t> idCandidates;
  makeListOfMonstersEligibleForAutoSpawning(
    NR_LEVELS_OUT_OF_DEPTH_ALLOWED, idCandidates);

  for(unsigned int i = 0; i < idCandidates.size(); i++) {
    const ActorData& d = eng.actorDataHandler->dataList[idCandidates.at(i)];
    bool monsterIsNativeToRoom = false;
    for(
      unsigned int i_nativeRooms = 0;
      i_nativeRooms < d.nativeRooms.size();
      i_nativeRooms++) {
      if(d.nativeRooms.at(i_nativeRooms) == roomTheme) {
        monsterIsNativeToRoom = true;
        break;
      }
    }
    if(monsterIsNativeToRoom == false) {
      idCandidates.erase(idCandidates.begin() + i);
      i--;
    }
  }

  if(idCandidates.empty()) {
    trace << "PopulateMonsters: Found no valid monsters to spawn ";
    trace << "at room theme (" + toString(roomTheme) + ")" << endl;
    return false;
  } else {
    const int ELEMENT = eng.dice.range(0, idCandidates.size() - 1);
    const ActorId_t id = idCandidates.at(ELEMENT);
    spawnGroupAt(id, sortedFreeCellsVector, blockers,
                 IS_ROAMING_ALLOWED);
    return true;
  }
}

void PopulateMonsters::spawnGroupAt(
  const ActorId_t id, const vector<Pos>& sortedFreeCellsVector,
  bool blockers[MAP_X_CELLS][MAP_Y_CELLS],
  const bool IS_ROAMING_ALLOWED) const {

  const ActorData& d = eng.actorDataHandler->dataList[id];

  int maxNrInGroup = 1;

  switch(d.groupSize) {
    case monsterGroupSize_few:    {maxNrInGroup = eng.dice.range(1, 2);}    break;
    case monsterGroupSize_group:  {maxNrInGroup = eng.dice.range(3, 4);}    break;
    case monsterGroupSize_horde:  {maxNrInGroup = eng.dice.range(6, 7);}    break;
    case monsterGroupSize_swarm:  {maxNrInGroup = eng.dice.range(12, 15);}  break;
    default: {} break;
  }

  Actor* originActor = NULL;

  const int NR_FREE_CELLS = sortedFreeCellsVector.size();
  const int NR_CAN_BE_SPAWNED = min(NR_FREE_CELLS, maxNrInGroup);
  for(int i = 0; i < NR_CAN_BE_SPAWNED; i++) {
    const Pos& pos = sortedFreeCellsVector.at((unsigned int)i);

    Actor* const actor = eng.actorFactory->spawnActor(id, pos);
    Monster* const monster = dynamic_cast<Monster*>(actor);
    monster->isRoamingAllowed = IS_ROAMING_ALLOWED;

    if(i == 0) {
      originActor = actor;
    } else {
      monster->leader = originActor;
    }

    blockers[pos.x][pos.y] = true;
  }
}

void PopulateMonsters::makeSortedFreeCellsVector(
  const Pos& origin, const bool blockers[MAP_X_CELLS][MAP_Y_CELLS],
  vector<Pos>& vectorToSet) const {

  vectorToSet.resize(0);

  const int RADI = 10;
  const int X0 = getConstrInRange(1, origin.x - RADI, MAP_X_CELLS - 2);
  const int Y0 = getConstrInRange(1, origin.y - RADI, MAP_Y_CELLS - 2);
  const int X1 = getConstrInRange(1, origin.x + RADI, MAP_X_CELLS - 2);
  const int Y1 = getConstrInRange(1, origin.y + RADI, MAP_Y_CELLS - 2);

  for(int y = Y0; y <= Y1; y++) {
    for(int x = X0; x <= X1; x++) {
      if(blockers[x][y] == false) {
        vectorToSet.push_back(Pos(x, y));
      }
    }
  }

  IsCloserToOrigin sorter(origin, eng);
  std::sort(vectorToSet.begin(), vectorToSet.end(), sorter);
}

int PopulateMonsters::getRandomOutOfDepth() const {
  const int DLVL = eng.map->getDlvl();
  if(DLVL == 0) {
    return 0;
  }

  const int RND = eng.dice.range(1, 100);

  if(RND <= 8 && DLVL > 1) {
    return 6;
  }
  if(RND <= 25) {
    return 3;
  }

  return 0;
}

//bool PopulateMonsters::spawnGroupOfMonstersAtFreeCells(vector<Pos>& freeCells, const bool ALLOW_ROAM /*,
//    const SpecialRoom_t belongingToSpecialRoomType */) const {
//  if(freeCells.size() > 0) {
//    trace << "PopulateMonsters::spawnGroupOfMonstersAtFreeCells()" << endl;
//
//    const int FREE_CELLS_ELEMENT = eng.dice(1, freeCells.size()) - 1;
//    const Pos pos(freeCells.at(FREE_CELLS_ELEMENT));
//
//    const int OUT_OF_DEPTH_OFFSET = getOutOfDepthOffset();
//
//    Actor* originActor = NULL;
//    originActor = eng.actorFactory->spawnRandomActor(pos, OUT_OF_DEPTH_OFFSET);
//
//    if(originActor != NULL) {
//      trace << "PopulateMonsters: Valid actor found, spawning group of monsters" << endl;
//
//      dynamic_cast<Monster*>(originActor)->isRoamingAllowed = ALLOW_ROAM;
//
//      freeCells.erase(freeCells.begin() + FREE_CELLS_ELEMENT);
//
//      int extraSpawns = 0;
//
//      const ActorData* const d = originActor->getData();
//      switch(d->groupSize) {
//      case monsterGroupSize_few: {
//        extraSpawns = eng.dice.range(0, 2);
//      }
//      break;
//      case monsterGroupSize_group: {
//        extraSpawns = eng.dice.range(2, 5);
//      }
//      break;
//      case monsterGroupSize_horde: {
//        extraSpawns = eng.dice.range(7, 13);
//      }
//      break;
//      default:
//      {} break;
//      }
//
//      IsCloserToOrigin isCloserToOrigin(pos, eng);
//      sort(freeCells.begin(), freeCells.end(), isCloserToOrigin);
//
//      for(int i = 0; i < extraSpawns; i++) {
//        if(freeCells.size() > 0) {
//          const Pos posExtra = freeCells.front();
//          Actor* const actorExtra = eng.actorFactory->spawnActor(originActor->getData()->actorId, posExtra);
//          dynamic_cast<Monster*>(actorExtra)->isRoamingAllowed = ALLOW_ROAM;
//          dynamic_cast<Monster*>(actorExtra)->leader = originActor;
//          freeCells.erase(freeCells.begin());
//        }
//      }
//      return true;
//    }
//  }
//  trace << "PopulateMonsters: Spawning group of monster failed (probably no monster met criteria)" << endl;
//  return false;
//}

//void PopulateMonsters::populate() const {
//  trace << "PopulateMonsters::PopulateMonsters()..." << endl;
//  bool blockers[MAP_X_CELLS][MAP_Y_CELLS];
//  eng.mapTests->makeMoveBlockerArrayForBodyType(bodyType_normal, blockers);
//  eng.basicUtils->reverseBoolArray(blockers);
//  vector<Pos> freeCells;
//  eng.mapTests->makeBoolVectorFromMapArray(blockers, freeCells);
//
//  const int MIN_DIST_FROM_PLAYER = eng.map->getDlvl() == 0 ? FOV_STANDARD_RADI_INT + 1 : FOV_STANDARD_RADI_INT - 1;
//
//  unsigned int i = 0;
//  bool increaseIndex = true;
//  while(i != freeCells.size()) {
//    increaseIndex = true;
//    if(eng.basicUtils->chebyshevDist(freeCells.at(i), eng.player->pos) < MIN_DIST_FROM_PLAYER) {
//      freeCells.erase(freeCells.begin() + i);
//      increaseIndex = false;
//    }
//
//    i += increaseIndex ? 1 : 0;
//  }
//
//  if(eng.map->getDlvl() >= 1 && eng.map->getDlvl() < FIRST_CAVERN_LEVEL) {
//    bool blockersTraps[MAP_X_CELLS][MAP_Y_CELLS];
//    eng.mapTests->makeItemBlockerArray(blockersTraps);
//    eng.basicUtils->reverseBoolArray(blockersTraps);
//    vector<Pos> freeCellsTraps;
//    eng.mapTests->makeBoolVectorFromMapArray(blockersTraps, freeCellsTraps);
//
//    const int MAX_CELLS_PER_TRAP = 210;
//    int nrOfTrapsOnMap = int(freeCellsTraps.size()) / MAX_CELLS_PER_TRAP;
//    nrOfTrapsOnMap = max(2, nrOfTrapsOnMap);
//    nrOfTrapsOnMap += eng.dice.range(0, nrOfTrapsOnMap / 4);
//
//    int n = 0;
//    for(int ii = 0; ii < nrOfTrapsOnMap; ii++) {
//      if(freeCellsTraps.size() > 0) {
//        n = eng.dice(1, freeCellsTraps.size()) - 1;
//        const Pos pos(freeCellsTraps.at(n));
//
//        if(eng.map->featuresStatic[pos.x][pos.y]->canHaveStaticFeature()) {
//          const FeatureDef* const defAtTrap = eng.featureData->getFeatureDef(eng.map->featuresStatic[pos.x][pos.y]->getId());
//          eng.featureFactory->spawnFeatureAt(feature_trap, pos, new TrapSpawnData(defAtTrap, trap_any));
//          freeCellsTraps.erase(freeCellsTraps.begin() + n);
//        }
//      }
//    }
//  }
//
//  const int CELLS_PER_MONSTER_GROUP = eng.map->getDlvl() == 0 ? 500 : 110;
//
//  trace << "PopulateMonsters: Cells per monster group: " << CELLS_PER_MONSTER_GROUP << endl;
//
//  int nrMonsterGroupsOnMap = freeCells.size() / CELLS_PER_MONSTER_GROUP;
//
//  nrMonsterGroupsOnMap = max(1, nrMonsterGroupsOnMap);
//  nrMonsterGroupsOnMap += eng.dice.range(0, nrMonsterGroupsOnMap / 4);
//
//  trace << "PopulateMonsters: Number of monster groups on map: " << nrMonsterGroupsOnMap << endl;
//
//  //Spawn monsters randomly from the Pos-vector
//  for(int ii = 0; ii < nrMonsterGroupsOnMap; ii++) {
//    ii -= spawnGroupOfMonstersAtFreeCells(freeCells, true) == false ? 1 : 0;
//  }
//  trace << "PopulateMonsters::PopulateMonsters() [DONE]" << endl;
//}

//void PopulateMonsters::spawnOneMonster() const {
//  bool blockers[MAP_X_CELLS][MAP_Y_CELLS];
//  eng.mapTests->makeMoveBlockerArrayForBodyType(bodyType_normal, blockers);
//  eng.basicUtils->reverseBoolArray(blockers);
//  vector<Pos> freeCells;
//  eng.mapTests->makeBoolVectorFromMapArray(blockers, freeCells);
//
//  const int MIN_DIST_FROM_PLAYER = FOV_STANDARD_RADI_INT + 4;
//
//  if(freeCells.size() > 0) {
//    bool done = false;
//    while(done == false) {
//
//      if(freeCells.size() == 0) {
//        done = true;
//        break;
//      }
//
//      const unsigned int FREE_CELLS_ELEMENT = eng.dice(1, freeCells.size()) - 1;
//
//      if(eng.map->explored[freeCells.at(FREE_CELLS_ELEMENT).x][freeCells.at(FREE_CELLS_ELEMENT).y] == false) {
//        done = true;
//        break;
//      }
//
//      const Pos dif = freeCells.at(FREE_CELLS_ELEMENT) - eng.player->pos;
//
//      if((dif.x >= MIN_DIST_FROM_PLAYER || dif.x <= -MIN_DIST_FROM_PLAYER) || (dif.y >= MIN_DIST_FROM_PLAYER || dif.y <= -MIN_DIST_FROM_PLAYER)) {
//        const int OUT_OF_DEPTH_OFFSET = getOutOfDepthOffset();
//        eng.actorFactory->spawnRandomActor(freeCells.at(FREE_CELLS_ELEMENT), OUT_OF_DEPTH_OFFSET);
//        done = true;
//      } else {
//        freeCells.erase(freeCells.begin() + FREE_CELLS_ELEMENT);
//      }
//    }
//  }
//}
