#include "FeatureTrap.h"

#include <algorithm>
#include <assert.h>

#include "Engine.h"

#include "FeatureFactory.h"
#include "FeatureData.h"
#include "ActorPlayer.h"
#include "Log.h"
#include "Map.h"
#include "ItemDrop.h"
#include "Postmortem.h"
#include "Explosion.h"
#include "Popup.h"
#include "ActorMonster.h"
#include "Inventory.h"
#include "Sound.h"
#include "ActorFactory.h"
#include "Renderer.h"

Trap::Trap(Feature_t id, Pos pos, Engine& engine, TrapSpawnData* spawnData) :
  FeatureStatic(id, pos, engine), mimicFeature_(spawnData->mimicFeature_),
  isHidden_(true) {

  assert(spawnData->trapType_ != endOfTraps);
  assert(mimicFeature_ != NULL);

  if(spawnData->trapType_ == trap_any) {
    setSpecificTrapFromId(Trap_t(eng.dice.range(0, endOfTraps - 1)));
  } else {
    setSpecificTrapFromId(spawnData->trapType_);
  }
  assert(specificTrap_ != NULL);
}

Trap::~Trap() {
  assert(specificTrap_ != NULL);
  delete specificTrap_;
}

void Trap::setSpecificTrapFromId(const Trap_t id) {
  switch(id) {
    case trap_dart:           specificTrap_ = new TrapDart(pos_, eng);            break;
    case trap_spear:          specificTrap_ = new TrapSpear(pos_, eng);           break;
    case trap_gasConfusion:   specificTrap_ = new TrapGasConfusion(pos_, eng);    break;
    case trap_gasParalyze:    specificTrap_ = new TrapGasParalyzation(pos_, eng); break;
    case trap_gasFear:        specificTrap_ = new TrapGasFear(pos_, eng);         break;
    case trap_blinding:       specificTrap_ = new TrapBlindingFlash(pos_, eng);   break;
    case trap_teleport:       specificTrap_ = new TrapTeleport(pos_, eng);        break;
    case trap_summonMonster:  specificTrap_ = new TrapSummonMonster(pos_, eng);   break;
    case trap_smoke:          specificTrap_ = new TrapSmoke(pos_, eng);           break;
    case trap_alarm:          specificTrap_ = new TrapAlarm(pos_, eng);           break;
    case trap_spiderWeb:      specificTrap_ = new TrapSpiderWeb(pos_, eng);       break;
    default:                  specificTrap_ = NULL;                               break;
  }
}

Trap_t Trap::getTrapType() const {return specificTrap_->trapType_;}

bool Trap::isMagical() const {return specificTrap_->isMagical();}

bool Trap::isDisarmable() const {return specificTrap_->isDisarmable();}

void Trap::triggerOnPurpose(Actor& actorTriggering) {
  const AbilityRollResult_t DODGE_RESULT = failSmall;
  specificTrap_->trapSpecificTrigger(actorTriggering, DODGE_RESULT);
}

void Trap::bump(Actor& actorBumping) {
  trace << "Trap::bump()..." << endl;

  const ActorData* const d = actorBumping.getData();

  trace << "Trap: Name of actor bumping: \"" << d->name_a << "\"" << endl;

  if(d->bodyType == bodyType_normal) {
    const bool IS_PLAYER = &actorBumping == actorBumping.eng.player;
    const bool ACTOR_CAN_SEE = actorBumping.getPropHandler()->allowSee();
    AbilityValues& abilities = actorBumping.getData()->abilityVals;
    const int DODGE_SKILL_VALUE =
      abilities.getVal(ability_dodgeTrap, true, actorBumping);
    const int BASE_CHANCE_TO_AVOID = 30;

    const string trapName = specificTrap_->getTrapSpecificTitle();

    if(IS_PLAYER) {
      trace << "Trap: Player bumping" << endl;
      const int CHANCE_TO_AVOID =
        isHidden_ == true ? 10 :
        (BASE_CHANCE_TO_AVOID + DODGE_SKILL_VALUE);
      const AbilityRollResult_t result =
        actorBumping.eng.abilityRoll->roll(CHANCE_TO_AVOID);

      if(result >= successSmall) {
        if(isHidden_ == false) {
          if(ACTOR_CAN_SEE) {
            actorBumping.eng.log->addMsg(
              "I avoid a " + trapName + ".", clrMessageGood);
          }
        }
      } else {
        trigger(actorBumping);
      }
    } else {
      if(d->actorSize == actorSize_humanoid && d->isSpider == false) {
        trace << "Trap: Humanoid monster bumping" << endl;
        Monster* const monster = dynamic_cast<Monster*>(&actorBumping);
        if(
          monster->playerAwarenessCounter > 0 &&
          monster->isStealth == false) {
          trace << "Trap: Monster eligible for triggering trap" << endl;

          const bool IS_ACTOR_SEEN_BY_PLAYER =
            actorBumping.eng.player->checkIfSeeActor(actorBumping, NULL);

          const int CHANCE_TO_AVOID =
            BASE_CHANCE_TO_AVOID + d->canDodge ? DODGE_SKILL_VALUE : 0;
          const AbilityRollResult_t result =
            actorBumping.eng.abilityRoll->roll(CHANCE_TO_AVOID);

          if(result >= successSmall) {
            if(isHidden_ == false && IS_ACTOR_SEEN_BY_PLAYER) {
              const string actorName = actorBumping.getNameThe();
              actorBumping.eng.log->addMsg(
                actorName + " avoids a " + trapName + ".");
            }
          } else {
            trigger(actorBumping);
          }
        }
      }
    }
  }
  trace << "Trap::bump() [DONE]" << endl;
}

void Trap::trigger(Actor& actor) {
  trace << "Trap::trigger()..." << endl;

  trace << "Trap: Specific trap is ";
  trace << specificTrap_->getTrapSpecificTitle() << endl;

  const ActorData* const d = actor.getData();

  trace << "Trap: Actor triggering is ";
  trace << d->name_a << endl;

  const int DODGE_SKILL_VALUE =
    d->abilityVals.getVal(ability_dodgeTrap, true, actor);

  trace << "Trap: Actor dodge skill is ";
  trace << DODGE_SKILL_VALUE << endl;

  if(&actor == eng.player) {
    traceVerbose << "Trap: Player triggering trap" << endl;
    const AbilityRollResult_t DODGE_RESULT =
      eng.abilityRoll->roll(DODGE_SKILL_VALUE);
    reveal(false);
    traceVerbose << "Trap: Calling trapSpecificTrigger" << endl;
    specificTrap_->trapSpecificTrigger(actor, DODGE_RESULT);
  } else {
    traceVerbose << "Trap: Monster triggering trap" << endl;
    const bool IS_ACTOR_SEEN_BY_PLAYER =
      eng.player->checkIfSeeActor(actor, NULL);
    const AbilityRollResult_t dodgeResult =
      d->canDodge ? eng.abilityRoll->roll(DODGE_SKILL_VALUE) : failNormal;
    if(IS_ACTOR_SEEN_BY_PLAYER) {
      reveal(false);
    }
    traceVerbose << "Trap: Calling trapSpecificTrigger" << endl;
    specificTrap_->trapSpecificTrigger(actor, dodgeResult);
  }

  trace << "Trap::trigger() [DONE]" << endl;
}

void Trap::reveal(const bool PRINT_MESSSAGE_WHEN_PLAYER_SEES) {
  traceVerbose << "Trap::reveal()..." << endl;
  isHidden_ = false;
  clearGore();

  Item* item = eng.map->cells[pos_.x][pos_.y].item;
  if(item != NULL) {
    eng.map->cells[pos_.x][pos_.y].item = NULL;
    eng.itemDrop->dropItemOnMap(pos_, *item);
  }

  if(eng.map->cells[pos_.x][pos_.y].isSeenByPlayer) {
    eng.renderer->drawMapAndInterface();

    if(PRINT_MESSSAGE_WHEN_PLAYER_SEES) {
      const string name = specificTrap_->getTrapSpecificTitle();
      eng.log->addMsg("I spot a " + name + ".");
    }
  }
  traceVerbose << "Trap::reveal() [DONE]" << endl;
}

void Trap::playerTrySpotHidden() {
  if(isHidden_) {
    if(eng.basicUtils->isPosAdj(pos_, eng.player->pos, false)) {
      const Abilities_t abilityUsed = ability_searching;
      const int PLAYER_SKILL =
        eng.player->getData()->abilityVals.getVal(
          abilityUsed, true, *(eng.player));

      if(eng.abilityRoll->roll(PLAYER_SKILL) >= successSmall) {
        reveal(true);
      }
    }
  }
}

string Trap::getDescription(const bool DEFINITE_ARTICLE) const {
  if(isHidden_) {
    return DEFINITE_ARTICLE == true ? mimicFeature_->name_the :
           mimicFeature_->name_a;
  } else {
    return "a " + specificTrap_->getTrapSpecificTitle();
  }
}

SDL_Color Trap::getColor() const {
  return isHidden_ ? mimicFeature_->color :
         specificTrap_->getTrapSpecificColor();
}

char Trap::getGlyph() const {
  return isHidden_ ? mimicFeature_->glyph :
         specificTrap_->getTrapSpecificGlyph();
}

Tile_t Trap::getTile() const {
  return isHidden_ ? mimicFeature_->tile :
         specificTrap_->getTrapSpecificTile();
}

bool Trap::canHaveCorpse() const {return isHidden_;}
bool Trap::canHaveBlood() const {return isHidden_;}
bool Trap::canHaveGore() const {return isHidden_;}
bool Trap::canHaveItem() const {return isHidden_;}

Dir_t Trap::actorTryLeave(Actor& actor, const Dir_t dir) {
  trace << "Trap::actorTryLeave()" << endl;
  return specificTrap_->specificTrapActorTryLeave(actor, dir);
}

MaterialType_t Trap::getMaterialType() const {
  return isHidden_ ? mimicFeature_->materialType : data_->materialType;
}

//================================================ SPECIFIC TRAPS
TrapDart::TrapDart(Pos pos, Engine& engine) :
  SpecificTrapBase(pos, trap_dart, engine), isPoisoned(false) {
  isPoisoned =
    eng.map->getDlvl() >= MIN_DLVL_NASTY_TRAPS && eng.dice.coinToss();
}

void TrapDart::trapSpecificTrigger(Actor& actor,
                                   const AbilityRollResult_t dodgeResult) {
  traceVerbose << "TrapDart::trapSpecificTrigger()..." << endl;
  const bool IS_PLAYER = &actor == eng.player;
  const bool CAN_SEE = actor.getPropHandler()->allowSee();
  const bool CAN_PLAYER_SEE_ACTOR = eng.player->checkIfSeeActor(actor, NULL);
  const string actorName = actor.getNameThe();

  //Dodge?
  if(dodgeResult >= successSmall) {
    if(IS_PLAYER) {
      if(CAN_SEE) {
        eng.log->addMsg("I dodge a dart!", clrMessageGood);
      } else {
        eng.log->addMsg(
          "I feel a mechanism trigger and quickly leap aside!",
          clrMessageGood);
      }
    } else {
      if(CAN_PLAYER_SEE_ACTOR) {
        eng.log->addMsg(actorName +  " dodges a dart!");
      }
    }
  } else {
    //Trap misses?
    const int CHANCE_TO_HIT = 75;
    if(eng.dice.percentile() > CHANCE_TO_HIT) {
      if(IS_PLAYER) {
        if(CAN_SEE) {
          eng.log->addMsg("A dart barely misses me!", clrMessageGood);
        } else {
          eng.log->addMsg(
            "A mechanism triggers, I hear something barely missing me!",
            clrMessageGood);
        }
      } else if(CAN_PLAYER_SEE_ACTOR) {
        eng.log->addMsg("A dart barely misses " + actorName + "!");
      }
    } else {
      //Dodge failed and trap hits
      if(IS_PLAYER) {
        if(CAN_SEE) {
          eng.log->addMsg("I am hit by a dart!", clrMessageBad);
        } else {
          eng.log->addMsg(
            "A mechanism triggers, I feel a needle piercing my skin!",
            clrMessageBad);
        }
      } else if(CAN_PLAYER_SEE_ACTOR) {
        eng.log->addMsg(actorName + " is hit by a dart!", clrMessageGood);
      }

      const int DMG = eng.dice(1, 8);
      actor.hit(DMG, dmgType_physical, true);
      if(actor.deadState == actorDeadState_alive && isPoisoned) {
        if(IS_PLAYER) {
          eng.log->addMsg("It was poisoned!");
        }
        actor.getPropHandler()->tryApplyProp(
          new PropPoisoned(eng, propTurnsStandard));
      }
    }
  }
  traceVerbose << "TrapDart::trapSpecificTrigger() [DONE]" << endl;
}

TrapSpear::TrapSpear(Pos pos, Engine& engine) :
  SpecificTrapBase(pos, trap_spear, engine), isPoisoned(false) {
  isPoisoned =
    eng.map->getDlvl() >= MIN_DLVL_NASTY_TRAPS && eng.dice.coinToss();
}

void TrapSpear::trapSpecificTrigger(Actor& actor,
                                    const AbilityRollResult_t dodgeResult) {
  traceVerbose << "TrapSpear::trapSpecificTrigger()..." << endl;

  const bool IS_PLAYER = &actor == eng.player;
  const bool CAN_SEE = actor.getPropHandler()->allowSee();
  const bool CAN_PLAYER_SEE_ACTOR = eng.player->checkIfSeeActor(actor, NULL);
  const string actorName = actor.getNameThe();

  //Dodge?
  if(dodgeResult >= successSmall) {
    if(IS_PLAYER) {
      if(CAN_SEE) {
        eng.log->addMsg("I dodge a spear!", clrMessageGood);
      } else {
        eng.log->addMsg(
          "I feel a mechanism trigger and quickly leap aside!",
          clrMessageGood);
      }
    } else {
      if(CAN_PLAYER_SEE_ACTOR) {
        eng.log->addMsg(actorName +  " dodges a spear!");
      }
    }
  } else {
    //Trap misses?
    const int CHANCE_TO_HIT = 75;
    if(eng.dice.percentile() > CHANCE_TO_HIT) {
      if(IS_PLAYER) {
        if(CAN_SEE) {
          eng.log->addMsg("A spear barely misses me!", clrMessageGood);
        } else {
          eng.log->addMsg(
            "A mechanism triggers, I hear a *swoosh*!",
            clrMessageGood);
        }
      } else {
        if(CAN_PLAYER_SEE_ACTOR) {
          eng.log->addMsg("A spear barely misses " + actorName + "!");
        }
      }
    } else {
      //Dodge failed and trap hits
      if(IS_PLAYER) {
        if(CAN_SEE) {
          eng.log->addMsg("I am hit by a spear!", clrMessageBad);
        } else {
          eng.log->addMsg(
            "A mechanism triggers, something sharp pierces my skin!",
            clrMessageBad);
        }
      } else {
        if(CAN_PLAYER_SEE_ACTOR) {
          eng.log->addMsg(
            actorName + " is hit by a spear!", clrMessageGood);
        }
      }

      const int DMG = eng.dice(2, 6);
      actor.hit(DMG, dmgType_physical, true);
      if(actor.deadState == actorDeadState_alive && isPoisoned) {
        if(IS_PLAYER) {
          eng.log->addMsg("It was poisoned!");
        }
        actor.getPropHandler()->tryApplyProp(
          new PropPoisoned(eng, propTurnsStandard));
      }
    }
  }
  traceVerbose << "TrapSpear::trapSpecificTrigger()..." << endl;
}

void TrapGasConfusion::trapSpecificTrigger(Actor& actor,
    const AbilityRollResult_t dodgeResult) {
  traceVerbose << "TrapGasConfusion::trapSpecificTrigger()..." << endl;
  (void)dodgeResult;

  const bool IS_PLAYER = &actor == eng.player;
  const bool CAN_SEE = actor.getPropHandler()->allowSee();
  const bool CAN_PLAYER_SEE_ACTOR = eng.player->checkIfSeeActor(actor, NULL);
  const string actorName = actor.getNameThe();

  if(IS_PLAYER) {
    if(CAN_SEE) {
      eng.log->addMsg("I am hit by a burst of gas!");
    } else {
      eng.log->addMsg(
        "A mechanism triggers, I am hit by a burst of gas!");
    }
  } else {
    if(CAN_PLAYER_SEE_ACTOR) {
      eng.log->addMsg(actorName + " is hit by a burst of gas!");
    }
  }

  eng.explosionMaker->runExplosion(
    pos_, endOfSfx, false, new PropConfused(eng, propTurnsStandard), true,
    getTrapSpecificColor());
  traceVerbose << "TrapGasConfusion::trapSpecificTrigger() [DONE]" << endl;
}

void TrapGasParalyzation::trapSpecificTrigger(Actor& actor,
    const AbilityRollResult_t dodgeResult) {
  traceVerbose << "TrapGasParalyzation::trapSpecificTrigger()..." << endl;
  (void)dodgeResult;

  const bool IS_PLAYER = &actor == eng.player;
  const bool CAN_SEE = actor.getPropHandler()->allowSee();
  const bool CAN_PLAYER_SEE_ACTOR = eng.player->checkIfSeeActor(actor, NULL);
  const string actorName = actor.getNameThe();

  if(IS_PLAYER) {
    if(CAN_SEE) {
      eng.log->addMsg("I am hit by a burst of gas!");
    } else {
      eng.log->addMsg(
        "A mechanism triggers, I am hit by a burst of gas!");
    }
  } else {
    if(CAN_PLAYER_SEE_ACTOR) {
      eng.log->addMsg(actorName + " is hit by a burst of gas!");
    }
  }

  eng.explosionMaker-> runExplosion(
    pos_, endOfSfx, false, new PropParalyzed(eng, propTurnsStandard),
    true, getTrapSpecificColor());
  traceVerbose << "TrapGasParalyzation::trapSpecificTrigger() [DONE]" << endl;
}

void TrapGasFear::trapSpecificTrigger(Actor& actor,
                                      const AbilityRollResult_t dodgeResult) {
  traceVerbose << "TrapGasFear::trapSpecificTrigger()..." << endl;
  (void)dodgeResult;

  const bool IS_PLAYER = &actor == eng.player;
  const bool CAN_SEE = actor.getPropHandler()->allowSee();
  const bool CAN_PLAYER_SEE_ACTOR = eng.player->checkIfSeeActor(actor, NULL);
  const string actorName = actor.getNameThe();

  if(IS_PLAYER) {
    if(CAN_SEE) {
      eng.log->addMsg("I am hit by a burst of gas!");
    } else {
      eng.log->addMsg(
        "A mechanism triggers, I am hit by a burst of gas!");
    }
  } else {
    if(CAN_PLAYER_SEE_ACTOR) {
      eng.log->addMsg(actorName + " is hit by a burst of gas!");
    }
  }

  eng.explosionMaker-> runExplosion(
    pos_, endOfSfx, false, new PropTerrified(eng, propTurnsStandard),
    true, getTrapSpecificColor());
  traceVerbose << "TrapGasFear::trapSpecificTrigger() [DONE]" << endl;
}

void TrapBlindingFlash::trapSpecificTrigger(Actor& actor,
    const AbilityRollResult_t dodgeResult) {
  traceVerbose << "TrapBlindingFlash::trapSpecificTrigger()..." << endl;
  const bool IS_PLAYER = &actor == eng.player;
  const bool CAN_SEE = actor.getPropHandler()->allowSee();
  const bool CAN_PLAYER_SEE_ACTOR =
    eng.player->checkIfSeeActor(actor, NULL);
  const string actorName = actor.getNameThe();

  //Dodge?
  if(dodgeResult >= successSmall) {
    if(IS_PLAYER) {
      if(CAN_SEE) {
        eng.log->addMsg(
          "I cover my eyes just in time to avoid an intense flash!",
          clrMessageGood);
      } else {
        eng.log->addMsg("I feel a mechanism trigger!", clrMessageGood);
      }
    } else {
      if(CAN_PLAYER_SEE_ACTOR) {
        eng.log->addMsg(actorName + " covers from a blinding flash!");
      }
    }
  } else {
    //Dodge failed
    if(IS_PLAYER) {
      if(CAN_SEE) {
        eng.log->addMsg(
          "A sharp flash of light pierces my eyes!", clrWhite);
        actor.getPropHandler()->tryApplyProp(
          new PropBlind(eng, propTurnsStandard));
      } else {
        eng.log->addMsg("I feel a mechanism trigger!", clrWhite);
      }
    } else {
      if(CAN_PLAYER_SEE_ACTOR) {
        eng.log->addMsg(
          actorName + " is hit by a flash of blinding light!");
        actor.getPropHandler()->tryApplyProp(
          new PropBlind(eng, propTurnsStandard));
      }
    }
  }
  traceVerbose << "TrapBlindingFlash::trapSpecificTrigger() [DONE]" << endl;
}

void TrapTeleport::trapSpecificTrigger(Actor& actor,
                                       const AbilityRollResult_t dodgeResult) {
  traceVerbose << "TrapTeleport::trapSpecificTrigger()..." << endl;
  (void)dodgeResult;

  const bool IS_PLAYER = &actor == eng.player;
  const bool CAN_SEE = actor.getPropHandler()->allowSee();
  const bool CAN_PLAYER_SEE_ACTOR = eng.player->checkIfSeeActor(actor, NULL);
  const string actorName = actor.getNameThe();

  if(IS_PLAYER) {
    eng.player->updateFov();
    if(CAN_SEE) {
      eng.popup->showMessage(
        "A curious shape on the floor starts to glow!", true);
    } else {
      eng.popup->showMessage("I feel a peculiar energy around me!", true);
    }
  } else {
    if(CAN_PLAYER_SEE_ACTOR) {
      eng.log->addMsg(
        "A curious shape on the floor starts go glow under "
        + actorName + ".");
    }
  }

  actor.teleport(false);
  traceVerbose << "TrapTeleport::trapSpecificTrigger() [DONE]" << endl;
}

void TrapSummonMonster::trapSpecificTrigger(Actor& actor,
    const AbilityRollResult_t dodgeResult) {
  traceVerbose << "TrapSummonMonster::trapSpecificTrigger()..." << endl;

  (void)dodgeResult;

  const bool IS_PLAYER = &actor == eng.player;
  traceVerbose << "TrapSummonMonster: Is player: " << IS_PLAYER << endl;

  const bool CAN_SEE = actor.getPropHandler()->allowSee();
  traceVerbose << "TrapSummonMonster: Actor can see: " << CAN_SEE << endl;

  const bool CAN_PLAYER_SEE_ACTOR = eng.player->checkIfSeeActor(actor, NULL);
  traceVerbose << "TrapSummonMonster: Player see actor: ";
  traceVerbose << CAN_PLAYER_SEE_ACTOR << endl;

  const string actorName = actor.getNameThe();
  traceVerbose << "TrapSummonMonster: Actor name: " << actorName << endl;

  if(IS_PLAYER) {
    eng.player->updateFov();
    if(CAN_SEE) {
      eng.log->addMsg("A curious shape on the floor starts to glow!");
    } else {
      eng.log->addMsg("I feel a peculiar energy around me!");
    }
  } else {
    if(CAN_PLAYER_SEE_ACTOR) {
      eng.log->addMsg(
        "A curious shape on the floor starts go glow under "
        + actorName + ".");
    }
  }

  trace << "TrapSummonMonster: Finding summon canidates" << endl;
  vector<ActorId_t> summonCandidates;
  for(int i = 1; i < endOfActorIds; i++) {
    const ActorData& data = eng.actorDataHandler->dataList[i];
    if(data.canBeSummoned && data.spawnMinDLVL <= eng.map->getDlvl() + 3) {
      summonCandidates.push_back(ActorId_t(i));
    }
  }
  const int NR_ELEMENTS = summonCandidates.size();
  trace << "TrapSummonMonster: Nr candidates: " << NR_ELEMENTS << endl;

  if(NR_ELEMENTS == 0) {
    trace << "TrapSummonMonster: No eligible candidates found" << endl;
  } else {
    const int ELEMENT = eng.dice.range(0, NR_ELEMENTS - 1);
    const ActorId_t actorIdToSummon = summonCandidates.at(ELEMENT);
    trace << "TrapSummonMonster: Actor id: " << actorIdToSummon << endl;

    eng.actorFactory->summonMonsters(
      pos_, vector<ActorId_t>(1, actorIdToSummon), true);
    trace << "TrapSummonMonster: Monster was summoned" << endl;
  }
  traceVerbose << "TrapSummonMonster::trapSpecificTrigger() [DONE]" << endl;
}

void TrapSmoke::trapSpecificTrigger(Actor& actor,
                                    const AbilityRollResult_t dodgeResult) {
  traceVerbose << "TrapSmoke::trapSpecificTrigger()..." << endl;
  (void)dodgeResult;

  const bool IS_PLAYER = &actor == eng.player;
  const bool CAN_SEE = actor.getPropHandler()->allowSee();
  const bool CAN_PLAYER_SEE_ACTOR = eng.player->checkIfSeeActor(actor, NULL);
  const string actorName = actor.getNameThe();

  if(IS_PLAYER) {
    if(CAN_SEE) {
      eng.log->addMsg("Suddenly the air is thick with smoke!");
    } else {
      eng.log->addMsg(
        "A mechanism triggers, the air is thick with smoke!");
    }
  } else {
    if(CAN_PLAYER_SEE_ACTOR) {
      eng.log->addMsg("Suddenly the air around " + actorName +
                      " is thick with smoke!");
    }
  }

  eng.explosionMaker->runSmokeExplosion(pos_);
  traceVerbose << "TrapSmoke::trapSpecificTrigger() [DONE]" << endl;
}

void TrapAlarm::trapSpecificTrigger(Actor& actor,
                                    const AbilityRollResult_t dodgeResult) {
  traceVerbose << "TrapAlarm::trapSpecificTrigger() ..." << endl;
  (void)dodgeResult;

  const bool IS_PLAYER = &actor == eng.player;
  const bool CAN_PLAYER_SEE_ACTOR = eng.player->checkIfSeeActor(actor, NULL);
  const string actorName = actor.getNameThe();

  if(IS_PLAYER) {
    eng.log->addMsg("An alarm sounds!");
  } else {
    if(CAN_PLAYER_SEE_ACTOR) {
      eng.log->addMsg("Suddenly the air around " + actorName +
                      " is thick with smoke!");
    } else {
    }
  }

  Sound snd("I hear an alarm sounding!",
            endOfSfx, IS_PLAYER || CAN_PLAYER_SEE_ACTOR, pos_, true, true);
  eng.soundEmitter->emitSound(snd);
  traceVerbose << "TrapAlarm::trapSpecificTrigger() [DONE]" << endl;
}

void TrapSpiderWeb::trapSpecificTrigger(
  Actor& actor, const AbilityRollResult_t dodgeResult) {
  traceVerbose << "TrapSpiderWeb::trapSpecificTrigger()..." << endl;

  (void)dodgeResult;

  isHoldingActor = true;

  const bool IS_PLAYER = &actor == eng.player;
  const bool CAN_SEE = actor.getPropHandler()->allowSee();
  const bool CAN_PLAYER_SEE_ACTOR = eng.player->checkIfSeeActor(actor, NULL);
  const string actorName = actor.getNameThe();

  if(IS_PLAYER) {
    trace << "TrapSpiderWeb: Checking if player has machete" << endl;
    Inventory* const playerInv = eng.player->getInventory();
    Item* itemWielded = playerInv->getItemInSlot(slot_wielded);
    bool hasMachete = false;
    if(itemWielded != NULL) {
      hasMachete = itemWielded->getData().id == item_machete;
    }

    if(hasMachete) {
      if(CAN_SEE) {
        eng.log->addMsg("I cut down a spider web with my machete.");
      } else {
        eng.log->addMsg(
          "I cut down a sticky mass of threads with my machete.");
      }
      eng.featureFactory->spawnFeatureAt(feature_trashedSpiderWeb, pos_);
    } else {
      if(CAN_SEE) {
        eng.log->addMsg("I am entangled in a spider web!");
      } else {
        eng.log->addMsg("I am entangled by a sticky mass of threads!");
      }
    }
  } else {
    if(CAN_PLAYER_SEE_ACTOR) {
      eng.log->addMsg(actorName + " is entangled in a huge spider web!");
    }
  }
  traceVerbose << "TrapSpiderWeb::trapSpecificTrigger() [DONE]" << endl;
}

Dir_t TrapSpiderWeb::specificTrapActorTryLeave(Actor& actor, const Dir_t dir) {

  trace << "TrapSpiderWeb: specificTrapActorTryLeave()" << endl;

  if(isHoldingActor) {
    trace << "TrapSpiderWeb: Is holding actor" << endl;

    const bool IS_PLAYER = &actor == eng.player;
    const bool PLAYER_CAN_SEE = eng.player->getPropHandler()->allowSee();
    const bool PLAYER_CAN_SEE_ACTOR =
      eng.player->checkIfSeeActor(actor, NULL);
    const string actorName = actor.getNameThe();

    trace << "TrapSpiderWeb: Name of actor held: " << actorName << endl;

    //TODO reimplement something affecting chance of success?

    if(eng.dice.oneIn(4)) {
      trace << "TrapSpiderWeb: Actor succeeded to break free" << endl;

      isHoldingActor = false;

      if(IS_PLAYER) {
        eng.log->addMsg("I break free.");
      } else {
        if(PLAYER_CAN_SEE_ACTOR) {
          eng.log->addMsg(actorName + " breaks free from a spiderweb.");
        }
      }

      if(eng.dice.oneIn(2)) {
        trace << "TrapSpiderWeb: Web is destroyed" << endl;

        if(
          (IS_PLAYER && PLAYER_CAN_SEE) ||
          (IS_PLAYER == false && PLAYER_CAN_SEE_ACTOR)) {
          eng.log->addMsg("The web is destroyed.");
        }
        eng.featureFactory->spawnFeatureAt(feature_trashedSpiderWeb, pos_);
      }
    } else {
      if(IS_PLAYER) {
        eng.log->addMsg("I struggle to break free.");
      } else if(PLAYER_CAN_SEE_ACTOR) {
        eng.log->addMsg(actorName + " struggles to break free.");
      }
    }
    return dirCenter;
  } else {
    trace << "TrapSpiderWeb: Not holding actor" << endl;
  }
  return dir;
}
