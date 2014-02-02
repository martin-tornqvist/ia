#include "FeatureProxEvent.h"

#include "Engine.h"
#include "ActorPlayer.h"
#include "ActorMonster.h"
#include "Map.h"
#include "Blood.h"
#include "ActorFactory.h"
#include "Log.h"
#include "Renderer.h"

//-------------------------------------------PROX EVENT
void ProxEvent::newTurn() {
  if(eng.basicUtils->isPosAdj(pos_, eng.player->pos, true)) {
    playerIsNear();
  }
}

//-------------------------------------------WALL CRUMBLE
void ProxEventWallCrumble::playerIsNear() {
  //Check that it still makes sense to run the crumbling
  bool isCrumbleOk = true;
  const int NR_WALL_CELLS = wallCells_.size();
  for(int i = 0; i < NR_WALL_CELLS; i++) {
    const Pos c = wallCells_.at(i);
    FeatureStatic* const f = eng.map->cells[c.x][c.y].featureStatic;
    const bool IS_VISION_PASSABLE = f->isVisionPassable();
    const bool IS_WALK_PASSABLE   = f->canMoveCmn();
    if(IS_VISION_PASSABLE || IS_WALK_PASSABLE) {
      isCrumbleOk = false;
      break;
    }
  }
  const int NR_INNER_CELLS = innerCells_.size();
  for(int i = 0; i < NR_INNER_CELLS; i++) {
    const Pos c = innerCells_.at(i);
    FeatureStatic* const f  = eng.map->cells[c.x][c.y].featureStatic;
    const bool IS_VISION_PASSABLE = f->isVisionPassable();
    const bool IS_WALK_PASSABLE   = f->canMoveCmn();
    if(IS_VISION_PASSABLE || IS_WALK_PASSABLE) {
      isCrumbleOk = false;
      break;
    }
  }

  if(isCrumbleOk) {
    //Crumble
    bool done = false;
    while(done == false) {
      for(int i = 0; i < NR_WALL_CELLS; i++) {
        const Pos pos = wallCells_.at(i);
        if(eng.basicUtils->isPosInside(
              pos, Rect(Pos(1, 1), Pos(MAP_W - 2, MAP_H - 2)))) {
          eng.map->switchToDestroyedFeatAt(wallCells_.at(i));
        }
      }

      bool isOpeningMade = true;
      for(int i = 0; i < NR_WALL_CELLS; i++) {
        const Pos pos = wallCells_.at(i);
        if(eng.basicUtils->isPosAdj(eng.player->pos, pos, true)) {
          FeatureStatic* const f = eng.map->cells[pos.x][pos.y].featureStatic;
          if(f->canMoveCmn() == false) {
            isOpeningMade = false;
          }
        }
      }

      eng.player->updateFov();
      eng.renderer->drawMapAndInterface();

      done = isOpeningMade;
    }

    //Spawn things
    int nrMonsterLimitExceptAdjToEntry = 9999;
    ActorId_t monsterType = actor_zombie;
    const int RND = eng.dice.range(1, 5);
    switch(RND) {
      case 1: {
        monsterType = actor_zombie;
        nrMonsterLimitExceptAdjToEntry = 4;
      } break;

      case 2: {
        monsterType = actor_zombieAxe;
        nrMonsterLimitExceptAdjToEntry = 3;
      } break;

      case 3: {
        monsterType = actor_bloatedZombie;
        nrMonsterLimitExceptAdjToEntry = 1;
      } break;

      case 4: {
        monsterType = actor_rat;
        nrMonsterLimitExceptAdjToEntry = 30;
      } break;

      case 5: {
        monsterType = actor_ratThing;
        nrMonsterLimitExceptAdjToEntry = 20;
      } break;

      default: {} break;
    }
    int nrMonstersSpawned = 0;

    for(int i = 0; i < NR_INNER_CELLS; i++) {
      const Pos& pos = innerCells_.at(i);

      eng.featureFactory->spawnFeatureAt(
        feature_stoneFloor, pos);

      if(eng.dice.range(1, 100) < 20) {
        eng.gore->makeGore(pos);
        eng.gore->makeBlood(pos);
      }

      if(
        nrMonstersSpawned < nrMonsterLimitExceptAdjToEntry ||
        eng.basicUtils->isPosAdj(pos, pos_, false)) {
        Actor* const actor = eng.actorFactory->spawnActor(monsterType, pos);
        Monster* const monster = dynamic_cast<Monster*>(actor);
        monster->awareOfPlayerCounter_ =
          monster->getData().nrTurnsAwarePlayer;
        nrMonstersSpawned++;
      }
    }

    eng.log->addMsg("The walls suddenly crumbles!");
    eng.player->updateFov();
    eng.renderer->drawMapAndInterface();
    eng.gameTime->eraseFeatureMob(this, true);
  }
}

