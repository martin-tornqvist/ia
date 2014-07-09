#include "FeatureProxEvent.h"

#include "ActorPlayer.h"
#include "ActorMonster.h"
#include "Map.h"
#include "ActorFactory.h"
#include "Log.h"
#include "Renderer.h"
#include "Utils.h"
#include "FeatureStatic.h"

//-------------------------------------------PROX EVENT
void ProxEvent::newTurn() {
  if(Utils::isPosAdj(pos_, Map::player->pos, true)) {
    playerIsNear();
  }
}

//-------------------------------------------WALL CRUMBLE
void ProxEventWallCrumble::playerIsNear() {
  //Check that it still makes sense to run the crumbling
  bool isCrumbleOk = true;
  for(const Pos& p : wallCells_) {
    FeatureStatic* const f = Map::cells[p.x][p.y].featureStatic;
    const bool IS_VISION_PASSABLE = f->isVisionPassable();
    const bool IS_WALK_PASSABLE   = f->canMoveCmn();
    if(IS_VISION_PASSABLE || IS_WALK_PASSABLE) {
      isCrumbleOk = false;
      break;
    }
  }
  for(const Pos& p : innerCells_) {
    FeatureStatic* const f  = Map::cells[p.x][p.y].featureStatic;
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
    while(!done) {
      for(const Pos& p : wallCells_) {
        if(Utils::isPosInside(p, Rect(Pos(1, 1), Pos(MAP_W - 2, MAP_H - 2)))) {
          auto* const f = Map::cells[p.x][p.y].featureStatic;
          f->hit(DmgType::physical, DmgMethod::explosion);
        }
      }

      bool isOpeningMade = true;
      for(const Pos& p : wallCells_) {
        if(Utils::isPosAdj(Map::player->pos, p, true)) {
          FeatureStatic* const f = Map::cells[p.x][p.y].featureStatic;
          if(!f->canMoveCmn()) {isOpeningMade = false;}
        }
      }

      Map::player->updateFov();
      Renderer::drawMapAndInterface();

      done = isOpeningMade;
    }

    //Spawn things
    int nrMonsterLimitExceptAdjToEntry = 9999;
    ActorId monsterType = actor_zombie;
    const int RND = Rnd::range(1, 5);
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

    for(const Pos& p : innerCells_) {
      Map::put(new Floor(p));

      if(Rnd::oneIn(5)) {
        Map::mkGore(p);
        Map::mkBlood(p);
      }

      if(
        nrMonstersSpawned < nrMonsterLimitExceptAdjToEntry ||
        Utils::isPosAdj(p, pos_, false)) {
        Actor* const actor              = ActorFactory::mk(monsterType, p);
        Monster* const monster          = static_cast<Monster*>(actor);
        monster->awareOfPlayerCounter_  = monster->getData().nrTurnsAwarePlayer;
        nrMonstersSpawned++;
      }
    }

    Log::addMsg("The walls suddenly crumbles!");
    Map::player->updateFov();
    Renderer::drawMapAndInterface();
    GameTime::eraseFeatureMob(this, true);
  }
}

