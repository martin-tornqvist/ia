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

//------------------------------------------------------------- TRAP
Trap::Trap(FeatureId id, Pos pos, Engine& engine, TrapSpawnData* spawnData) :
  FeatureStatic(id, pos, engine), mimicFeature_(spawnData->mimicFeature_),
  isHidden_(true) {

  assert(spawnData->trapType_ != endOfTraps);
  assert(mimicFeature_ != NULL);

  if(spawnData->trapType_ == trap_any) {
    setSpecificTrapFromId(TrapId(eng.dice.range(0, endOfTraps - 1)));
  } else {
    setSpecificTrapFromId(spawnData->trapType_);
  }
  assert(specificTrap_ != NULL);
}

Trap::~Trap() {
  assert(specificTrap_ != NULL);
  delete specificTrap_;
}

void Trap::setSpecificTrapFromId(const TrapId id) {
  switch(id) {
    case trap_dart:
      specificTrap_ = new TrapDart(pos_, eng); break;
    case trap_spear:
      specificTrap_ = new TrapSpear(pos_, eng); break;
    case trap_gasConfusion:
      specificTrap_ = new TrapGasConfusion(pos_, eng); break;
    case trap_gasParalyze:
      specificTrap_ = new TrapGasParalyzation(pos_, eng); break;
    case trap_gasFear:
      specificTrap_ = new TrapGasFear(pos_, eng); break;
    case trap_blinding:
      specificTrap_ = new TrapBlindingFlash(pos_, eng);   break;
    case trap_teleport:
      specificTrap_ = new TrapTeleport(pos_, eng); break;
    case trap_summonMonster:
      specificTrap_ = new TrapSummonMonster(pos_, eng); break;
    case trap_smoke:
      specificTrap_ = new TrapSmoke(pos_, eng); break;
    case trap_alarm:
      specificTrap_ = new TrapAlarm(pos_, eng); break;
    case trap_spiderWeb:
      specificTrap_ = new TrapSpiderWeb(pos_, eng); break;
    default:
      specificTrap_ = NULL; break;
  }
}

TrapId Trap::getTrapType() const {return specificTrap_->trapType_;}

bool Trap::isMagical() const {return specificTrap_->isMagical();}

void Trap::triggerOnPurpose(Actor& actorTriggering) {
  const AbilityRollResult DODGE_RESULT = failSmall;
  specificTrap_->trigger(actorTriggering, DODGE_RESULT);
}

void Trap::bump(Actor& actorBumping) {
  trace << "Trap::bump()..." << endl;

  const ActorData& d = actorBumping.getData();

  trace << "Trap: Name of actor bumping: \"" << d.name_a << "\"" << endl;


  vector<PropId> props;
  actorBumping.getPropHandler().getAllActivePropIds(props);

  if(
    find(props.begin(), props.end(), propEthereal)  == props.end() &&
    find(props.begin(), props.end(), propFlying)    == props.end()) {
    const bool IS_PLAYER = &actorBumping == actorBumping.eng.player;
    const bool ACTOR_CAN_SEE = actorBumping.getPropHandler().allowSee();
    AbilityValues& abilities = actorBumping.getData().abilityVals;
    const int DODGE_SKILL =
      abilities.getVal(ability_dodgeTrap, true, actorBumping);
    const int BASE_CHANCE_TO_AVOID = 30;

    const string trapName = specificTrap_->getTitle();

    if(IS_PLAYER) {
      trace << "Trap: Player bumping" << endl;
      const int CHANCE_TO_AVOID = isHidden_ == true ? 10 :
                                  (BASE_CHANCE_TO_AVOID + DODGE_SKILL);
      const AbilityRollResult result =
        actorBumping.eng.abilityRoll->roll(CHANCE_TO_AVOID);

      if(result >= successSmall) {
        if(isHidden_ == false) {
          if(ACTOR_CAN_SEE) {
            actorBumping.eng.log->addMsg(
              "I avoid a " + trapName + ".", clrMsgGood);
          }
        }
      } else {
        triggerTrap(actorBumping);
      }
    } else {
      if(d.actorSize == actorSize_humanoid && d.isSpider == false) {
        trace << "Trap: Humanoid monster bumping" << endl;
        Monster* const monster = dynamic_cast<Monster*>(&actorBumping);
        if(
          monster->awareOfPlayerCounter_ > 0 &&
          monster->isStealth == false) {
          trace << "Trap: Monster eligible for triggering trap" << endl;

          const bool IS_ACTOR_SEEN_BY_PLAYER =
            actorBumping.eng.player->isSeeingActor(actorBumping, NULL);

          const int CHANCE_TO_AVOID = BASE_CHANCE_TO_AVOID + DODGE_SKILL;
          const AbilityRollResult result =
            actorBumping.eng.abilityRoll->roll(CHANCE_TO_AVOID);

          if(result >= successSmall) {
            if(isHidden_ == false && IS_ACTOR_SEEN_BY_PLAYER) {
              const string actorName = actorBumping.getNameThe();
              actorBumping.eng.log->addMsg(
                actorName + " avoids a " + trapName + ".");
            }
          } else {
            triggerTrap(actorBumping);
          }
        }
      }
    }
  }
  trace << "Trap::bump() [DONE]" << endl;
}

void Trap::disarm() {
  vector<PropId> props;
  eng.player->getPropHandler().getAllActivePropIds(props);


  const bool IS_BLESSED =
    find(props.begin(), props.end(), propBlessed) != props.end();
  const bool IS_CURSED =
    find(props.begin(), props.end(), propCursed)  != props.end();

  int       disarmNumerator     = 5;
  const int DISARM_DENOMINATOR  = 10;

  if(IS_BLESSED)  disarmNumerator += 3;
  if(IS_CURSED)   disarmNumerator -= 3;

  constrInRange(1, disarmNumerator, DISARM_DENOMINATOR - 1);

  const bool IS_DISARMED =
    eng.dice.fraction(disarmNumerator, DISARM_DENOMINATOR);
  if(IS_DISARMED) {
    eng.log->addMsg("I disarm a trap.");
  } else {
    eng.log->addMsg("I fail to disarm a trap.");
    const int TRIGGER_ONE_IN_N = IS_BLESSED ? 9 : IS_CURSED ? 2 : 4;
    if(eng.dice.oneIn(TRIGGER_ONE_IN_N)) {
      triggerTrap(*eng.player);
    }
  }
  eng.gameTime->actorDidAct();

  if(IS_DISARMED) {
    eng.featureFactory->spawnFeatureAt(feature_stoneFloor, pos_);
  }
}

void Trap::triggerTrap(Actor& actor) {
  trace << "Trap::trigger()..." << endl;

  trace << "Trap: Specific trap is ";
  trace << specificTrap_->getTitle() << endl;

  const ActorData& d = actor.getData();

  trace << "Trap: Actor triggering is ";
  trace << d.name_a << endl;

  const int DODGE_SKILL =
    d.abilityVals.getVal(ability_dodgeTrap, true, actor);

  trace << "Trap: Actor dodge skill is ";
  trace << DODGE_SKILL << endl;

  if(&actor == eng.player) {
    traceVerbose << "Trap: Player triggering trap" << endl;
    const AbilityRollResult DODGE_RESULT =
      eng.abilityRoll->roll(DODGE_SKILL);
    reveal(false);
    traceVerbose << "Trap: Calling trigger" << endl;
    specificTrap_->trigger(actor, DODGE_RESULT);
  } else {
    traceVerbose << "Trap: Monster triggering trap" << endl;
    const bool IS_ACTOR_SEEN_BY_PLAYER =
      eng.player->isSeeingActor(actor, NULL);
    const AbilityRollResult dodgeResult = eng.abilityRoll->roll(DODGE_SKILL);
    if(IS_ACTOR_SEEN_BY_PLAYER) {
      reveal(false);
    }
    traceVerbose << "Trap: Calling trigger" << endl;
    specificTrap_->trigger(actor, dodgeResult);
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
      const string name = specificTrap_->getTitle();
      eng.log->addMsg("I spot a " + name + ".", clrOrange, false, true);
    }
  }
  traceVerbose << "Trap::reveal() [DONE]" << endl;
}

void Trap::playerTrySpotHidden() {
  if(isHidden_) {
    const int PLAYER_SKILL = eng.player->getData().abilityVals.getVal(
                               ability_searching, true, *(eng.player));

    if(eng.abilityRoll->roll(PLAYER_SKILL) >= successSmall) {
      reveal(true);
    }
  }
}

string Trap::getDescr(const bool DEFINITE_ARTICLE) const {
  if(isHidden_) {
    return DEFINITE_ARTICLE == true ? mimicFeature_->name_the :
           mimicFeature_->name_a;
  } else {
    return "a " + specificTrap_->getTitle();
  }
}

SDL_Color Trap::getClr() const {
  return isHidden_ ? mimicFeature_->color : specificTrap_->getClr();
}

char Trap::getGlyph() const {
  return isHidden_ ? mimicFeature_->glyph :
         specificTrap_->getGlyph();
}

TileId Trap::getTile() const {
  return isHidden_ ? mimicFeature_->tile : specificTrap_->getTile();
}

Dir Trap::actorTryLeave(Actor& actor, const Dir dir) {
  trace << "Trap::actorTryLeave()" << endl;
  return specificTrap_->actorTryLeave(actor, dir);
}

MaterialType Trap::getMaterialType() const {
  return isHidden_ ? mimicFeature_->materialType : data_->materialType;
}

//------------------------------------------------------------- SPECIFIC TRAPS
TrapDart::TrapDart(Pos pos, Engine& engine) :
  SpecificTrapBase(pos, trap_dart, engine), isPoisoned(false) {
  isPoisoned =
    eng.map->getDlvl() >= MIN_DLVL_NASTY_TRAPS && eng.dice.coinToss();
}

void TrapDart::trigger(
  Actor& actor, const AbilityRollResult dodgeResult) {

  traceVerbose << "TrapDart::trigger()..." << endl;
  const bool IS_PLAYER = &actor == eng.player;
  const bool CAN_SEE = actor.getPropHandler().allowSee();
  const bool CAN_PLAYER_SEE_ACTOR = eng.player->isSeeingActor(actor, NULL);
  const string actorName = actor.getNameThe();

  //Dodge?
  if(dodgeResult >= successSmall) {
    if(IS_PLAYER) {
      if(CAN_SEE) {
        eng.log->addMsg("I dodge a dart!", clrMsgGood);
      } else {
        eng.log->addMsg(
          "I feel a mechanism trigger and quickly leap aside!",
          clrMsgGood);
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
          eng.log->addMsg("A dart barely misses me!", clrMsgGood);
        } else {
          eng.log->addMsg(
            "A mechanism triggers, I hear something barely missing me!",
            clrMsgGood);
        }
      } else if(CAN_PLAYER_SEE_ACTOR) {
        eng.log->addMsg("A dart barely misses " + actorName + "!");
      }
    } else {
      //Dodge failed and trap hits
      if(IS_PLAYER) {
        if(CAN_SEE) {
          eng.log->addMsg("I am hit by a dart!", clrMsgBad);
        } else {
          eng.log->addMsg(
            "A mechanism triggers, I feel a needle piercing my skin!",
            clrMsgBad);
        }
      } else if(CAN_PLAYER_SEE_ACTOR) {
        eng.log->addMsg(actorName + " is hit by a dart!", clrMsgGood);
      }

      const int DMG = eng.dice(1, 8);
      actor.hit(DMG, dmgType_physical, true);
      if(actor.deadState == actorDeadState_alive && isPoisoned) {
        if(IS_PLAYER) {
          eng.log->addMsg("It was poisoned!");
        }
        actor.getPropHandler().tryApplyProp(
          new PropPoisoned(eng, propTurnsStd));
      }
    }
  }
  traceVerbose << "TrapDart::trigger() [DONE]" << endl;
}

TrapSpear::TrapSpear(Pos pos, Engine& engine) :
  SpecificTrapBase(pos, trap_spear, engine), isPoisoned(false) {
  isPoisoned =
    eng.map->getDlvl() >= MIN_DLVL_NASTY_TRAPS && eng.dice.coinToss();
}

void TrapSpear::trigger(
  Actor& actor, const AbilityRollResult dodgeResult) {

  traceVerbose << "TrapSpear::trigger()..." << endl;

  const bool IS_PLAYER = &actor == eng.player;
  const bool CAN_SEE = actor.getPropHandler().allowSee();
  const bool CAN_PLAYER_SEE_ACTOR = eng.player->isSeeingActor(actor, NULL);
  const string actorName = actor.getNameThe();

  //Dodge?
  if(dodgeResult >= successSmall) {
    if(IS_PLAYER) {
      if(CAN_SEE) {
        eng.log->addMsg("I dodge a spear!", clrMsgGood);
      } else {
        eng.log->addMsg(
          "I feel a mechanism trigger and quickly leap aside!",
          clrMsgGood);
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
          eng.log->addMsg("A spear barely misses me!", clrMsgGood);
        } else {
          eng.log->addMsg(
            "A mechanism triggers, I hear a *swoosh*!",
            clrMsgGood);
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
          eng.log->addMsg("I am hit by a spear!", clrMsgBad);
        } else {
          eng.log->addMsg(
            "A mechanism triggers, something sharp pierces my skin!",
            clrMsgBad);
        }
      } else {
        if(CAN_PLAYER_SEE_ACTOR) {
          eng.log->addMsg(actorName + " is hit by a spear!", clrMsgGood);
        }
      }

      const int DMG = eng.dice(2, 6);
      actor.hit(DMG, dmgType_physical, true);
      if(actor.deadState == actorDeadState_alive && isPoisoned) {
        if(IS_PLAYER) {
          eng.log->addMsg("It was poisoned!");
        }
        actor.getPropHandler().tryApplyProp(
          new PropPoisoned(eng, propTurnsStd));
      }
    }
  }
  traceVerbose << "TrapSpear::trigger()..." << endl;
}

void TrapGasConfusion::trigger(
  Actor& actor, const AbilityRollResult dodgeResult) {

  traceVerbose << "TrapGasConfusion::trigger()..." << endl;
  (void)dodgeResult;

  const bool IS_PLAYER = &actor == eng.player;
  const bool CAN_SEE = actor.getPropHandler().allowSee();
  const bool CAN_PLAYER_SEE_ACTOR = eng.player->isSeeingActor(actor, NULL);
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

  Explosion::runExplosionAt(
    pos_, eng, 0, endOfSfxId, false, new PropConfused(eng, propTurnsStd), true,
    getClr());
  traceVerbose << "TrapGasConfusion::trigger() [DONE]" << endl;
}

void TrapGasParalyzation::trigger(
  Actor& actor,  const AbilityRollResult dodgeResult) {

  traceVerbose << "TrapGasParalyzation::trigger()..." << endl;
  (void)dodgeResult;

  const bool IS_PLAYER = &actor == eng.player;
  const bool CAN_SEE = actor.getPropHandler().allowSee();
  const bool CAN_PLAYER_SEE_ACTOR = eng.player->isSeeingActor(actor, NULL);
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

  Explosion::runExplosionAt(
    pos_, eng, 0, endOfSfxId, false, new PropParalyzed(eng, propTurnsStd),
    true, getClr());
  traceVerbose << "TrapGasParalyzation::trigger() [DONE]" << endl;
}

void TrapGasFear::trigger(Actor& actor,
                          const AbilityRollResult dodgeResult) {

  traceVerbose << "TrapGasFear::trigger()..." << endl;
  (void)dodgeResult;

  const bool IS_PLAYER = &actor == eng.player;
  const bool CAN_SEE = actor.getPropHandler().allowSee();
  const bool CAN_PLAYER_SEE_ACTOR = eng.player->isSeeingActor(actor, NULL);
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

  Explosion::runExplosionAt(
    pos_, eng, 0, endOfSfxId, false, new PropTerrified(eng, propTurnsStd),
    true, getClr());
  traceVerbose << "TrapGasFear::trigger() [DONE]" << endl;
}

void TrapBlindingFlash::trigger(
  Actor& actor, const AbilityRollResult dodgeResult) {

  traceVerbose << "TrapBlindingFlash::trigger()..." << endl;
  const bool IS_PLAYER = &actor == eng.player;
  const bool CAN_SEE = actor.getPropHandler().allowSee();
  const bool CAN_PLAYER_SEE_ACTOR =
    eng.player->isSeeingActor(actor, NULL);
  const string actorName = actor.getNameThe();

  //Dodge?
  if(dodgeResult >= successSmall) {
    if(IS_PLAYER) {
      if(CAN_SEE) {
        eng.log->addMsg(
          "I cover my eyes just in time to avoid an intense flash!",
          clrMsgGood);
      } else {
        eng.log->addMsg("I feel a mechanism trigger!", clrMsgGood);
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
        actor.getPropHandler().tryApplyProp(
          new PropBlind(eng, propTurnsStd));
      } else {
        eng.log->addMsg("I feel a mechanism trigger!", clrWhite);
      }
    } else {
      if(CAN_PLAYER_SEE_ACTOR) {
        eng.log->addMsg(
          actorName + " is hit by a flash of blinding light!");
        actor.getPropHandler().tryApplyProp(
          new PropBlind(eng, propTurnsStd));
      }
    }
  }
  traceVerbose << "TrapBlindingFlash::trigger() [DONE]" << endl;
}

void TrapTeleport::trigger(
  Actor& actor, const AbilityRollResult dodgeResult) {

  traceVerbose << "TrapTeleport::trigger()..." << endl;
  (void)dodgeResult;

  const bool IS_PLAYER = &actor == eng.player;
  const bool CAN_SEE = actor.getPropHandler().allowSee();
  const bool CAN_PLAYER_SEE_ACTOR = eng.player->isSeeingActor(actor, NULL);
  const string actorName = actor.getNameThe();

  if(IS_PLAYER) {
    eng.player->updateFov();
    if(CAN_SEE) {
      eng.popup->showMsg(
        "A curious shape on the floor starts to glow!", true);
    } else {
      eng.popup->showMsg("I feel a peculiar energy around me!", true);
    }
  } else {
    if(CAN_PLAYER_SEE_ACTOR) {
      eng.log->addMsg(
        "A curious shape on the floor starts go glow under "
        + actorName + ".");
    }
  }

  actor.teleport(false);
  traceVerbose << "TrapTeleport::trigger() [DONE]" << endl;
}

void TrapSummonMonster::trigger(
  Actor& actor, const AbilityRollResult dodgeResult) {

  traceVerbose << "TrapSummonMonster::trigger()..." << endl;

  (void)dodgeResult;

  const bool IS_PLAYER = &actor == eng.player;
  traceVerbose << "TrapSummonMonster: Is player: " << IS_PLAYER << endl;

  const bool CAN_SEE = actor.getPropHandler().allowSee();
  traceVerbose << "TrapSummonMonster: Actor can see: " << CAN_SEE << endl;

  const bool CAN_PLAYER_SEE_ACTOR = eng.player->isSeeingActor(actor, NULL);
  traceVerbose << "TrapSummonMonster: Player see actor: ";
  traceVerbose << CAN_PLAYER_SEE_ACTOR << endl;

  const string actorName = actor.getNameThe();
  traceVerbose << "TrapSummonMonster: Actor name: " << actorName << endl;

  if(IS_PLAYER) {
    eng.player->incrShock(5, shockSrc_misc);
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

  trace << "TrapSummonMonster: Finding summon candidates" << endl;
  vector<ActorId> summonCandidates;
  for(int i = 1; i < endOfActorIds; i++) {
    const ActorData& data = eng.actorDataHandler->dataList[i];
    if(data.canBeSummoned && data.spawnMinDLVL <= eng.map->getDlvl() + 3) {
      summonCandidates.push_back(ActorId(i));
    }
  }
  const int NR_ELEMENTS = summonCandidates.size();
  trace << "TrapSummonMonster: Nr candidates: " << NR_ELEMENTS << endl;

  if(NR_ELEMENTS == 0) {
    trace << "TrapSummonMonster: No eligible candidates found" << endl;
  } else {
    const int ELEMENT = eng.dice.range(0, NR_ELEMENTS - 1);
    const ActorId actorIdToSummon = summonCandidates.at(ELEMENT);
    trace << "TrapSummonMonster: Actor id: " << actorIdToSummon << endl;

    eng.actorFactory->summonMonsters(
      pos_, vector<ActorId>(1, actorIdToSummon), true);
    trace << "TrapSummonMonster: Monster was summoned" << endl;
  }
  traceVerbose << "TrapSummonMonster::trigger() [DONE]" << endl;
}

void TrapSmoke::trigger(
  Actor& actor, const AbilityRollResult dodgeResult) {

  traceVerbose << "TrapSmoke::trigger()..." << endl;
  (void)dodgeResult;

  const bool IS_PLAYER = &actor == eng.player;
  const bool CAN_SEE = actor.getPropHandler().allowSee();
  const bool CAN_PLAYER_SEE_ACTOR = eng.player->isSeeingActor(actor, NULL);
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

  Explosion::runSmokeExplosionAt(pos_, eng);
  traceVerbose << "TrapSmoke::trigger() [DONE]" << endl;
}

void TrapAlarm::trigger(
  Actor& actor, const AbilityRollResult dodgeResult) {

  traceVerbose << "TrapAlarm::trigger() ..." << endl;
  (void)dodgeResult;

  const bool IS_PLAYER            = &actor == eng.player;
  const bool CAN_PLAYER_SEE_ACTOR = eng.player->isSeeingActor(actor, NULL);
  const string actorName          = actor.getNameThe();

  eng.log->addMsg("An alarm sounds!");

  const IgnoreMsgIfOriginSeen msgIgnore =
    (IS_PLAYER || CAN_PLAYER_SEE_ACTOR) ?
    IgnoreMsgIfOriginSeen::no : IgnoreMsgIfOriginSeen::yes;

  Snd snd("I hear an alarm sounding!", endOfSfxId, msgIgnore, pos_, &actor,
          SndVol::high, AlertsMonsters::yes);
  eng.sndEmitter->emitSnd(snd);
  traceVerbose << "TrapAlarm::trigger() [DONE]" << endl;
}

void TrapSpiderWeb::trigger(
  Actor& actor, const AbilityRollResult dodgeResult) {
  traceVerbose << "TrapSpiderWeb::trigger()..." << endl;

  (void)dodgeResult;

  isHoldingActor = true;

  const bool IS_PLAYER = &actor == eng.player;
  const bool CAN_SEE = actor.getPropHandler().allowSee();
  const bool CAN_PLAYER_SEE_ACTOR = eng.player->isSeeingActor(actor, NULL);
  const string actorName = actor.getNameThe();

  if(IS_PLAYER) {
    trace << "TrapSpiderWeb: Checking if player has machete" << endl;
    Inventory& playerInv = eng.player->getInv();
    Item* itemWielded = playerInv.getItemInSlot(slot_wielded);
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
      eng.featureFactory->spawnFeatureAt(feature_stoneFloor, pos_);
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
  traceVerbose << "TrapSpiderWeb::trigger() [DONE]" << endl;
}

Dir TrapSpiderWeb::actorTryLeave(Actor& actor, const Dir dir) {

  trace << "TrapSpiderWeb: actorTryLeave()" << endl;

  if(isHoldingActor) {
    trace << "TrapSpiderWeb: Is holding actor" << endl;

    const bool IS_PLAYER = &actor == eng.player;
    const bool PLAYER_CAN_SEE = eng.player->getPropHandler().allowSee();
    const bool PLAYER_CAN_SEE_ACTOR =
      eng.player->isSeeingActor(actor, NULL);
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
        eng.featureFactory->spawnFeatureAt(feature_stoneFloor, pos_);
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
