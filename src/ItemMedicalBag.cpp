#include "ItemMedicalBag.h"

#include <vector>

#include "Init.h"
#include "Properties.h"
#include "ActorPlayer.h"
#include "Log.h"
#include "Renderer.h"
#include "Inventory.h"
#include "PlayerBon.h"
#include "Popup.h"
#include "GameTime.h"
#include "MapParsing.h"
#include "Map.h"
#include "Query.h"
#include "Input.h"

using namespace std;

const int NR_TRN_BEFORE_HEAL  = 10;
const int NR_TRN_PER_HP       = 2;

ConsumeItem MedicalBag::activateDefault(Actor* const actor) {
  (void)actor;

  vector<Actor*> spottedEnemies;
  Map::player->getSpottedEnemies(spottedEnemies);
  if(!spottedEnemies.empty()) {
    Log::addMsg("Not while an enemy is near.");
    curAction_ = MedBagAction::END;
    return ConsumeItem::no;
  }

  curAction_ = playerChooseAction();

  Log::clearLog();

  if(curAction_ == MedBagAction::END) {return ConsumeItem::no;}

  //Check if chosen action can be done
  vector<PropId> props;
  Map::player->getPropHandler().getAllActivePropIds(props);
  switch(curAction_) {
    case MedBagAction::treatWounds: {
      if(Map::player->getHp() >= Map::player->getHpMax(true)) {
        Log::addMsg("I have no wounds to treat.");
        curAction_ = MedBagAction::END;
        return ConsumeItem::no;
      }
    } break;

    case MedBagAction::sanitizeInfection: {
      if(find(begin(props), end(props), propInfected) == end(props)) {
        Log::addMsg("I have no infection to sanitize.");
        curAction_ = MedBagAction::END;
        return ConsumeItem::no;
      }
    } break;

//      case MedBagAction::takeMorphine: {
//        if(Map::player->getHp() >= Map::player->getHpMax(true)) {
//          Log::addMsg("I am not in pain.");
//          curAction_ = MedBagAction::END;
//        }
//      } break;

    case MedBagAction::END: {} break;
  }

  bool isEnoughSuppl = true;

  switch(curAction_) {
    case MedBagAction::sanitizeInfection: {
      isEnoughSuppl = getTotSupplForSanitize() <= nrSupplies_;
    } break;

    case MedBagAction::treatWounds: //Costs one supply per turn
    case MedBagAction::END: {} break;
  }

  if(!isEnoughSuppl) {
    Log::addMsg("I do not have enough supplies for that.");
    curAction_ = MedBagAction::END;
    return ConsumeItem::no;
  }

  //Action can be done
  Map::player->activeMedicalBag = this;

  switch(curAction_) {
    case MedBagAction::treatWounds: {
      Log::addMsg("I start to treat my wounds...");
      nrTurnsUntilHealWounds_ = NR_TRN_BEFORE_HEAL;
    } break;

    case MedBagAction::sanitizeInfection: {
      Log::addMsg("I start to sanitize an infection...");
      nrTurnsLeftSanitize_ = getTotTurnsForSanitize();
    } break;

//        case MedBagAction::takeMorphine: {
//          Log::addMsg("I start to take Morphine...");
//        } break;

    case MedBagAction::END: {} break;
  }

  GameTime::actorDidAct();

  return ConsumeItem::no;
}

MedBagAction MedicalBag::playerChooseAction() const {

  Log::clearLog();

  Log::addMsg("Use Medical Bag how? [h/enter] Treat wounds [s] Sanitize infection",
              clrWhiteHigh);

//  int suppl = getTotSuppliesFor(MedBagAction::treatWounds);
//  int turns = getTotTurnsFor(MedBagAction::treatWounds);
//  Log::addMsg("[h/enter] Treat wounds (" + toStr(suppl) + "," + toStr(turns) + ")",
//              clrWhiteHigh);

//  suppl     = getTotSuppliesFor(MedBagAction::sanitizeInfection);
//  turns     = getTotTurnsFor(MedBagAction::sanitizeInfection);
//  Log::addMsg("[s] Sanitize infection (" + toStr(suppl) + "," + toStr(turns) + ")",
//              clrWhiteHigh);

  Renderer::drawMapAndInterface(true);

  while(true) {
    const KeyData d = Query::letter(true);
    if(d.sdlKey == SDLK_ESCAPE || d.sdlKey == SDLK_SPACE) {
      return MedBagAction::END;
    } else if(d.sdlKey == SDLK_RETURN || d.key == 'h') {
      return MedBagAction::treatWounds;
    } else if(d.key == 's') {
      return MedBagAction::sanitizeInfection;
    }
  }

  return MedBagAction(MedBagAction::END);
}

void MedicalBag::continueAction() {
  switch(curAction_) {
    case MedBagAction::treatWounds: {

      auto& player = *Map::player;

      const bool IS_HEALER = PlayerBon::hasTrait(Trait::healer);

      if(nrTurnsUntilHealWounds_ > 0) {
        nrTurnsUntilHealWounds_ -= IS_HEALER ? 2 : 1;
      } else {
        //If player is healer, double the rate of HP healing.
        const int NR_TRN_PER_HP_W_BON = IS_HEALER ? (NR_TRN_PER_HP / 2) : NR_TRN_PER_HP;

        if(GameTime::getTurn() % NR_TRN_PER_HP_W_BON == 0) {
          player.restoreHp(1, false);
        }

        //The rate of supply use is consistent (effectively, this means that with the
        //healer trait, you spend half the time and supplies, as the description says).
        if(GameTime::getTurn() % NR_TRN_PER_HP == 0) {
          --nrSupplies_;
        }
      }

      if(nrSupplies_ <= 0) {
        Log::addMsg("No more medical supplies.");
        finishCurAction();
        return;
      }

      if(player.getHp() >= player.getHpMax(true)) {
        finishCurAction();
        return;
      }

      GameTime::actorDidAct();

    } break;

    case MedBagAction::sanitizeInfection: {
      --nrTurnsLeftSanitize_;
      if(nrTurnsLeftSanitize_ <= 0) {
        finishCurAction();
      } else {
        GameTime::actorDidAct();
      }
    } break;

    case MedBagAction::END: {
      assert(false && "Illegal action");
    } break;
  }
}

void MedicalBag::finishCurAction() {
  Map::player->activeMedicalBag = nullptr;

  switch(curAction_) {
    case MedBagAction::sanitizeInfection: {
      bool visionBlockers[MAP_W][MAP_H];
      MapParse::parse(CellPred::BlocksVision(), visionBlockers);
      Map::player->getPropHandler().endAppliedProp(propInfected, visionBlockers);
      nrSupplies_ -= getTotSupplForSanitize();
    } break;

    case MedBagAction::treatWounds: {
      Log::addMsg("I finish treating my wounds.");
    } break;

//    case MedBagAction::takeMorphine: {
//      Map::player->restoreHp(999);
//      Log::addMsg("The morphine takes a toll on my mind.");
//      Map::player->incrShock(ShockValue::heavy, ShockSrc::misc);
//    } break;

    case MedBagAction::END: {} break;
  }

  curAction_ = MedBagAction::END;

  if(nrSupplies_ <= 0) {
    Map::player->getInv().removeItemInGeneralWithPointer(this, true);
  }
}

void MedicalBag::interrupted() {
  Log::addMsg("My healing is disrupted.", clrWhite, false);

  nrTurnsUntilHealWounds_ = -1;
  nrTurnsLeftSanitize_    = -1;

  Map::player->activeMedicalBag = nullptr;
}

int MedicalBag::getTotTurnsForSanitize() const {
  return PlayerBon::hasTrait(Trait::healer) ? 10 : 20;
}

int MedicalBag::getTotSupplForSanitize() const {
  return PlayerBon::hasTrait(Trait::healer) ? 5 : 10;
}
