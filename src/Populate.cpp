#include "Populate.h"

#include <algorithm>

#include "Engine.h"
#include "FeatureTrap.h"
#include "Map.h"
#include "ActorFactory.h"
#include "ActorMonster.h"
#include "ActorPlayer.h"
#include "FeatureFactory.h"

using namespace std;

int Populate::getOutOfDepthOffset() const {
  if(eng->map->getDungeonLevel() == 0) {
    return 0;
  }

  const int RND = eng->dice(1, 1000);

  if(RND <= 7) {
    return eng->dice.getInRange(1, 7);
  }

  if(RND <= 75) {
    return eng->dice.getInRange(1, 3);
  }

  return 0;
}

bool Populate::spawnGroupOfMonstersAtFreeCells(vector<coord>& freeCells, const bool IS_AFTER_MAP_CREATION, const bool ALLOW_ROAM /*,
    const SpecialRoom_t belongingToSpecialRoomType */) const {
  if(freeCells.size() > 0) {
    tracer << "Populate::spawnGroupOfMonstersAtFreeCells()" << endl;

    const int FREE_CELLS_ELEMENT = eng->dice(1, freeCells.size()) - 1;
    const coord pos(freeCells.at(FREE_CELLS_ELEMENT));

    const int OUT_OF_DEPTH_OFFSET = getOutOfDepthOffset();

//    tracer << "Populate: belongingToSpecialRoomType: " << belongingToSpecialRoomType << endl;

    Actor* originActor = NULL;
//    if(belongingToSpecialRoomType == endOfSpecialRooms) {
//      tracer << "Populate: no special room set, spawning any random actor" << endl;
      originActor = eng->actorFactory->spawnRandomActor(pos, OUT_OF_DEPTH_OFFSET, IS_AFTER_MAP_CREATION);
//    } else {
//      tracer << "Populate: special room is set, spawning random actor belonging to room" << endl;
//      originActor = eng->actorFactory->spawnRandomActorRelatedToSpecialRoom(pos, belongingToSpecialRoomType, OUT_OF_DEPTH_OFFSET);
//    }

    if(originActor != NULL) {
      dynamic_cast<Monster*>(originActor)->isRoamingAllowed = ALLOW_ROAM;

      freeCells.erase(freeCells.begin() + FREE_CELLS_ELEMENT);

      const int DLVL_IS_ZERO = eng->map->getDungeonLevel() == 0;

      const int CHANCE_FOR_EXTRA = dynamic_cast<Monster*>(originActor)->getDef()->chanceToSpawnExtra;

      int baseChanceForExtra = CHANCE_FOR_EXTRA - 10 * DLVL_IS_ZERO;

      IsCloserToOrigin isCloserToOrigin(pos, eng);

      sort(freeCells.begin(), freeCells.end(), isCloserToOrigin);

      while(eng->dice(1, 100) < baseChanceForExtra) {
        if(freeCells.size() > 0) {
          const coord posExtra = freeCells.front();
          Actor* const actor = eng->actorFactory->spawnActor(originActor->getDef()->devName, posExtra);
          dynamic_cast<Monster*>(actor)->isRoamingAllowed = ALLOW_ROAM;
          dynamic_cast<Monster*>(actor)->leader = originActor;
          freeCells.erase(freeCells.begin());
        }
        baseChanceForExtra -= 5;
      }
      return true;
    }
  }
  return false;
}

void Populate::populate() const {
  tracer << "Populate::populate()..." << endl;
  bool blockers[MAP_X_CELLS][MAP_Y_CELLS];
  eng->mapTests->makeMoveBlockerArrayForMoveType(moveType_walk, blockers);
  eng->basicUtils->reverseBoolArray(blockers);
  vector<coord> freeCells;
  eng->mapTests->makeMapVectorFromArray(blockers, freeCells);

  const int MIN_DIST_FROM_PLAYER = eng->map->getDungeonLevel() == 0 ? FOV_STANDARD_RADI_INT + 1 : FOV_STANDARD_RADI_INT - 1;

//  const unsigned int NR_OF_SPECIAL_ROOMS = eng->specialRoomHandler->getNrOfRooms();
  unsigned int i = 0;
  bool increaseIndex = true;
  while(i != freeCells.size()) {
    increaseIndex = true;
    if(eng->basicUtils->chebyshevDistance(freeCells.at(i), eng->player->pos) < MIN_DIST_FROM_PLAYER) {
      freeCells.erase(freeCells.begin() + i);
      increaseIndex = false;
    }

//    if(i < freeCells.size()) {
//      for(unsigned int room_i = 0; room_i < NR_OF_SPECIAL_ROOMS; room_i++) {
//        if(eng->specialRoomHandler->getRoomAtIndex(room_i)->isCellInside(freeCells.at(i))) {
//          freeCells.erase(freeCells.begin() + i);
//          increaseIndex = false;
//          room_i = 99999;
//        }
//      }
//    }
    i += increaseIndex ? 1 : 0;
  }

  if(eng->map->getDungeonLevel() >= 1 && eng->map->getDungeonLevel() < FIRST_CAVERN_LEVEL) {
    bool blockersTraps[MAP_X_CELLS][MAP_Y_CELLS];
    eng->mapTests->makeItemBlockerArray(blockersTraps);
    eng->basicUtils->reverseBoolArray(blockersTraps);
    vector<coord> freeCellsTraps;
    eng->mapTests->makeMapVectorFromArray(blockersTraps, freeCellsTraps);

    const int MAX_CELLS_PER_TRAP = 165;
    int nrOfTrapsOnMap = static_cast<int>(freeCellsTraps.size()) / MAX_CELLS_PER_TRAP;
    nrOfTrapsOnMap = max(2, nrOfTrapsOnMap);
    if(nrOfTrapsOnMap > 1) {
      nrOfTrapsOnMap += eng->dice(1, nrOfTrapsOnMap / 4);
    }

    int n = 0;
    for(int ii = 0; ii < nrOfTrapsOnMap; ii++) {
      if(freeCellsTraps.size() > 0) {
        n = eng->dice(1, freeCellsTraps.size()) - 1;
        const coord pos(freeCellsTraps.at(n));

        if(eng->map->featuresStatic[pos.x][pos.y]->canHaveStaticFeature()) {
          const FeatureDef* const defAtTrap = eng->featureData->getFeatureDef(eng->map->featuresStatic[pos.x][pos.y]->getId());
          eng->featureFactory->spawnFeatureAt(feature_trap, pos, new TrapSpawnData(defAtTrap, trap_any));
          freeCellsTraps.erase(freeCellsTraps.begin() + n);
        }
      }
    }
  }

  const int CELLS_PER_MONSTER_GROUP = eng->map->getDungeonLevel() == 0 ? 170 : 110;

  tracer << "Populate: Cells per monster group: " << CELLS_PER_MONSTER_GROUP << endl;

  int nrMonsterGroupsOnMap = freeCells.size() / CELLS_PER_MONSTER_GROUP;

  tracer << "Populate: Number of monster groups on map: " << nrMonsterGroupsOnMap << endl;

  nrMonsterGroupsOnMap = max(3, nrMonsterGroupsOnMap);
  nrMonsterGroupsOnMap += eng->dice.getInRange(0, nrMonsterGroupsOnMap / 4);

  //Spawn monsters randomly from the coord-vector
  for(int ii = 0; ii < nrMonsterGroupsOnMap; ii++) {
    ii -= spawnGroupOfMonstersAtFreeCells(freeCells, false, true) == false ? 1 : 0;
  }
  tracer << "Populate::populate() [DONE]" << endl;
}

void Populate::spawnOneMonster(const bool IS_AFTER_MAP_CREATION) const {
  bool blockers[MAP_X_CELLS][MAP_Y_CELLS];
  eng->mapTests->makeMoveBlockerArrayForMoveType(moveType_walk, blockers);
  eng->basicUtils->reverseBoolArray(blockers);
  vector<coord> freeCells;
  eng->mapTests->makeMapVectorFromArray(blockers, freeCells);

  const int MIN_DIST_FROM_PLAYER = FOV_STANDARD_RADI_INT + 4;

  if(freeCells.size() > 0) {
    bool done = false;
    while(done == false) {

      if(freeCells.size() == 0) {
        done = true;
        break;
      }

      const unsigned int FREE_CELLS_ELEMENT = eng->dice(1, freeCells.size()) - 1;

      if(eng->map->explored[freeCells.at(FREE_CELLS_ELEMENT).x][freeCells.at(FREE_CELLS_ELEMENT).y] == false) {
        done = true;
        break;
      }

      const coord dif = freeCells.at(FREE_CELLS_ELEMENT) - eng->player->pos;

      if((dif.x >= MIN_DIST_FROM_PLAYER || dif.x <= -MIN_DIST_FROM_PLAYER) || (dif.y >= MIN_DIST_FROM_PLAYER || dif.y <= -MIN_DIST_FROM_PLAYER)) {
        const int OUT_OF_DEPTH_OFFSET = getOutOfDepthOffset();
        eng->actorFactory->spawnRandomActor(freeCells.at(FREE_CELLS_ELEMENT), OUT_OF_DEPTH_OFFSET, IS_AFTER_MAP_CREATION);
        done = true;
      } else {
        freeCells.erase(freeCells.begin() + FREE_CELLS_ELEMENT);
      }
    }
  }
}
