#include "FeatureProxEvent.h"

#include <assert.h>

#include "ActorPlayer.h"
#include "ActorMonster.h"
#include "Map.h"
#include "ActorFactory.h"
#include "Log.h"
#include "Renderer.h"
#include "Utils.h"
#include "FeatureRigid.h"

using namespace std;

//-------------------------------------------PROX EVENT
void ProxEvent::onNewTurn() {
  if(Utils::isPosAdj(pos_, Map::player->pos, true)) {onPlayerAdj();}
}

//-------------------------------------------WALL CRUMBLE
void ProxEventWallCrumble::onPlayerAdj() {
  //Check that it still makes sense to run the crumbling
  auto checkCellsHaveWall = [](const vector<Pos>& cells) {
    for(const Pos& p : cells) {
      const auto fId = Map::cells[p.x][p.y].rigid->getId();
      if(fId != FeatureId::wall && fId != FeatureId::rubbleHigh) {return false;}
    }
    return true;
  };

  if(checkCellsHaveWall(wallCells_) && checkCellsHaveWall(innerCells_)) {

    if(Map::player->getPropHandler().allowSee()) {
      Log::addMsg("Suddenly, the walls collapse!", clrWhite, false, true);
    }

    //Crumble
    bool done = false;
    while(!done) {
      for(const Pos& p : wallCells_) {
        if(Utils::isPosInside(p, Rect(Pos(1, 1), Pos(MAP_W - 2, MAP_H - 2)))) {
          auto* const f = Map::cells[p.x][p.y].rigid;
          f->hit(DmgType::physical, DmgMethod::forced, nullptr);
        }
      }

      bool isOpeningMade = true;
      for(const Pos& p : wallCells_) {
        if(Utils::isPosAdj(Map::player->pos, p, true)) {
          Rigid* const f = Map::cells[p.x][p.y].rigid;
          if(!f->canMoveCmn()) {isOpeningMade = false;}
        }
      }

      Map::player->updateFov();
      Renderer::drawMapAndInterface();

      done = isOpeningMade;
    }

    //Spawn things
    int nrMonsterLimitExceptAdjToEntry = 9999;
    ActorId monsterType = ActorId::zombie;
    const int RND = Rnd::range(1, 5);
    switch(RND) {
      case 1: {
        monsterType = ActorId::zombie;
        nrMonsterLimitExceptAdjToEntry = 4;
      } break;

      case 2: {
        monsterType = ActorId::zombieAxe;
        nrMonsterLimitExceptAdjToEntry = 3;
      } break;

      case 3: {
        monsterType = ActorId::bloatedZombie;
        nrMonsterLimitExceptAdjToEntry = 1;
      } break;

      case 4: {
        monsterType = ActorId::rat;
        nrMonsterLimitExceptAdjToEntry = 30;
      } break;

      case 5: {
        monsterType = ActorId::ratThing;
        nrMonsterLimitExceptAdjToEntry = 20;
      } break;

      default: {} break;
    }
    int nrMonstersSpawned = 0;

    random_shuffle(begin(innerCells_), end(innerCells_));

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
        ++nrMonstersSpawned;
      }
    }

    Map::player->updateFov();
    Renderer::drawMapAndInterface();
  }
  GameTime::eraseMob(this, true);
}

