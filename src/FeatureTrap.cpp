#include "FeatureTrap.h"

#include <algorithm>
#include <assert.h>

#include "Init.h"
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
#include "Utils.h"
#include "PlayerBon.h"

using namespace std;

//------------------------------------------------------------- TRAP
Trap::Trap(FeatureId id, Pos pos, TrapSpawnData* spawnData) :
  FeatureStatic(id, pos), mimicFeature_(spawnData->mimicFeature_),
  isHidden_(true) {

  assert(spawnData->trapType_ != endOfTraps);
  assert(mimicFeature_);

  assert(
    Map::cells[pos.x][pos.y].featureStatic->canHaveStaticFeature());

  if(spawnData->trapType_ == trap_any) {
    setSpecificTrapFromId(TrapId(Rnd::range(0, endOfTraps - 1)));
  } else {
    setSpecificTrapFromId(spawnData->trapType_);
  }
  assert(specificTrap_);
}

Trap::~Trap() {
  assert(specificTrap_);
  delete specificTrap_;
}

void Trap::setSpecificTrapFromId(const TrapId id) {
  switch(id) {
    case trap_dart:
      specificTrap_ = new TrapDart(pos_); break;
    case trap_spear:
      specificTrap_ = new TrapSpear(pos_); break;
    case trap_gasConfusion:
      specificTrap_ = new TrapGasConfusion(pos_); break;
    case trap_gasParalyze:
      specificTrap_ = new TrapGasParalyzation(pos_); break;
    case trap_gasFear:
      specificTrap_ = new TrapGasFear(pos_); break;
    case trap_blinding:
      specificTrap_ = new TrapBlindingFlash(pos_);   break;
    case trap_teleport:
      specificTrap_ = new TrapTeleport(pos_); break;
    case trap_summonMonster:
      specificTrap_ = new TrapSummonMonster(pos_); break;
    case trap_smoke:
      specificTrap_ = new TrapSmoke(pos_); break;
    case trap_alarm:
      specificTrap_ = new TrapAlarm(pos_); break;
    case trap_spiderWeb:
      specificTrap_ = new TrapSpiderWeb(pos_); break;
    default:
      specificTrap_ = nullptr; break;
  }
}

TrapId Trap::getTrapType() const {return specificTrap_->trapType_;}

bool Trap::isMagical() const {return specificTrap_->isMagical();}

void Trap::triggerOnPurpose(Actor& actorTriggering) {
  const AbilityRollResult DODGE_RESULT = failSmall;
  specificTrap_->trigger(actorTriggering, DODGE_RESULT);
}

void Trap::bump(Actor& actorBumping) {
  TRACE_FUNC_BEGIN;

  const ActorDataT& d = actorBumping.getData();

  TRACE << "Trap: Name of actor bumping: \"" << d.name_a << "\"" << endl;


  vector<PropId> props;
  actorBumping.getPropHandler().getAllActivePropIds(props);

  if(
    find(begin(props), end(props), propEthereal)  == end(props) &&
    find(begin(props), end(props), propFlying)    == end(props)) {
    const bool IS_PLAYER      = &actorBumping == Map::player;
    const bool ACTOR_CAN_SEE  = actorBumping.getPropHandler().allowSee();
    AbilityVals& abilities    = actorBumping.getData().abilityVals;
    const int DODGE_SKILL =
      abilities.getVal(AbilityId::dodgeTrap, true, actorBumping);
    const int BASE_CHANCE_TO_AVOID = 30;

    const string trapName = specificTrap_->getTitle();

    if(IS_PLAYER) {
      TRACE << "Trap: Player bumping" << endl;
      const int CHANCE_TO_AVOID = isHidden_ ? 10 :
                                  (BASE_CHANCE_TO_AVOID + DODGE_SKILL);
      const AbilityRollResult result = AbilityRoll::roll(CHANCE_TO_AVOID);

      if(result >= successSmall) {
        if(!isHidden_) {
          if(ACTOR_CAN_SEE) {
            Log::addMsg("I avoid a " + trapName + ".", clrMsgGood);
          }
        }
      } else {
        triggerTrap(actorBumping);
      }
    } else {
      if(d.actorSize == actorSize_humanoid && !d.isSpider) {
        TRACE << "Trap: Humanoid monster bumping" << endl;
        Monster* const monster = static_cast<Monster*>(&actorBumping);
        if(monster->awareOfPlayerCounter_ > 0 && !monster->isStealth) {
          TRACE << "Trap: Monster eligible for triggering trap" << endl;

          const bool IS_ACTOR_SEEN_BY_PLAYER =
            Map::player->isSeeingActor(actorBumping, nullptr);

          const int CHANCE_TO_AVOID = BASE_CHANCE_TO_AVOID + DODGE_SKILL;
          const AbilityRollResult result = AbilityRoll::roll(CHANCE_TO_AVOID);

          if(result >= successSmall) {
            if(!isHidden_ && IS_ACTOR_SEEN_BY_PLAYER) {
              const string actorName = actorBumping.getNameThe();
              Log::addMsg(actorName + " avoids a " + trapName + ".");
            }
          } else {
            triggerTrap(actorBumping);
          }
        }
      }
    }
  }
  TRACE_FUNC_END;
}

void Trap::disarm() {
  //Abort if trap is hidden
  if(isHidden()) {
    Log::addMsg(msgDisarmNoTrap);
    Renderer::drawMapAndInterface();
    return;
  }

  //Spider webs are automatically destroyed if wielding machete
  bool isAutoSucceed = false;
  if(getTrapType() == trap_spiderWeb) {
    Item* item = Map::player->getInv().getItemInSlot(SlotId::wielded);
    if(item) {
      isAutoSucceed = item->getData().id == ItemId::machete;
    }
  }

  const bool IS_OCCULTIST   = PlayerBon::getBg() == Bg::occultist;

  if(isMagical() && !IS_OCCULTIST) {
    Log::addMsg("I do not know how to dispel magic traps.");
    return;
  }

  vector<PropId> props;
  Map::player->getPropHandler().getAllActivePropIds(props);

  const bool IS_BLESSED =
    find(begin(props), end(props), propBlessed) != end(props);
  const bool IS_CURSED =
    find(begin(props), end(props), propCursed)  != end(props);

  int       disarmNumerator     = 5;
  const int DISARM_DENOMINATOR  = 10;

  if(IS_BLESSED)  disarmNumerator += 3;
  if(IS_CURSED)   disarmNumerator -= 3;

  constrInRange(1, disarmNumerator, DISARM_DENOMINATOR - 1);

  const bool IS_DISARMED =
    Rnd::fraction(disarmNumerator, DISARM_DENOMINATOR) || isAutoSucceed;
  if(IS_DISARMED) {
    Log::addMsg(specificTrap_->getDisarmMsg());
  } else {
    Log::addMsg(specificTrap_->getDisarmFailMsg());

    Renderer::drawMapAndInterface();
    const int TRIGGER_ONE_IN_N = IS_BLESSED ? 9 : IS_CURSED ? 2 : 4;
    if(Rnd::oneIn(TRIGGER_ONE_IN_N)) {
      if(getTrapType() == trap_spiderWeb) {
        Map::player->pos = pos_;
      }
      triggerTrap(*Map::player);
    }
  }
  GameTime::actorDidAct();

  if(IS_DISARMED) {FeatureFactory::mk(FeatureId::floor, pos_);}
}

void Trap::triggerTrap(Actor& actor) {
  TRACE_FUNC_BEGIN;

  TRACE << "Trap: Specific trap is " << specificTrap_->getTitle() << endl;

  const ActorDataT& d = actor.getData();

  TRACE << "Trap: Actor triggering is " << d.name_a << endl;

  const int DODGE_SKILL =
    d.abilityVals.getVal(AbilityId::dodgeTrap, true, actor);

  TRACE << "Trap: Actor dodge skill is " << DODGE_SKILL << endl;

  if(&actor == Map::player) {
    TRACE_VERBOSE << "Trap: Player triggering trap" << endl;
    const AbilityRollResult DODGE_RESULT =
      AbilityRoll::roll(DODGE_SKILL);
    reveal(false);
    TRACE_VERBOSE << "Trap: Calling trigger" << endl;
    specificTrap_->trigger(actor, DODGE_RESULT);
  } else {
    TRACE_VERBOSE << "Trap: Monster triggering trap" << endl;
    const bool IS_ACTOR_SEEN_BY_PLAYER =
      Map::player->isSeeingActor(actor, nullptr);
    const AbilityRollResult dodgeResult = AbilityRoll::roll(DODGE_SKILL);
    if(IS_ACTOR_SEEN_BY_PLAYER) {
      reveal(false);
    }
    TRACE_VERBOSE << "Trap: Calling trigger" << endl;
    specificTrap_->trigger(actor, dodgeResult);
  }

  TRACE_FUNC_END;
}

void Trap::reveal(const bool PRINT_MESSSAGE_WHEN_PLAYER_SEES) {
  TRACE_FUNC_BEGIN_VERBOSE;
  isHidden_ = false;

  //Destroy any corpse on the trap
  for(Actor* actor : GameTime::actors_) {
    if(actor->pos == pos_ && actor->deadState == ActorDeadState::corpse) {
      actor->deadState = ActorDeadState::destroyed;
    }
  }

  clearGore();

  Item* item = Map::cells[pos_.x][pos_.y].item;
  if(item) {
    Map::cells[pos_.x][pos_.y].item = nullptr;
    ItemDrop::dropItemOnMap(pos_, *item);
  }

  if(Map::cells[pos_.x][pos_.y].isSeenByPlayer) {
    Renderer::drawMapAndInterface();

    if(PRINT_MESSSAGE_WHEN_PLAYER_SEES) {
      const string name = specificTrap_->getTitle();
      Log::addMsg("I spot a " + name + ".", clrMsgWarning, false, true);
    }
  }
  TRACE_FUNC_END_VERBOSE;
}

void Trap::playerTrySpotHidden() {
  if(isHidden_) {
    const int PLAYER_SKILL = Map::player->getData().abilityVals.getVal(
                               AbilityId::searching, true, *(Map::player));

    if(AbilityRoll::roll(PLAYER_SKILL) >= successSmall) {
      reveal(true);
    }
  }
}

string Trap::getDescr(const bool DEFINITE_ARTICLE) const {
  if(isHidden_) {
    return DEFINITE_ARTICLE ? mimicFeature_->name_the :
           mimicFeature_->name_a;
  } else {
    return "a " + specificTrap_->getTitle();
  }
}

SDL_Color Trap::getClr() const {
  return isHidden_ ? mimicFeature_->clr : specificTrap_->getClr();
}

char Trap::getGlyph() const {
  return isHidden_ ? mimicFeature_->glyph : specificTrap_->getGlyph();
}

TileId Trap::getTile() const {
  return isHidden_ ? mimicFeature_->tile : specificTrap_->getTile();
}

Dir Trap::actorTryLeave(Actor& actor, const Dir dir) {
  TRACE << "Trap::actorTryLeave()" << endl;
  return specificTrap_->actorTryLeave(actor, dir);
}

MaterialType Trap::getMaterialType() const {
  return isHidden_ ? mimicFeature_->materialType : data_->materialType;
}

//------------------------------------------------------------- SPECIFIC TRAPS
TrapDart::TrapDart(Pos pos) :
  SpecificTrapBase(pos, trap_dart), isPoisoned(false) {
  isPoisoned =
    Map::dlvl >= MIN_DLVL_HARDER_TRAPS && Rnd::coinToss();
}

void TrapDart::trigger(
  Actor& actor, const AbilityRollResult dodgeResult) {

  TRACE_FUNC_BEGIN_VERBOSE;
  const bool IS_PLAYER = &actor == Map::player;
  const bool CAN_SEE = actor.getPropHandler().allowSee();
  const bool CAN_PLAYER_SEE_ACTOR = Map::player->isSeeingActor(actor, nullptr);
  const string actorName = actor.getNameThe();

  //Dodge?
  if(dodgeResult >= successSmall) {
    if(IS_PLAYER) {
      if(CAN_SEE) {
        Log::addMsg("I dodge a dart!", clrMsgGood);
      } else {
        Log::addMsg(
          "I feel a mechanism trigger and quickly leap aside!",
          clrMsgGood);
      }
    } else {
      if(CAN_PLAYER_SEE_ACTOR) {
        Log::addMsg(actorName +  " dodges a dart!");
      }
    }
  } else {
    //Trap misses?
    const int CHANCE_TO_HIT = 75;
    if(Rnd::percentile() > CHANCE_TO_HIT) {
      if(IS_PLAYER) {
        if(CAN_SEE) {
          Log::addMsg("A dart barely misses me!", clrMsgGood);
        } else {
          Log::addMsg(
            "A mechanism triggers, I hear something barely missing me!",
            clrMsgGood);
        }
      } else if(CAN_PLAYER_SEE_ACTOR) {
        Log::addMsg("A dart barely misses " + actorName + "!");
      }
    } else {
      //Dodge failed and trap hits
      if(IS_PLAYER) {
        if(CAN_SEE) {
          Log::addMsg("I am hit by a dart!", clrMsgBad);
        } else {
          Log::addMsg(
            "A mechanism triggers, I feel a needle piercing my skin!",
            clrMsgBad);
        }
      } else if(CAN_PLAYER_SEE_ACTOR) {
        Log::addMsg(actorName + " is hit by a dart!", clrMsgGood);
      }

      const int DMG = Rnd::dice(1, 8);
      actor.hit(DMG, DmgType::physical, true);
      if(actor.deadState == ActorDeadState::alive && isPoisoned) {
        if(IS_PLAYER) {
          Log::addMsg("It was poisoned!");
        }
        actor.getPropHandler().tryApplyProp(
          new PropPoisoned(propTurnsStd));
      }
    }
  }
  TRACE_FUNC_END_VERBOSE;
}

TrapSpear::TrapSpear(Pos pos) :
  SpecificTrapBase(pos, trap_spear), isPoisoned(false) {
  isPoisoned = Map::dlvl >= MIN_DLVL_HARDER_TRAPS && Rnd::oneIn(4);
}

void TrapSpear::trigger(
  Actor& actor, const AbilityRollResult dodgeResult) {

  TRACE_FUNC_BEGIN_VERBOSE;

  const bool IS_PLAYER = &actor == Map::player;
  const bool CAN_SEE = actor.getPropHandler().allowSee();
  const bool CAN_PLAYER_SEE_ACTOR = Map::player->isSeeingActor(actor, nullptr);
  const string actorName = actor.getNameThe();

  //Dodge?
  if(dodgeResult >= successSmall) {
    if(IS_PLAYER) {
      if(CAN_SEE) {
        Log::addMsg("I dodge a spear!", clrMsgGood);
      } else {
        Log::addMsg(
          "I feel a mechanism trigger and quickly leap aside!",
          clrMsgGood);
      }
    } else {
      if(CAN_PLAYER_SEE_ACTOR) {
        Log::addMsg(actorName +  " dodges a spear!");
      }
    }
  } else {
    //Trap misses?
    const int CHANCE_TO_HIT = 75;
    if(Rnd::percentile() > CHANCE_TO_HIT) {
      if(IS_PLAYER) {
        if(CAN_SEE) {
          Log::addMsg("A spear barely misses me!", clrMsgGood);
        } else {
          Log::addMsg(
            "A mechanism triggers, I hear a *swoosh*!",
            clrMsgGood);
        }
      } else {
        if(CAN_PLAYER_SEE_ACTOR) {
          Log::addMsg("A spear barely misses " + actorName + "!");
        }
      }
    } else {
      //Dodge failed and trap hits
      if(IS_PLAYER) {
        if(CAN_SEE) {
          Log::addMsg("I am hit by a spear!", clrMsgBad);
        } else {
          Log::addMsg(
            "A mechanism triggers, something sharp pierces my skin!",
            clrMsgBad);
        }
      } else {
        if(CAN_PLAYER_SEE_ACTOR) {
          Log::addMsg(actorName + " is hit by a spear!", clrMsgGood);
        }
      }

      const int DMG = Rnd::dice(2, 6);
      actor.hit(DMG, DmgType::physical, true);
      if(actor.deadState == ActorDeadState::alive && isPoisoned) {
        if(IS_PLAYER) {
          Log::addMsg("It was poisoned!");
        }
        actor.getPropHandler().tryApplyProp(
          new PropPoisoned(propTurnsStd));
      }
    }
  }
  TRACE_FUNC_BEGIN_VERBOSE;
}

void TrapGasConfusion::trigger(
  Actor& actor, const AbilityRollResult dodgeResult) {

  TRACE_FUNC_BEGIN_VERBOSE;
  (void)dodgeResult;

  const bool IS_PLAYER = &actor == Map::player;
  const bool CAN_SEE = actor.getPropHandler().allowSee();
  const bool CAN_PLAYER_SEE_ACTOR = Map::player->isSeeingActor(actor, nullptr);
  const string actorName = actor.getNameThe();

  if(IS_PLAYER) {
    if(CAN_SEE) {
      Log::addMsg("I am hit by a burst of gas!");
    } else {
      Log::addMsg(
        "A mechanism triggers, I am hit by a burst of gas!");
    }
  } else {
    if(CAN_PLAYER_SEE_ACTOR) {
      Log::addMsg(actorName + " is hit by a burst of gas!");
    }
  }

  SDL_Color clr = getClr();

  Explosion::runExplosionAt(
    pos_, ExplType::applyProp, ExplSrc::misc, 0, SfxId::endOfSfxId,
    new PropConfused(propTurnsStd), &clr);
  TRACE_FUNC_END_VERBOSE;
}

void TrapGasParalyzation::trigger(
  Actor& actor,  const AbilityRollResult dodgeResult) {

  TRACE_FUNC_BEGIN_VERBOSE;
  (void)dodgeResult;

  const bool IS_PLAYER = &actor == Map::player;
  const bool CAN_SEE = actor.getPropHandler().allowSee();
  const bool CAN_PLAYER_SEE_ACTOR = Map::player->isSeeingActor(actor, nullptr);
  const string actorName = actor.getNameThe();

  if(IS_PLAYER) {
    if(CAN_SEE) {
      Log::addMsg("I am hit by a burst of gas!");
    } else {
      Log::addMsg(
        "A mechanism triggers, I am hit by a burst of gas!");
    }
  } else {
    if(CAN_PLAYER_SEE_ACTOR) {
      Log::addMsg(actorName + " is hit by a burst of gas!");
    }
  }

  SDL_Color clr = getClr();
  Explosion::runExplosionAt(
    pos_, ExplType::applyProp, ExplSrc::misc, 0, SfxId::endOfSfxId,
    new PropParalyzed(propTurnsStd), &clr) ;
  TRACE_FUNC_END_VERBOSE;
}

void TrapGasFear::trigger(Actor& actor,
                          const AbilityRollResult dodgeResult) {

  TRACE_FUNC_BEGIN_VERBOSE;
  (void)dodgeResult;

  const bool IS_PLAYER = &actor == Map::player;
  const bool CAN_SEE = actor.getPropHandler().allowSee();
  const bool CAN_PLAYER_SEE_ACTOR = Map::player->isSeeingActor(actor, nullptr);
  const string actorName = actor.getNameThe();

  if(IS_PLAYER) {
    if(CAN_SEE) {
      Log::addMsg("I am hit by a burst of gas!");
    } else {
      Log::addMsg(
        "A mechanism triggers, I am hit by a burst of gas!");
    }
  } else {
    if(CAN_PLAYER_SEE_ACTOR) {
      Log::addMsg(actorName + " is hit by a burst of gas!");
    }
  }

  SDL_Color clr = getClr();
  Explosion::runExplosionAt(
    pos_, ExplType::applyProp, ExplSrc::misc, 0, SfxId::endOfSfxId,
    new PropTerrified(propTurnsStd), &clr);
  TRACE_FUNC_END_VERBOSE;
}

void TrapBlindingFlash::trigger(
  Actor& actor, const AbilityRollResult dodgeResult) {

  TRACE_FUNC_BEGIN_VERBOSE;
  const bool IS_PLAYER = &actor == Map::player;
  const bool CAN_SEE = actor.getPropHandler().allowSee();
  const bool CAN_PLAYER_SEE_ACTOR =
    Map::player->isSeeingActor(actor, nullptr);
  const string actorName = actor.getNameThe();

  //Dodge?
  if(dodgeResult >= successSmall) {
    if(IS_PLAYER) {
      if(CAN_SEE) {
        Log::addMsg(
          "I cover my eyes just in time to avoid an intense flash!",
          clrMsgGood);
      } else {
        Log::addMsg("I feel a mechanism trigger!", clrMsgGood);
      }
    } else {
      if(CAN_PLAYER_SEE_ACTOR) {
        Log::addMsg(actorName + " covers from a blinding flash!");
      }
    }
  } else {
    //Dodge failed
    if(IS_PLAYER) {
      if(CAN_SEE) {
        Log::addMsg(
          "A sharp flash of light pierces my eyes!", clrWhite);
        actor.getPropHandler().tryApplyProp(
          new PropBlind(propTurnsStd));
      } else {
        Log::addMsg("I feel a mechanism trigger!", clrWhite);
      }
    } else {
      if(CAN_PLAYER_SEE_ACTOR) {
        Log::addMsg(
          actorName + " is hit by a flash of blinding light!");
        actor.getPropHandler().tryApplyProp(
          new PropBlind(propTurnsStd));
      }
    }
  }
  TRACE_FUNC_END_VERBOSE;
}

void TrapTeleport::trigger(
  Actor& actor, const AbilityRollResult dodgeResult) {

  TRACE_FUNC_BEGIN_VERBOSE;
  (void)dodgeResult;

  const bool IS_PLAYER = &actor == Map::player;
  const bool CAN_SEE = actor.getPropHandler().allowSee();
  const bool CAN_PLAYER_SEE_ACTOR = Map::player->isSeeingActor(actor, nullptr);
  const string actorName = actor.getNameThe();

  if(IS_PLAYER) {
    Map::player->updateFov();
    if(CAN_SEE) {
      Popup::showMsg("A curious shape on the floor starts to glow!", true);
    } else {
      Popup::showMsg("I feel a peculiar energy around me!", true);
    }
  } else {
    if(CAN_PLAYER_SEE_ACTOR) {
      Log::addMsg(
        "A curious shape on the floor starts go glow under " + actorName + ".");
    }
  }

  actor.teleport(false);
  TRACE_FUNC_END_VERBOSE;
}

void TrapSummonMonster::trigger(
  Actor& actor, const AbilityRollResult dodgeResult) {

  TRACE_FUNC_BEGIN_VERBOSE;

  (void)dodgeResult;

  const bool IS_PLAYER = &actor == Map::player;
  TRACE_VERBOSE << "TrapSummonMonster: Is player: " << IS_PLAYER << endl;

  if(!IS_PLAYER) {
    TRACE_VERBOSE << "TrapSummonMonster: Not triggered by player" << endl;
    TRACE_FUNC_END_VERBOSE;
    return;
  }

  const bool CAN_SEE = actor.getPropHandler().allowSee();
  TRACE_VERBOSE << "TrapSummonMonster: Actor can see: " << CAN_SEE << endl;

  const string actorName = actor.getNameThe();
  TRACE_VERBOSE << "TrapSummonMonster: Actor name: " << actorName << endl;

  Map::player->incrShock(5, ShockSrc::misc);
  Map::player->updateFov();
  if(CAN_SEE) {
    Log::addMsg("A curious shape on the floor starts to glow!");
  } else {
    Log::addMsg("I feel a peculiar energy around me!");
  }

  TRACE << "TrapSummonMonster: Finding summon candidates" << endl;
  vector<ActorId> summonBucket;
  for(int i = 1; i < endOfActorIds; ++i) {
    const ActorDataT& data = ActorData::data[i];
    if(data.canBeSummoned && data.spawnMinDLVL <= Map::dlvl + 3) {
      summonBucket.push_back(ActorId(i));
    }
  }
  const int NR_ELEMENTS = summonBucket.size();
  TRACE << "TrapSummonMonster: Nr candidates: " << NR_ELEMENTS << endl;

  if(NR_ELEMENTS == 0) {
    TRACE << "TrapSummonMonster: No eligible candidates found" << endl;
  } else {
    const int ELEMENT = Rnd::range(0, NR_ELEMENTS - 1);
    const ActorId actorIdToSummon = summonBucket.at(ELEMENT);
    TRACE << "TrapSummonMonster: Actor id: " << actorIdToSummon << endl;

    ActorFactory::summonMonsters(
      pos_, vector<ActorId>(1, actorIdToSummon), true);
    TRACE << "TrapSummonMonster: Monster was summoned" << endl;
  }
  TRACE_FUNC_END_VERBOSE;
}

void TrapSmoke::trigger(
  Actor& actor, const AbilityRollResult dodgeResult) {

  TRACE_FUNC_BEGIN_VERBOSE;
  (void)dodgeResult;

  const bool IS_PLAYER = &actor == Map::player;
  const bool CAN_SEE = actor.getPropHandler().allowSee();
  const bool CAN_PLAYER_SEE_ACTOR = Map::player->isSeeingActor(actor, nullptr);
  const string actorName = actor.getNameThe();

  if(IS_PLAYER) {
    if(CAN_SEE) {
      Log::addMsg("Suddenly the air is thick with smoke!");
    } else {
      Log::addMsg("A mechanism triggers, the air is thick with smoke!");
    }
  } else {
    if(CAN_PLAYER_SEE_ACTOR) {
      Log::addMsg("Suddenly the air around " + actorName +
                  " is thick with smoke!");
    }
  }

  Explosion::runSmokeExplosionAt(pos_);
  TRACE_FUNC_END_VERBOSE;
}

void TrapAlarm::trigger(
  Actor& actor, const AbilityRollResult dodgeResult) {

  TRACE_FUNC_BEGIN_VERBOSE;
  (void)dodgeResult;

  IgnoreMsgIfOriginSeen msgIgnoreRule;

  if(Map::cells[pos_.x][pos_.y].isSeenByPlayer) {
    Log::addMsg("An alarm sounds!");
    msgIgnoreRule = IgnoreMsgIfOriginSeen::yes;
  } else {
    msgIgnoreRule = IgnoreMsgIfOriginSeen::no;
  }

  Snd snd("I hear an alarm sounding!", SfxId::endOfSfxId, msgIgnoreRule, pos_,
          &actor, SndVol::high, AlertsMonsters::yes);
  SndEmit::emitSnd(snd);
  TRACE_FUNC_END_VERBOSE;
}

void TrapSpiderWeb::trigger(
  Actor& actor, const AbilityRollResult dodgeResult) {
  TRACE_FUNC_BEGIN_VERBOSE;

  (void)dodgeResult;

  isHoldingActor = true;

  const bool IS_PLAYER = &actor == Map::player;
  const bool CAN_SEE = actor.getPropHandler().allowSee();
  const bool CAN_PLAYER_SEE_ACTOR = Map::player->isSeeingActor(actor, nullptr);
  const string actorName = actor.getNameThe();

  if(IS_PLAYER) {
    TRACE << "TrapSpiderWeb: Checking if player has machete" << endl;
    Inventory& playerInv = Map::player->getInv();
    Item* itemWielded = playerInv.getItemInSlot(SlotId::wielded);
    bool hasMachete = false;
    if(itemWielded) {
      hasMachete = itemWielded->getData().id == ItemId::machete;
    }

    if(hasMachete) {
      if(CAN_SEE) {
        Log::addMsg("I cut down a spider web with my machete.");
      } else {
        Log::addMsg(
          "I cut down a sticky mass of threads with my machete.");
      }
      FeatureFactory::mk(FeatureId::floor, pos_);
    } else {
      if(CAN_SEE) {
        Log::addMsg("I am entangled in a spider web!");
      } else {
        Log::addMsg("I am entangled by a sticky mass of threads!");
      }
    }
  } else {
    if(CAN_PLAYER_SEE_ACTOR) {
      Log::addMsg(actorName + " is entangled in a huge spider web!");
    }
  }
  TRACE_FUNC_END_VERBOSE;
}

Dir TrapSpiderWeb::actorTryLeave(Actor& actor, const Dir dir) {

  TRACE << "TrapSpiderWeb: actorTryLeave()" << endl;

  if(isHoldingActor) {
    TRACE << "TrapSpiderWeb: Is holding actor" << endl;

    const bool IS_PLAYER = &actor == Map::player;
    const bool PLAYER_CAN_SEE = Map::player->getPropHandler().allowSee();
    const bool PLAYER_CAN_SEE_ACTOR =
      Map::player->isSeeingActor(actor, nullptr);
    const string actorName = actor.getNameThe();

    TRACE << "TrapSpiderWeb: Name of actor held: " << actorName << endl;

    //TODO reimplement something affecting chance of success?

    if(Rnd::oneIn(4)) {
      TRACE << "TrapSpiderWeb: Actor succeeded to break free" << endl;

      isHoldingActor = false;

      if(IS_PLAYER) {
        Log::addMsg("I break free.");
      } else {
        if(PLAYER_CAN_SEE_ACTOR) {
          Log::addMsg(actorName + " breaks free from a spiderweb.");
        }
      }

      if(Rnd::oneIn(2)) {
        TRACE << "TrapSpiderWeb: Web is destroyed" << endl;

        if(
          (IS_PLAYER && PLAYER_CAN_SEE) ||
          (!IS_PLAYER && PLAYER_CAN_SEE_ACTOR)) {
          Log::addMsg("The web is destroyed.");
        }
        FeatureFactory::mk(FeatureId::floor, pos_);
      }
    } else {
      if(IS_PLAYER) {
        Log::addMsg("I struggle to break free.");
      } else if(PLAYER_CAN_SEE_ACTOR) {
        Log::addMsg(actorName + " struggles to break free.");
      }
    }
    return Dir::center;
  } else {
    TRACE << "TrapSpiderWeb: Not holding actor" << endl;
  }
  return dir;
}
