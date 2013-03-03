#include "PopulateMonsters.h"

#include <algorithm>

#include "Engine.h"
#include "FeatureTrap.h"
#include "Map.h"
#include "ActorFactory.h"
#include "ActorMonster.h"
#include "ActorPlayer.h"
#include "FeatureFactory.h"
#include "MapBuildBSP.h"

using namespace std;

void PopulateMonsters::makeListOfMonstersEligibleForAutoSpawning(const int NR_LVLS_OUT_OF_DEPTH_ALLOWED,
    vector<ActorDevNames_t>& listToFill) const {

  listToFill.resize(0);

  const int DLVL = eng->map->getDungeonLevel();

  for(unsigned int i = 1; i < endOfActorDevNames; i++) {
    const ActorDefinition& d = eng->actorData->actorDefinitions[i];
    if(
      d.isAutoSpawnAllowed &&
      (d.nrLeftAllowedToSpawn == -1 || d.nrLeftAllowedToSpawn > 0) &&
      DLVL >= (d.spawnMinLevel - NR_LVLS_OUT_OF_DEPTH_ALLOWED) &&
      DLVL <= d.spawnMaxLevel) {
      listToFill.push_back(static_cast<ActorDevNames_t>(i));
    }
  }
}

void PopulateMonsters::spawnGroupOfRandomAt(const vector<coord>& sortedFreeCellsVector, bool forbiddenCells[MAP_X_CELLS][MAP_Y_CELLS],
    const int NR_LVLS_OUT_OF_DEPTH_ALLOWED, const bool IS_ROAMING_ALLOWED) const {
  vector<ActorDevNames_t> idCandidates;
  makeListOfMonstersEligibleForAutoSpawning(NR_LVLS_OUT_OF_DEPTH_ALLOWED, idCandidates);

  if(idCandidates.empty() == false) {
    const ActorDevNames_t id = idCandidates.at(eng->dice.getInRange(0, idCandidates.size() - 1));

    spawnGroupAt(id, sortedFreeCellsVector, forbiddenCells, IS_ROAMING_ALLOWED);
  }
}

void PopulateMonsters::attemptSpawnDueToTimePassed() const {
  tracer << "PopulateMonsters::attemptSpawnDueToTimePassed()..." << endl;

  bool blockers[MAP_X_CELLS][MAP_Y_CELLS];
  eng->mapTests->makeMoveBlockerArrayForMoveType(moveType_walk, blockers);

  const int MIN_DIST_TO_PLAYER = FOV_STANDARD_RADI_INT + 6;

  const coord& playerPos = eng->player->pos;
  for(int x = max(0, playerPos.x - MIN_DIST_TO_PLAYER); x <= min(MAP_X_CELLS - 1, playerPos.x + MIN_DIST_TO_PLAYER); x++) {
    for(int y = max(0, playerPos.y - MIN_DIST_TO_PLAYER); y <= min(MAP_Y_CELLS - 1, playerPos.y + MIN_DIST_TO_PLAYER); y++) {
      blockers[x][y] = true;
    }
  }

  vector<coord> freeCellsVector;
  for(int y = 1; y < MAP_Y_CELLS - 1; y++) {
    for(int x = 1; x < MAP_X_CELLS - 1; x++) {
      if(blockers[x][y] == false) {
        freeCellsVector.push_back(coord(x, y));
      }
    }
  }
  if(freeCellsVector.empty() == false) {

    const coord& origin = freeCellsVector.at(eng->dice.getInRange(0, freeCellsVector.size() - 1));

    makeSortedFreeCellsVector(origin, blockers, freeCellsVector);

    const coord& pos = freeCellsVector.at(eng->dice.getInRange(0, freeCellsVector.size() - 1));

    if(eng->map->explored[pos.x][pos.y]) {
      const int NR_LVLS_OUT_OF_DEPTH_OFFSET_ALLOWED = getRandomOutOfDepth();
      spawnGroupOfRandomAt(freeCellsVector, blockers, NR_LVLS_OUT_OF_DEPTH_OFFSET_ALLOWED, true);
    }
  }
  tracer << "PopulateMonsters::attemptSpawnDueToTimePassed() [DONE]" << endl;
}

void PopulateMonsters::populateRoomAndCorridorLevel(RoomTheme_t themeMap[MAP_X_CELLS][MAP_Y_CELLS], const vector<Room*>& rooms) const {
  const int NR_GROUPS_ALLOWED = eng->dice.getInRange(5, 8);
  int nrGroupsSpawned = 0;

  bool forbiddenCells[MAP_X_CELLS][MAP_Y_CELLS];

  const int MIN_DIST_FROM_PLAYER = FOV_STANDARD_RADI_INT - 1;
  eng->mapTests->makeMoveBlockerArrayForMoveType(moveType_walk, forbiddenCells);
  const coord& playerPos = eng->player->pos;

  for(int y = max(0, playerPos.y - MIN_DIST_FROM_PLAYER); y < min(MAP_Y_CELLS - 1, playerPos.y + MIN_DIST_FROM_PLAYER); y++) {
    for(int x = max(0, playerPos.x - MIN_DIST_FROM_PLAYER); x < min(MAP_X_CELLS - 1, playerPos.x + MIN_DIST_FROM_PLAYER); x++) {
      forbiddenCells[x][y] = true;
    }
  }

  //First, attempt to populate all non-plain themed rooms
  for(unsigned int i = 0; i < rooms.size(); i++) {
    Room* const room = rooms.at(i);
    if(room->roomTheme != roomTheme_plain) {

      const unsigned int MAX_NR_GROUPS_IN_ROOM = 3;
      for(unsigned int i_groupsInRoom = 0; i_groupsInRoom < MAX_NR_GROUPS_IN_ROOM; i_groupsInRoom++) {

        //Randomly pick a free position inside the room
        vector<coord> originCandidates;
        for(int y = room->getY0(); y <= room->getY1(); y++) {
          for(int x = room->getX0(); x <= room->getX1(); x++) {
            if(forbiddenCells[x][y] == false && themeMap[x][y] == room->roomTheme) {
              originCandidates.push_back(coord(x, y));
            }
          }
        }

        //If room is too full (due to spawned monsters and features), stop spawning in this room
        const int ROOM_W = room->getX1() - room->getX0() + 1;
        const int ROOM_H = room->getY1() - room->getY0() + 1;
        const unsigned int NR_CELLS_IN_ROOM = ROOM_W * ROOM_H;

        if(originCandidates.size() < NR_CELLS_IN_ROOM / 2) {
          continue;
        }

        //Spawn monsters in room
        if(originCandidates.empty() == false) {
          const coord& origin = originCandidates.at(eng->dice.getInRange(0, originCandidates.size() - 1));
          vector<coord> sortedFreeCellsVector;
          makeSortedFreeCellsVector(origin, forbiddenCells, sortedFreeCellsVector);

          if(spawnGroupOfRandomNativeToRoomThemeAt(room->roomTheme, sortedFreeCellsVector, forbiddenCells, false)) {
            nrGroupsSpawned++;
            if(nrGroupsSpawned >= NR_GROUPS_ALLOWED) {
              return;
            }
          }
        }
      }

      //After attempting to populate a non-plain themed room, mark that area as forbidden
      for(int y = room->getY0(); y <= room->getY1(); y++) {
        for(int x = room->getX0(); x <= room->getX1(); x++) {
          forbiddenCells[x][y] = true;
        }
      }
    }
  }

  //Second, place groups randomly in plain-themed areas until no more groups to place
  while(nrGroupsSpawned < NR_GROUPS_ALLOWED) {
    vector<coord> originCandidates;
    for(int y = 1; y < MAP_Y_CELLS - 1; y++) {
      for(int x = 1; x < MAP_X_CELLS - 1; x++) {
        if(forbiddenCells[x][y] == false && themeMap[x][y] == roomTheme_plain) {
          originCandidates.push_back(coord(x, y));
        }
      }
    }
    const coord origin = originCandidates.at(eng->dice.getInRange(0, originCandidates.size() - 1));
    vector<coord> sortedFreeCellsVector;
    makeSortedFreeCellsVector(origin, forbiddenCells, sortedFreeCellsVector);
    if(spawnGroupOfRandomNativeToRoomThemeAt(roomTheme_plain, sortedFreeCellsVector, forbiddenCells, true)) {
      nrGroupsSpawned++;
    }
  }
}

bool PopulateMonsters::spawnGroupOfRandomNativeToRoomThemeAt(const RoomTheme_t roomTheme, const vector<coord>& sortedFreeCellsVector,
    bool forbiddenCells[MAP_X_CELLS][MAP_Y_CELLS], const bool IS_ROAMING_ALLOWED) const {
  tracer << "PopulateMonsters::spawnGroupOfRandomNativeToRoomThemeAt()" << endl;
  const int NR_LEVELS_OUT_OF_DEPTH_ALLOWED = getRandomOutOfDepth();
  vector<ActorDevNames_t> idCandidates;
  makeListOfMonstersEligibleForAutoSpawning(NR_LEVELS_OUT_OF_DEPTH_ALLOWED, idCandidates);

  for(unsigned int i = 0; i < idCandidates.size(); i++) {
    const ActorDefinition& d = eng->actorData->actorDefinitions[idCandidates.at(i)];
    bool monsterIsNativeToRoom = false;
    for(unsigned int i_nativeRooms = 0; i_nativeRooms < d.nativeRooms.size(); i_nativeRooms++) {
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
    tracer << "PopulateMonsters: Found no valid monsters to spawn at room theme (" + intToString(roomTheme) + ")" << endl;
    return false;
  } else {
    const ActorDevNames_t id = idCandidates.at(eng->dice.getInRange(0, idCandidates.size() - 1));
    spawnGroupAt(id, sortedFreeCellsVector, forbiddenCells, IS_ROAMING_ALLOWED);
    return true;
  }
}

void PopulateMonsters::spawnGroupAt(const ActorDevNames_t id, const vector<coord>& sortedFreeCellsVector,
                                    bool forbiddenCells[MAP_X_CELLS][MAP_Y_CELLS], const bool IS_ROAMING_ALLOWED) const {

  const ActorDefinition& d = eng->actorData->actorDefinitions[id];

  int maxNrInGroup = 1;

  switch(d.groupSize) {
  case monsterGroupSize_few: {
    maxNrInGroup = eng->dice.getInRange(1, 3);
  }
  break;
  case monsterGroupSize_group: {
    maxNrInGroup = eng->dice.getInRange(3, 6);
  }
  break;
  case monsterGroupSize_horde: {
    maxNrInGroup = eng->dice.getInRange(8, 14);
  }
  break;
  default:
  {} break;
  }

  Actor* originActor = NULL;

  const int NR_FREE_CELLS = sortedFreeCellsVector.size();
  const int NR_CAN_BE_SPAWNED = min(NR_FREE_CELLS, maxNrInGroup);
  for(unsigned int i = 0; i < static_cast<unsigned int>(NR_CAN_BE_SPAWNED); i++) {
    const coord& pos = sortedFreeCellsVector.at(i);

    Actor* const actor = eng->actorFactory->spawnActor(id, pos);
    Monster* const monster = dynamic_cast<Monster*>(actor);
    monster->isRoamingAllowed = IS_ROAMING_ALLOWED;

    if(i == 0) {
      originActor = actor;
    } else {
      monster->leader = originActor;
    }

    forbiddenCells[pos.x][pos.y] = true;
  }
}

void PopulateMonsters::makeSortedFreeCellsVector(const coord& origin, const bool forbiddenCells[MAP_X_CELLS][MAP_Y_CELLS], vector<coord>& vectorToFill) const {
  vectorToFill.resize(0);

  for(int y = 1; y < MAP_Y_CELLS - 1; y++) {
    for(int x = 1; x < MAP_X_CELLS - 1; x++) {
      if(forbiddenCells[x][y] == false) {
        vectorToFill.push_back(coord(x, y));
      }
    }
  }

  IsCloserToOrigin sorter(origin, eng);
  std::sort(vectorToFill.begin(), vectorToFill.end(), sorter);
}

int PopulateMonsters::getRandomOutOfDepth() const {
  if(eng->map->getDungeonLevel() == 0) {
    return 0;
  }

  const int RND = eng->dice(1, 1000);

  if(RND <= 7) {
    return 5;
  }

  if(RND <= 75) {
    return 2;
  }

  return 0;
}

//bool PopulateMonsters::spawnGroupOfMonstersAtFreeCells(vector<coord>& freeCells, const bool ALLOW_ROAM /*,
//    const SpecialRoom_t belongingToSpecialRoomType */) const {
//  if(freeCells.size() > 0) {
//    tracer << "PopulateMonsters::spawnGroupOfMonstersAtFreeCells()" << endl;
//
//    const int FREE_CELLS_ELEMENT = eng->dice(1, freeCells.size()) - 1;
//    const coord pos(freeCells.at(FREE_CELLS_ELEMENT));
//
//    const int OUT_OF_DEPTH_OFFSET = getOutOfDepthOffset();
//
//    Actor* originActor = NULL;
//    originActor = eng->actorFactory->spawnRandomActor(pos, OUT_OF_DEPTH_OFFSET);
//
//    if(originActor != NULL) {
//      tracer << "PopulateMonsters: Valid actor found, spawning group of monsters" << endl;
//
//      dynamic_cast<Monster*>(originActor)->isRoamingAllowed = ALLOW_ROAM;
//
//      freeCells.erase(freeCells.begin() + FREE_CELLS_ELEMENT);
//
//      int extraSpawns = 0;
//
//      const ActorDefinition* const d = originActor->getDef();
//      switch(d->groupSize) {
//      case monsterGroupSize_few: {
//        extraSpawns = eng->dice.getInRange(0, 2);
//      }
//      break;
//      case monsterGroupSize_group: {
//        extraSpawns = eng->dice.getInRange(2, 5);
//      }
//      break;
//      case monsterGroupSize_horde: {
//        extraSpawns = eng->dice.getInRange(7, 13);
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
//          const coord posExtra = freeCells.front();
//          Actor* const actorExtra = eng->actorFactory->spawnActor(originActor->getDef()->devName, posExtra);
//          dynamic_cast<Monster*>(actorExtra)->isRoamingAllowed = ALLOW_ROAM;
//          dynamic_cast<Monster*>(actorExtra)->leader = originActor;
//          freeCells.erase(freeCells.begin());
//        }
//      }
//      return true;
//    }
//  }
//  tracer << "PopulateMonsters: Spawning group of monster failed (probably no monster met criteria)" << endl;
//  return false;
//}

//void PopulateMonsters::populate() const {
//  tracer << "PopulateMonsters::PopulateMonsters()..." << endl;
//  bool blockers[MAP_X_CELLS][MAP_Y_CELLS];
//  eng->mapTests->makeMoveBlockerArrayForMoveType(moveType_walk, blockers);
//  eng->basicUtils->reverseBoolArray(blockers);
//  vector<coord> freeCells;
//  eng->mapTests->makeMapVectorFromArray(blockers, freeCells);
//
//  const int MIN_DIST_FROM_PLAYER = eng->map->getDungeonLevel() == 0 ? FOV_STANDARD_RADI_INT + 1 : FOV_STANDARD_RADI_INT - 1;
//
//  unsigned int i = 0;
//  bool increaseIndex = true;
//  while(i != freeCells.size()) {
//    increaseIndex = true;
//    if(eng->basicUtils->chebyshevDistance(freeCells.at(i), eng->player->pos) < MIN_DIST_FROM_PLAYER) {
//      freeCells.erase(freeCells.begin() + i);
//      increaseIndex = false;
//    }
//
//    i += increaseIndex ? 1 : 0;
//  }
//
//  if(eng->map->getDungeonLevel() >= 1 && eng->map->getDungeonLevel() < FIRST_CAVERN_LEVEL) {
//    bool blockersTraps[MAP_X_CELLS][MAP_Y_CELLS];
//    eng->mapTests->makeItemBlockerArray(blockersTraps);
//    eng->basicUtils->reverseBoolArray(blockersTraps);
//    vector<coord> freeCellsTraps;
//    eng->mapTests->makeMapVectorFromArray(blockersTraps, freeCellsTraps);
//
//    const int MAX_CELLS_PER_TRAP = 210;
//    int nrOfTrapsOnMap = static_cast<int>(freeCellsTraps.size()) / MAX_CELLS_PER_TRAP;
//    nrOfTrapsOnMap = max(2, nrOfTrapsOnMap);
//    nrOfTrapsOnMap += eng->dice.getInRange(0, nrOfTrapsOnMap / 4);
//
//    int n = 0;
//    for(int ii = 0; ii < nrOfTrapsOnMap; ii++) {
//      if(freeCellsTraps.size() > 0) {
//        n = eng->dice(1, freeCellsTraps.size()) - 1;
//        const coord pos(freeCellsTraps.at(n));
//
//        if(eng->map->featuresStatic[pos.x][pos.y]->canHaveStaticFeature()) {
//          const FeatureDef* const defAtTrap = eng->featureData->getFeatureDef(eng->map->featuresStatic[pos.x][pos.y]->getId());
//          eng->featureFactory->spawnFeatureAt(feature_trap, pos, new TrapSpawnData(defAtTrap, trap_any));
//          freeCellsTraps.erase(freeCellsTraps.begin() + n);
//        }
//      }
//    }
//  }
//
//  const int CELLS_PER_MONSTER_GROUP = eng->map->getDungeonLevel() == 0 ? 500 : 110;
//
//  tracer << "PopulateMonsters: Cells per monster group: " << CELLS_PER_MONSTER_GROUP << endl;
//
//  int nrMonsterGroupsOnMap = freeCells.size() / CELLS_PER_MONSTER_GROUP;
//
//  nrMonsterGroupsOnMap = max(1, nrMonsterGroupsOnMap);
//  nrMonsterGroupsOnMap += eng->dice.getInRange(0, nrMonsterGroupsOnMap / 4);
//
//  tracer << "PopulateMonsters: Number of monster groups on map: " << nrMonsterGroupsOnMap << endl;
//
//  //Spawn monsters randomly from the coord-vector
//  for(int ii = 0; ii < nrMonsterGroupsOnMap; ii++) {
//    ii -= spawnGroupOfMonstersAtFreeCells(freeCells, true) == false ? 1 : 0;
//  }
//  tracer << "PopulateMonsters::PopulateMonsters() [DONE]" << endl;
//}

//void PopulateMonsters::spawnOneMonster() const {
//  bool blockers[MAP_X_CELLS][MAP_Y_CELLS];
//  eng->mapTests->makeMoveBlockerArrayForMoveType(moveType_walk, blockers);
//  eng->basicUtils->reverseBoolArray(blockers);
//  vector<coord> freeCells;
//  eng->mapTests->makeMapVectorFromArray(blockers, freeCells);
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
//      const unsigned int FREE_CELLS_ELEMENT = eng->dice(1, freeCells.size()) - 1;
//
//      if(eng->map->explored[freeCells.at(FREE_CELLS_ELEMENT).x][freeCells.at(FREE_CELLS_ELEMENT).y] == false) {
//        done = true;
//        break;
//      }
//
//      const coord dif = freeCells.at(FREE_CELLS_ELEMENT) - eng->player->pos;
//
//      if((dif.x >= MIN_DIST_FROM_PLAYER || dif.x <= -MIN_DIST_FROM_PLAYER) || (dif.y >= MIN_DIST_FROM_PLAYER || dif.y <= -MIN_DIST_FROM_PLAYER)) {
//        const int OUT_OF_DEPTH_OFFSET = getOutOfDepthOffset();
//        eng->actorFactory->spawnRandomActor(freeCells.at(FREE_CELLS_ELEMENT), OUT_OF_DEPTH_OFFSET);
//        done = true;
//      } else {
//        freeCells.erase(freeCells.begin() + FREE_CELLS_ELEMENT);
//      }
//    }
//  }
//}
