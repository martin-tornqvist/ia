#include "ItemMedicalBag.h"

#include "Engine.h"
#include "StatusEffects.h"
#include "ActorPlayer.h"
#include "Log.h"
#include "Render.h"
#include "Inventory.h"
#include "PlayerBonuses.h"
#include "Popup.h"

bool MedicalBag::activateDefault(Actor* const actor,
                                 Engine* const engine) {
  (void)actor;

  curAction_ = playerChooseAction(engine);

  if(curAction_ != endOfMedicalBagActions) {
    //Check if chosen action can be done
    const StatusHandler* const status =
      engine->player->getStatusHandler();
    switch(curAction_) {
      case medicalBagAction_sanitizeInfection: {
        if(status->hasEffect(statusInfected) == false) {
          engine->log->addMessage("I have no infections to sanitize.");
          curAction_ = endOfMedicalBagActions;
        }
      } break;

      case medicalBagAction_treatWound: {
        if(status->hasEffect(statusWound) == false) {
          engine->log->addMessage("I have no wounds to treat.");
          curAction_ = endOfMedicalBagActions;
        }
      } break;

      case endOfMedicalBagActions: {} break;
    }

    if(curAction_ != endOfMedicalBagActions) {
      if(getNrSuppliesNeededForAction(curAction_, engine) > nrSupplies_) {
        engine->log->addMessage("I do not have enough supplies for that.");
        curAction_ = endOfMedicalBagActions;
      }
    }

    if(curAction_ != endOfMedicalBagActions) {
      //Action can be done
      nrTurnsLeft_ = getTotTurnsForAction(curAction_, engine);
      engine->player->activeMedicalBag = this;
      engine->gameTime->endTurnOfCurrentActor();
    }
  }

  return false;
}

MedicalBagAction_t MedicalBag::playerChooseAction(Engine* const engine) const {
  vector<string> choiceLabels;
  for(int actionNr = 0; actionNr < endOfMedicalBagActions; actionNr++) {
    string label = "";
    switch(actionNr) {
      case medicalBagAction_sanitizeInfection: {
        label = "Sanitize infection";
      } break;

      case medicalBagAction_treatWound: {
        label = "Treat wound";
      } break;
    }

    const int NR_TURNS_NEEDED =
      getTotTurnsForAction(MedicalBagAction_t(actionNr), engine);
    const int NR_SUPPL_NEEDED =
      getNrSuppliesNeededForAction(MedicalBagAction_t(actionNr), engine);
    label += " (" + intToString(NR_SUPPL_NEEDED) + " suppl";
    label += "/"  + intToString(NR_TURNS_NEEDED) + " turns)";
    choiceLabels.push_back(label);
  }
  choiceLabels.push_back("Cancel");

  const string nrSuppliesMsg =
    intToString(nrSupplies_) + " medical supplies available.";

  return MedicalBagAction_t(engine->popup->showMultiChoiceMessage(
                              nrSuppliesMsg, true, choiceLabels,
                              "Use medical bag"));
}

void MedicalBag::continueAction(Engine* const engine) {
  nrTurnsLeft_--;
  if(nrTurnsLeft_ <= 0) {
    finishCurAction(engine);
  } else {
    engine->gameTime->endTurnOfCurrentActor();
  }
}

void MedicalBag::finishCurAction(Engine* const engine) {
  engine->player->activeMedicalBag = NULL;

  switch(curAction_) {
    case medicalBagAction_sanitizeInfection: {
    } break;

    case medicalBagAction_treatWound: {
      StatusEffect* effect =
        engine->player->getStatusHandler()->getEffect(statusWound);
      if(effect == NULL) {
        tracer << "[WARNING] No wound status effect found, ";
        tracer << "in MedicalBag::finishCurAction()" << endl;
      } else {
        StatusWound* wound = dynamic_cast<StatusWound*>(effect);
        wound->healOneWound(engine);
      }
//        engine->log->clearLog();
//        engine->log->addMessage("I finish applying first aid.");
//        engine->renderer->drawMapAndInterface();
    } break;

    case endOfMedicalBagActions: {} break;
  }

  nrSupplies_ -= getNrSuppliesNeededForAction(curAction_, engine);

  curAction_ = endOfMedicalBagActions;

  if(nrSupplies_ <= 0) {
    Inventory* const inv = engine->player->getInventory();
    inv->removetemInGeneralWithPointer(this, true);
  }
}

void MedicalBag::interrupted(Engine* const engine) {
//  switch(curAction_) {
//    case medicalBagAction_sanitizeInfection: {
//    } break;
//
//    case medicalBagAction_treatWound: {
//    } break;
//  }
//
//  engine->log->addMessage("My applying of first aid is disrupted.", clrWhite,
//                          messageInterrupt_never);
//  nrTurnsLeft_ = -1;
//
//  engine->player->activeMedicalBag = NULL;
}

int MedicalBag::getTotTurnsForAction(const MedicalBagAction_t action,
                                     Engine* const engine) const {
  const bool IS_WOUND_TREATER =
    engine->playerBonHandler->isBonPicked(playerBon_skillfulWoundTreater);

  switch(action) {
    case medicalBagAction_sanitizeInfection: {
      const int TURNS_BEFORE_BON = 20;
      return IS_WOUND_TREATER ? TURNS_BEFORE_BON / 2 : TURNS_BEFORE_BON;
    } break;

    case medicalBagAction_treatWound: {
      const int TURNS_BEFORE_BON = 70;
      return IS_WOUND_TREATER ? TURNS_BEFORE_BON / 2 : TURNS_BEFORE_BON;
    } break;

    case endOfMedicalBagActions: {
      //Should not happen
    } break;
  }
  return -1;
}

int MedicalBag::getNrSuppliesNeededForAction(const MedicalBagAction_t action,
    Engine* const engine) const {
  switch(action) {
    case medicalBagAction_sanitizeInfection: {
      return 1;
    } break;

    case medicalBagAction_treatWound: {
      return 5;
    } break;

    case endOfMedicalBagActions: {
      //Should not happen
    } break;
  }
  return -1;
}

// (Interrupted)
//const bool IS_FAINTED = statusEffectsHandler_->hasEffect(statusFainted);
//const bool IS_PARALYSED = statusEffectsHandler_->hasEffect(statusParalyzed);
//const bool IS_DEAD = deadState != actorDeadState_alive;
//getSpotedEnemies();
//const int TOTAL_TURNS = getHealingTimeTotal();
//const bool IS_ENOUGH_TIME_PASSED = firstAidTurnsLeft < TOTAL_TURNS - 10;
//const int MISSING_HP = getHpMax(true) - getHp();
//const int HP_HEALED_IF_ABORTED =
//  IS_ENOUGH_TIME_PASSED ? (MISSING_HP * (TOTAL_TURNS - firstAidTurnsLeft)) / TOTAL_TURNS  : 0;
//
//bool isAborted = false;
//if(spotedEnemies.size() > 0 || IS_FAINTED || IS_PARALYSED || IS_DEAD || PROMPT_FOR_ABORT == false) {
//  firstAidTurnsLeft = -1;
//  isAborted = true;
//  eng->log->addMessage("I stop tending to my wounds.", clrWhite);
//  eng->renderer->drawMapAndInterface();
//} else {
//  const string TURNS_STR = intToString(firstAidTurnsLeft);
//  const string ABORTED_HP_STR = intToString(HP_HEALED_IF_ABORTED);
//  string abortStr = "Continue healing (" + TURNS_STR + " turns)? (y/n), ";
//  abortStr += ABORTED_HP_STR + " HP restored if canceled.";
//  eng->log->addMessage(abortStr , clrWhiteHigh);
//  eng->renderer->drawMapAndInterface();
//
//  if(eng->query->yesOrNo() == false) {
//    firstAidTurnsLeft = -1;
//    isAborted = true;
//  }
//
//  eng->log->clearLog();
//  eng->renderer->drawMapAndInterface();
//}
//if(isAborted && IS_ENOUGH_TIME_PASSED) {
//  restoreHP(HP_HEALED_IF_ABORTED);
//}


// (Request healing)
//    clearLogMessages();
//    if(eng->player->deadState == actorDeadState_alive) {
//      if(eng->player->getStatusHandler()->hasEffect(statusPoisoned)) {
//        eng->log->addMessage("Not while poisoned.");
//        eng->renderer->drawMapAndInterface();
//      } else {
//        bool allowHeal = false;
//        const bool IS_DISEASED = eng->player->getStatusHandler()->hasEffect(statusDiseased);
//
//        if(eng->player->getHp() < eng->player->getHpMax(true)) {
//          allowHeal = true;
//        } else if(IS_DISEASED && eng->playerBonHandler->isBonPicked(playerBon_curer)) {
//          allowHeal = true;
//        }
//
//        if(allowHeal) {
//          eng->player->getSpotedEnemies();
//          if(eng->player->spotedEnemies.size() == 0) {
//            const int TURNS_TO_HEAL = eng->player->getHealingTimeTotal();
//            const string TURNS_STR = intToString(TURNS_TO_HEAL);
//            eng->log->addMessage("I rest here and attend my wounds (" + TURNS_STR + " turns)...");
//            eng->player->firstAidTurnsLeft = TURNS_TO_HEAL - 1;
//            eng->gameTime->endTurnOfCurrentActor();
//          } else {
//            eng->log->addMessage("Not while an enemy is near.");
//            eng->renderer->drawMapAndInterface();
//          }
//        } else {
//          if(IS_DISEASED) {
//            eng->log->addMessage("I cannot heal this disease.");
//          } else {
//            eng->log->addMessage("I am already at good health.");
//          }
//          eng->renderer->drawMapAndInterface();
//        }
//      }
//    }
//    clearEvents();
//    return;
