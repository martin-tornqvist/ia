#include "FeatureProxEvent.h"

#include "Engine.h"
#include "ActorPlayer.h"
#include "ActorMonster.h"
#include "Map.h"
#include "Blood.h"
#include "ActorFactory.h"
#include "MapTests.h"
#include "Log.h"

//-------------------------------------------PROX EVENT
void ProxEvent::newTurn() {
  if(eng->mapTests->isCellsNeighbours(pos_, eng->player->pos, true)) {
    playerIsNear();
  }
}


//-------------------------------------------WALL CRUMBLE
void ProxEventWallCrumble::playerIsNear() {
  //Check that it still makes sense to run the crumbling
  bool isCrumbleOk = true;
  for(unsigned int i = 0; i < wallCells_.size(); i++) {
    const coord c = wallCells_.at(i);
    const bool IS_VISION_PASSABLE = eng->map->featuresStatic[c.x][c.y]->isVisionPassable();
    const bool IS_WALK_PASSABLE = eng->map->featuresStatic[c.x][c.y]->isMoveTypePassable(moveType_walk);
    if(IS_VISION_PASSABLE || IS_WALK_PASSABLE) {
      isCrumbleOk = false;
      break;
    }
  }
  for(unsigned int i = 0; i < innerCells_.size(); i++) {
    const coord c = innerCells_.at(i);
    const bool IS_VISION_PASSABLE = eng->map->featuresStatic[c.x][c.y]->isVisionPassable();
    const bool IS_WALK_PASSABLE = eng->map->featuresStatic[c.x][c.y]->isMoveTypePassable(moveType_walk);
    if(IS_VISION_PASSABLE || IS_WALK_PASSABLE) {
      isCrumbleOk = false;
      break;
    }
  }

  if(isCrumbleOk) {
    //Crumble
    bool done = false;
    while(done == false) {
      for(unsigned int i = 0; i < wallCells_.size(); i++) {
        if(eng->mapTests->isCellInside(wallCells_.at(i), coord(1, 1), coord(MAP_X_CELLS - 2, MAP_Y_CELLS - 2))) {
          eng->map->switchToDestroyedFeatAt(wallCells_.at(i));
        }
      }

      bool isOpeningMade = true;
      for(unsigned int ii = 0; ii < wallCells_.size(); ii++) {
        if(eng->mapTests->isCellsNeighbours(eng->player->pos, wallCells_.at(ii), true)) {
          if(eng->map->featuresStatic[wallCells_.at(ii).x][wallCells_.at(ii).y]->isMoveTypePassable(moveType_walk) == false) {
            isOpeningMade = false;
          }
        }
      }

      eng->player->FOVupdate();
      eng->renderer->drawMapAndInterface();

      done = isOpeningMade;
    }

    //Spawn things
    int nrMonsterLimitExceptAdjToEntry = 9999;
    ActorId_t monsterType = actor_zombie;
    const int RND = eng->dice.getInRange(0, 4);
    switch(RND) {
    case 0: {monsterType = actor_zombie; nrMonsterLimitExceptAdjToEntry = 4;} break;
    case 1: {monsterType = actor_zombieAxe; nrMonsterLimitExceptAdjToEntry = 3;} break;
    case 2: {monsterType = actor_bloatedZombie; nrMonsterLimitExceptAdjToEntry = 1;} break;
    case 3: {monsterType = actor_rat; nrMonsterLimitExceptAdjToEntry = 30;} break;
    case 4: {monsterType = actor_ratThing; nrMonsterLimitExceptAdjToEntry = 20;} break;
    default: {} break;
    }
    int nrMonstersSpawned = 0;
    for(unsigned int i = 0; i < innerCells_.size(); i++) {

      eng->featureFactory->spawnFeatureAt(feature_stoneFloor, innerCells_.at(i));

      if(eng->dice.getInRange(1, 100) < 20) {
        eng->gore->makeGore(innerCells_.at(i));
        eng->gore->makeBlood(innerCells_.at(i));
      }

      if(nrMonstersSpawned < nrMonsterLimitExceptAdjToEntry || eng->mapTests->isCellsNeighbours(innerCells_.at(i), pos_, false)) {
        Monster* const monster = dynamic_cast<Monster*>(eng->actorFactory->spawnActor(monsterType, innerCells_.at(i)));
        monster->playerAwarenessCounter = monster->getDef()->nrTurnsAwarePlayer;
        nrMonstersSpawned++;
      }
    }

    eng->log->addMessage("The walls suddenly crumbles!");
    eng->player->FOVupdate();
    eng->renderer->drawMapAndInterface();
    eng->gameTime->eraseFeatureMob(this, true);
  }
}

