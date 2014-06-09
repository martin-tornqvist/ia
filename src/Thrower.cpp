#include "Thrower.h"

#include <vector>

#include "Item.h"
#include "ItemPotion.h"
#include "ActorData.h"
#include "ActorPlayer.h"
#include "Renderer.h"
#include "Map.h"
#include "Log.h"
#include "FeatureFactory.h"
#include "Explosion.h"
#include "ItemDrop.h"
#include "Inventory.h"
#include "ItemFactory.h"
#include "Attack.h"
#include "LineCalc.h"
#include "PlayerBon.h"
#include "Utils.h"
#include "SdlWrapper.h"

using namespace std;

namespace Throwing {

void playerThrowLitExplosive(const Pos& aimCell) {
  const int DYNAMITE_FUSE = Map::player->dynamiteFuseTurns;
  const int FLARE_FUSE = Map::player->flareFuseTurns;

  Map::player->explosiveThrown();

  vector<Pos> path;
  LineCalc::calcNewLine(Map::player->pos, aimCell, true,
                        THROWING_RANGE_LIMIT, false, path);

  //Remove cells after blocked cells
  for(size_t i = 1; i < path.size(); i++) {
    const Pos curPos = path.at(i);
    const auto* featureHere = Map::cells[curPos.x][curPos.y].featureStatic;
    if(!featureHere->isProjectilePassable()) {
      path.resize(i);
      break;
    }
  }

  //Render
  if(path.size() > 1) {
    const auto GLYPH = ItemData::data[int(ItemId::dynamite)]->glyph;
    SDL_Color clr = DYNAMITE_FUSE != -1 ? clrRedLgt : clrYellow;
    for(size_t i = 1; i < path.size() - 1; i++) {
      Renderer::drawMapAndInterface(false);
      if(Map::cells[path[i].x][path[i].y].isSeenByPlayer) {
        Renderer::drawGlyph(GLYPH, Panel::map, path[i], clr);
        Renderer::updateScreen();
        SdlWrapper::sleep(Config::getDelayProjectileDraw());
      }
    }
  }

  auto* const featureAtDest =
    Map::cells[path.back().x][path.back().y].featureStatic;
  const bool IS_DEST_FEAT_BOTTOMLESS = featureAtDest->isBottomless();

  if(DYNAMITE_FUSE != -1) {
    Log::addMsg("I throw a lit dynamite stick.");
    if(!IS_DEST_FEAT_BOTTOMLESS) {
      FeatureFactory::mk(FeatureId::litDynamite, path.back(),
                         new DynamiteSpawnData(DYNAMITE_FUSE));
    }
  } else if(FLARE_FUSE != -1) {
    Log::addMsg("I throw a lit flare.");
    if(!IS_DEST_FEAT_BOTTOMLESS) {
      FeatureFactory::mk(FeatureId::litFlare, path.back(),
                         new DynamiteSpawnData(FLARE_FUSE));
    }
    GameTime::updateLightMap();
    Map::player->updateFov();
    Renderer::drawMapAndInterface();
  } else {
    Log::addMsg("I throw a lit Molotov Cocktail.");
    const int EXPL_RADI_CHANGE =
      PlayerBon::hasTrait(Trait::demolitionExpert) ? 1 : 0;
    if(!IS_DEST_FEAT_BOTTOMLESS) {
      Explosion::runExplosionAt(
        path.back(), ExplType::applyProp,
        ExplSrc::playerUseMoltvIntended, EXPL_RADI_CHANGE,
        SfxId::explosionMolotov, new PropBurning(propTurnsStd));
    }
  }

  GameTime::actorDidAct();
}

void throwItem(Actor& actorThrowing, const Pos& targetCell, Item& itemThrown) {
  MissileAttData* data = new MissileAttData(
    actorThrowing, itemThrown, targetCell, actorThrowing.pos);

  const ActorSize aimLvl = data->intendedAimLvl;

  vector<Pos> path;
  LineCalc::calcNewLine(actorThrowing.pos, targetCell, false,
                        THROWING_RANGE_LIMIT, false, path);

  const ItemDataT& itemThrownData = itemThrown.getData();

  const string itemName_a =
    ItemData::getItemRef(itemThrown, ItemRefType::a, true);
  if(&actorThrowing == Map::player) {
    Log::clearLog();
    Log::addMsg("I throw " + itemName_a + ".");
  } else {
    const Pos& p = path.front();
    if(Map::cells[p.x][p.y].isSeenByPlayer) {
      Log::addMsg(
        actorThrowing.getNameThe() + " throws " + itemName_a + ".");
    }
  }
  Renderer::drawMapAndInterface(true);

  int blockedInElement = -1;
  bool isActorHit = false;

  const char      glyph = itemThrown.getGlyph();
  const SDL_Color clr   = itemThrown.getClr();

  int chanceToDestroyItem = 0;

  Pos curPos(-1, -1);

  for(unsigned int i = 1; i < path.size(); i++) {
    Renderer::drawMapAndInterface(false);

    curPos.set(path.at(i));

    Actor* const actorHere = Utils::getActorAtPos(curPos);
    if(actorHere) {
      if(
        curPos == targetCell ||
        actorHere->getData().actorSize >= actorSize_humanoid) {

        delete data;
        data = new MissileAttData(actorThrowing, itemThrown, targetCell,
                                  curPos, aimLvl);

        if(
          data->attackResult >= successSmall &&
          !data->isEtherealDefenderMissed) {
          if(Map::cells[curPos.x][curPos.y].isSeenByPlayer) {
            Renderer::drawGlyph('*', Panel::map,
                                curPos, clrRedLgt);
            Renderer::updateScreen();
            SdlWrapper::sleep(Config::getDelayProjectileDraw() * 4);
          }
          const SDL_Color hitMessageClr =
            actorHere == Map::player ? clrMsgBad : clrMsgGood;

          const bool CAN_SEE_ACTOR =
            Map::player->isSeeingActor(*actorHere, nullptr);
          string defenderName = CAN_SEE_ACTOR ? actorHere->getNameThe() : "It";

          Log::addMsg(defenderName + " is hit.", hitMessageClr);

          actorHere->hit(data->dmg, DmgType::physical, true);
          isActorHit = true;

          //If throwing a potion on an actor, let it make stuff happen...
          if(itemThrownData.isPotion) {
            dynamic_cast<Potion*>(&itemThrown)->collide(curPos, actorHere);
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
      Renderer::drawGlyph(glyph, Panel::map, curPos, clr);
      Renderer::updateScreen();
      SdlWrapper::sleep(Config::getDelayProjectileDraw());
    }

    const auto* featureHere = Map::cells[curPos.x][curPos.y].featureStatic;
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
      dynamic_cast<Potion*>(&itemThrown)->collide(
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
    const MaterialType materialAtDropPos =
      Map::cells[dropPos.x][dropPos.y].featureStatic->getMaterialType();
    if(materialAtDropPos == materialType_hard) {
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
  Renderer::drawMapAndInterface();
  GameTime::actorDidAct();
}

} //Throwing
