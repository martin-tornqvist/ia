#include "Throwing.h"

#include <vector>
#include <assert.h>

#include "Item.h"
#include "ItemPotion.h"
#include "ActorData.h"
#include "ActorPlayer.h"
#include "Render.h"
#include "Map.h"
#include "Log.h"
#include "Explosion.h"
#include "ItemDrop.h"
#include "Inventory.h"
#include "ItemFactory.h"
#include "Attack.h"
#include "LineCalc.h"
#include "PlayerBon.h"
#include "Utils.h"
#include "SdlWrapper.h"
#include "FeatureRigid.h"
#include "FeatureMob.h"

using namespace std;

namespace Throwing {

void playerThrowLitExplosive(const Pos& aimCell) {
  assert(Map::player->activeExplosive);

  vector<Pos> path;
  LineCalc::calcNewLine(Map::player->pos, aimCell, true, THROW_RANGE_LMT, false, path);

  //Remove cells after blocked cells
  for(size_t i = 1; i < path.size(); ++i) {
    const Pos   p = path.at(i);
    const auto* f = Map::cells[p.x][p.y].rigid;
    if(!f->isProjectilePassable()) {
      path.resize(i);
      break;
    }
  }

  const Pos endPos(path.empty() ? Pos() : path.back());

  auto* const explosive = Map::player->activeExplosive;

  Log::addMsg(explosive->getStrOnPlayerThrow());

  //Render
  if(path.size() > 1) {
    const auto  clr = explosive->getIgnitedProjectileClr();

    for(const Pos& p : path) {
      Render::drawMapAndInterface(false);
      if(Map::cells[p.x][p.y].isSeenByPlayer) {
        if(Config::isTilesMode()) {
          Render::drawTile(explosive->getTile(),    Panel::map, p, clr);
        } else {
          Render::drawGlyph(explosive->getGlyph(),  Panel::map, p, clr);
        }
        Render::updateScreen();
        SdlWrapper::sleep(Config::getDelayProjectileDraw());
      }
    }
  }

  if(!Map::cells[endPos.x][endPos.y].rigid->isBottomless()) {
    explosive->onThrownIgnitedLanding(endPos);
  }

  delete explosive;
  Map::player->activeExplosive = nullptr;

  GameTime::actorDidAct();
}

void throwItem(Actor& actorThrowing, const Pos& targetCell, Item& itemThrown) {
  ThrowAttData* data =
    new ThrowAttData(actorThrowing, itemThrown, targetCell, actorThrowing.pos);

  const ActorSize aimLvl = data->intendedAimLvl;

  vector<Pos> path;
  LineCalc::calcNewLine(actorThrowing.pos, targetCell, false, THROW_RANGE_LMT,
                        false, path);

  const ItemDataT& itemThrownData = itemThrown.getData();

  const string itemNameA = itemThrown.getName(ItemRefType::a);
  if(&actorThrowing == Map::player) {
    Log::clearLog();
    Log::addMsg("I throw " + itemNameA + ".");
  } else {
    const Pos& p = path.front();
    if(Map::cells[p.x][p.y].isSeenByPlayer) {
      Log::addMsg(actorThrowing.getNameThe() + " throws " + itemNameA + ".");
    }
  }
  Render::drawMapAndInterface(true);

  int       blockedInElement      = -1;
  bool        isActorHit          = false;
  const char  glyph               = itemThrown.getGlyph();
  const Clr   clr                 = itemThrown.getClr();
  int         chanceToDestroyItem = 0;

  Pos curPos(-1, -1);

  for(size_t i = 1; i < path.size(); ++i) {
    Render::drawMapAndInterface(false);

    curPos.set(path.at(i));

    Actor* const actorHere = Utils::getFirstActorAtPos(curPos);
    if(actorHere) {
      if(
        curPos == targetCell ||
        actorHere->getData().actorSize >= actorSize_humanoid) {

        delete data;
        data = new ThrowAttData(actorThrowing, itemThrown, targetCell, curPos, aimLvl);

        if(
          data->attackResult >= successSmall &&
          !data->isEtherealDefenderMissed) {
          if(Map::cells[curPos.x][curPos.y].isSeenByPlayer) {
            Render::drawGlyph('*', Panel::map, curPos, clrRedLgt);
            Render::updateScreen();
            SdlWrapper::sleep(Config::getDelayProjectileDraw() * 4);
          }
          const Clr hitMessageClr   = actorHere == Map::player ? clrMsgBad : clrMsgGood;
          const bool CAN_SEE_ACTOR  = Map::player->isSeeingActor(*actorHere, nullptr);
          string defenderName       = CAN_SEE_ACTOR ? actorHere->getNameThe() : "It";

          Log::addMsg(defenderName + " is hit.", hitMessageClr);

          actorHere->hit(data->dmg, DmgType::physical);
          isActorHit = true;

          //If throwing a potion on an actor, let it make stuff happen...
          if(itemThrownData.isPotion) {
            static_cast<Potion*>(&itemThrown)->collide(curPos, actorHere);
            delete &itemThrown;
            delete data;
            GameTime::actorDidAct();
            return;
          }

          blockedInElement = i;
          chanceToDestroyItem = 25;
          break;
        }
      }
    }

    if(Map::cells[curPos.x][curPos.y].isSeenByPlayer) {
      Render::drawGlyph(glyph, Panel::map, curPos, clr);
      Render::updateScreen();
      SdlWrapper::sleep(Config::getDelayProjectileDraw());
    }

    const auto* featureHere = Map::cells[curPos.x][curPos.y].rigid;
    if(!featureHere->isProjectilePassable()) {
      blockedInElement = itemThrownData.isPotion ? i : i - 1;
      break;
    }

    if(curPos == targetCell) {
      blockedInElement = i;
      break;
    }
  }

  //If potion, collide it on the landscape
  if(itemThrownData.isPotion) {
    if(blockedInElement >= 0) {
      static_cast<Potion*>(&itemThrown)->collide(
        path.at(blockedInElement), nullptr);
      delete &itemThrown;
      delete data;
      GameTime::actorDidAct();
      return;
    }
  }

  if(Rnd::percentile() < chanceToDestroyItem) {
    delete &itemThrown;
  } else {
    const int DROP_ELEMENT = blockedInElement == -1 ?
                             path.size() - 1 : blockedInElement;
    const Pos dropPos = path.at(DROP_ELEMENT);
    const Matl matlAtDropPos =
      Map::cells[dropPos.x][dropPos.y].rigid->getMatl();

    bool isMakingNoise = false;

    switch(matlAtDropPos) {
      case Matl::empty:   isMakingNoise = false;  break;
      case Matl::stone:   isMakingNoise = true;   break;
      case Matl::metal:   isMakingNoise = true;   break;
      case Matl::plant:   isMakingNoise = false;  break;
      case Matl::wood:    isMakingNoise = true;   break;
      case Matl::cloth:   isMakingNoise = false;  break;
      case Matl::fluid:   isMakingNoise = false;  break;
    }

    if(isMakingNoise) {
      const AlertsMonsters alertsMonsters = &actorThrowing == Map::player ?
                                            AlertsMonsters::yes :
                                            AlertsMonsters::no;
      if(!isActorHit) {
        Snd snd(itemThrownData.landOnHardSurfaceSoundMsg,
                itemThrownData.landOnHardSurfaceSfx,
                IgnoreMsgIfOriginSeen::yes, dropPos, nullptr, SndVol::low,
                alertsMonsters);
        SndEmit::emitSnd(snd);
      }
    }
    ItemDrop::dropItemOnMap(dropPos, itemThrown);
  }

  delete data;
  Render::drawMapAndInterface();
  GameTime::actorDidAct();
}

} //Throwing
