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

using namespace std;

ConsumeItem MedicalBag::activateDefault(Actor* const actor) {
  (void)actor;

  vector<Actor*> spottedEnemies;
  Map::player->getSpottedEnemies(spottedEnemies);
  if(!spottedEnemies.empty()) {
    Log::addMsg("Not while an enemy is near.");
    return ConsumeItem::no;
  }

  curAction_ = playerChooseAction();

  if(curAction_ != endOfMedicalBagActions) {
    //Check if chosen action can be done
    vector<PropId> props;
    Map::player->getPropHandler().getAllActivePropIds(props);
    switch(curAction_) {
      case medicalBagAction_sanitizeInfection: {
        if(find(begin(props), end(props), propInfected) == end(props)) {
          Log::addMsg("I have no infections to sanitize.");
          curAction_ = endOfMedicalBagActions;
        }
      } break;

      case medicalBagAction_takeMorphine: {
        if(Map::player->getHp() >= Map::player->getHpMax(true)) {
          Log::addMsg("I am not in pain.");
          curAction_ = endOfMedicalBagActions;
        }
      } break;

      case medicalBagAction_treatWound: {
        if(find(begin(props), end(props), propWound) == end(props)) {
          Log::addMsg("I have no wounds to treat.");
          curAction_ = endOfMedicalBagActions;
        }
      } break;

      case endOfMedicalBagActions: {} break;
    }

    if(curAction_ != endOfMedicalBagActions) {
      if(getNrSuppliesNeededForAction(curAction_) > nrSupplies_) {
        Log::addMsg("I do not have enough supplies for that.");
        curAction_ = endOfMedicalBagActions;
      }
    }

    if(curAction_ != endOfMedicalBagActions) {
      //Action can be done
      nrTurnsLeft_ = getTotTurnsForAction(curAction_);
      Map::player->activeMedicalBag = this;

      switch(curAction_) {
        case medicalBagAction_sanitizeInfection: {
          Log::addMsg("I start to sanitize an infection...");
        } break;

        case medicalBagAction_takeMorphine: {
          Log::addMsg("I start to take Morphine...");
        } break;

        case medicalBagAction_treatWound: {
          Log::addMsg("I start to treat a wound...");
        } break;

        case endOfMedicalBagActions: {} break;
      }

      GameTime::actorDidAct();
    }
  }

  return ConsumeItem::no;
}

MedicalBagAction MedicalBag::playerChooseAction() const {
  vector<string> choiceLabels;
  for(int actionNr = 0; actionNr < endOfMedicalBagActions; actionNr++) {
    string label = "";
    switch(MedicalBagAction(actionNr)) {
      case medicalBagAction_sanitizeInfection: {
        label = "Sanitize infection";
      } break;

      case medicalBagAction_takeMorphine: {
        label = "Take morphine";
      } break;

      case medicalBagAction_treatWound: {
        label = "Treat wound";
      } break;

      case endOfMedicalBagActions: {} break;
    }

    const int NR_TURNS_NEEDED =
      getTotTurnsForAction(MedicalBagAction(actionNr));
    const int NR_SUPPL_NEEDED =
      getNrSuppliesNeededForAction(MedicalBagAction(actionNr));
    label += " (" + toStr(NR_SUPPL_NEEDED) + " suppl";
    label += "/"  + toStr(NR_TURNS_NEEDED) + " turns)";
    choiceLabels.push_back(label);
  }
  choiceLabels.push_back("Cancel");

  const string suppliesMsg =
    toStr(nrSupplies_) + " medical supplies available.";

  const int CHOICE_NR =
    Popup::showMenuMsg(suppliesMsg, true, choiceLabels, "Use medical bag");
  return MedicalBagAction(CHOICE_NR);
}

void MedicalBag::continueAction() {
  nrTurnsLeft_--;
  if(nrTurnsLeft_ <= 0) {
    finishCurAction();
  } else {
    GameTime::actorDidAct();
  }
}

void MedicalBag::finishCurAction() {
  Map::player->activeMedicalBag = nullptr;

  switch(curAction_) {
    case medicalBagAction_sanitizeInfection: {
      bool visionBlockers[MAP_W][MAP_H];
      MapParse::parse(CellPred::BlocksVision(), visionBlockers);
      Map::player->getPropHandler().endAppliedProp(
        propInfected, visionBlockers);
    } break;

    case medicalBagAction_treatWound: {
      Prop* prop =
        Map::player->getPropHandler().getProp(propWound, PropSrc::applied);
      if(prop) {
        static_cast<PropWound*>(prop)->healOneWound();
      } else {
        TRACE << "[WARNING] No wound prop found, ";
        TRACE << "in MedicalBag::finishCurAction()" << endl;
      }
    } break;

    case medicalBagAction_takeMorphine: {
      Map::player->restoreHp(999);
      Log::addMsg("The morphine takes a toll on my mind.");
      Map::player->incrShock(ShockValue::shockValue_heavy, ShockSrc::misc);
    } break;

    case endOfMedicalBagActions: {} break;
  }

  nrSupplies_ -= getNrSuppliesNeededForAction(curAction_);

  curAction_ = endOfMedicalBagActions;

  if(nrSupplies_ <= 0) {
    Map::player->getInv().removetemInGeneralWithPointer(this, true);
  }
}

void MedicalBag::interrupted() {
  Log::addMsg("My healing is disrupted.", clrWhite, false);

  nrTurnsLeft_ = -1;

  Map::player->activeMedicalBag = nullptr;
}

int MedicalBag::getTotTurnsForAction(const MedicalBagAction action) const {
  const bool IS_HEALER =
    PlayerBon::hasTrait(Trait::healer);

  switch(action) {
    case medicalBagAction_sanitizeInfection: {
      return 20 / (IS_HEALER ? 2 : 1);
    } break;

    case medicalBagAction_treatWound: {
      return 70 / (IS_HEALER ? 2 : 1);
    } break;

    case medicalBagAction_takeMorphine: {
      return 8 / (IS_HEALER ? 2 : 1);
    } break;

    case endOfMedicalBagActions: {} break;
  }
  return -1;
}

int MedicalBag::getNrSuppliesNeededForAction(
  const MedicalBagAction action) const {

  const bool IS_HEALER =
    PlayerBon::hasTrait(Trait::healer);

  switch(action) {
    case medicalBagAction_sanitizeInfection: {
      return 4 / (IS_HEALER ? 2 : 1);
    } break;

    case medicalBagAction_treatWound: {
      return 10 / (IS_HEALER ? 2 : 1);
    } break;

    case medicalBagAction_takeMorphine: {
      return 4 / (IS_HEALER ? 2 : 1);
    } break;

    case endOfMedicalBagActions: {} break;
  }
  return -1;
}
