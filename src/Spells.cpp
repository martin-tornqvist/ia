#include "Spells.h"

#include <algorithm>

#include "Engine.h"
#include "ActorMonster.h"
#include "ActorPlayer.h"
#include "Log.h"
#include "Postmortem.h"
#include "Knockback.h"
#include "Map.h"
#include "ActorFactory.h"

Spell* SpellHandler::getRandomSpellForMonsters() {
  return getSpellFromEnum(static_cast<Spells_t>(eng->dice.getInRange(0, endOfSpells - 1)));
}

void SpellHandler::addAllCommonSpellsForMonsters(vector<Spell*>& knownSpells) const {
  for(unsigned int i = 0; i < endOfSpells; i++) {
    knownSpells.push_back(getSpellFromEnum(static_cast<Spells_t>(i)));
  }
}

Spell* SpellHandler::getSpellFromEnum(const Spells_t spell) const {
  switch(spell) {
  case spell_blind:
    return new SpellBlind;
    break;
  case spell_disease:
    return new SpellDisease;
    break;
  case spell_fear:
    return new SpellFear;
    break;
  case spell_shriveling:
    return new SpellShriveling;
    break;
  case spell_slow:
    return new SpellSlow;
    break;
  case spell_summonRandom:
    return new SpellSummonRandom;
    break;
  case spell_healSelf:
    return new SpellHealSelf;
    break;
  case spell_knockBack:
    return new SpellKnockBack;
    break;
  case spell_teleport:
    return new SpellTeleport;
    break;
  case spell_confuse:
    return new SpellConfuse;
    break;
  default:
    return NULL;
    break;
  }
}

//SHRIVELING---------------------------------------------------------------------------
void SpellShriveling::specificCast(const SpellData& d, Engine* const eng) {
  Actor* actor = eng->mapTests->getActorAtPos(d.targetCell_);

  if(actor != NULL) {

    if(actor == eng->player) {
      if(actor->getStatusEffectsHandler()->allowSee() == true) {
        eng->log->addMessage("I feel a terrible pain as my skin suddenly shrivel and blacken!", clrMessageBad);
      } else {
        eng->log->addMessage("I suddenly feel a terrible pain over my skin!", clrMessageBad);
      }
    }

    actor->hit(eng->dice(1, 6), damageType_pure);
  }
}

void SpellShriveling::specificMonsterCast(Monster* const monster, Engine* const eng) {
  specificCast(SpellData(monster, eng->player->pos), eng);
}

bool SpellShriveling::isGoodForMonsterNow(const Monster* const monster, Engine* const engine) {
  bool blockers[MAP_X_CELLS][MAP_Y_CELLS];
  engine->mapTests->makeVisionBlockerArray(monster->pos, blockers);
  return monster->checkIfSeeActor(*(engine->player), blockers);
}


//TELEPORT---------------------------------------------------------------------------
void SpellTeleport::specificCast(const SpellData& d, Engine* const eng) {
  if(eng->player->checkIfSeeActor(*(d.caster_), NULL)) {
    eng->log->addMessage(d.caster_->getNameThe() + " dissapears in a blast of smoke!");
  }

  d.caster_->teleportToRandom();
}

void SpellTeleport::specificMonsterCast(Monster* const monster, Engine* const eng) {
  specificCast(SpellData(monster, coord(-1, -1)), eng);
}

bool SpellTeleport::isGoodForMonsterNow(const Monster* const monster, Engine* const engine) {
  bool blockers[MAP_X_CELLS][MAP_Y_CELLS];
  engine->mapTests->makeVisionBlockerArray(monster->pos, blockers);
  return monster->checkIfSeeActor(*(engine->player), blockers) && monster->getHp() <= (monster->getHpMax() / 2) && engine->dice.coinToss();
}



//KNOCK BACK---------------------------------------------------------------------------
void SpellKnockBack::specificCast(const SpellData& d, Engine* const eng) {
  Actor* actor = eng->mapTests->getActorAtPos(d.targetCell_);

  if(actor != NULL) {
    if(actor == eng->player) {
      eng->log->addMessage("A force pushes me!", clrMessageBad);
    }
    eng->knockBack->attemptKnockBack(eng->player, d.caster_->pos, false);
  }
}

void SpellKnockBack::specificMonsterCast(Monster* const monster, Engine* const eng) {
  specificCast(SpellData(monster, eng->player->pos), eng);
}

bool SpellKnockBack::isGoodForMonsterNow(const Monster* const monster, Engine* const engine) {
  bool blockers[MAP_X_CELLS][MAP_Y_CELLS];
  engine->mapTests->makeVisionBlockerArray(monster->pos, blockers);
  return monster->checkIfSeeActor(*(engine->player), blockers);
}

//CONFUSE--------------------------------------------------------------------------------
void SpellConfuse::specificCast(const SpellData& d, Engine* const eng) {
  Actor* actor = eng->mapTests->getActorAtPos(d.targetCell_);

  if(actor != NULL) {

    if(actor == eng->player) {
      eng->log->addMessage("My mind is reeling!");
    }

    actor->getStatusEffectsHandler()->attemptAddEffect(new StatusConfused(eng->dice(3, 6)));
  }
}

void SpellConfuse::specificMonsterCast(Monster* const monster, Engine* const eng) {
  specificCast(SpellData(monster, eng->player->pos), eng);
}

bool SpellConfuse::isGoodForMonsterNow(const Monster* const monster, Engine* const engine) {
  bool blockers[MAP_X_CELLS][MAP_Y_CELLS];
  engine->mapTests->makeVisionBlockerArray(monster->pos, blockers);
  if(monster->checkIfSeeActor(*(engine->player), blockers)) {
    return engine->player->getStatusEffectsHandler()->allowSee() == true;
  }
  return false;
}

//BLIND--------------------------------------------------------------------------------
void SpellBlind::specificCast(const SpellData& d, Engine* const eng) {
  Actor* actor = eng->mapTests->getActorAtPos(d.targetCell_);

  if(actor != NULL) {

    if(actor == eng->player) {
      eng->log->addMessage("Scales starts to grow over my eyes!");
    }

    actor->getStatusEffectsHandler()->attemptAddEffect(new StatusBlind(eng->dice(3, 6)));
  }
}

void SpellBlind::specificMonsterCast(Monster* const monster, Engine* const eng) {
  specificCast(SpellData(monster, eng->player->pos), eng);
}

bool SpellBlind::isGoodForMonsterNow(const Monster* const monster, Engine* const engine) {
  bool blockers[MAP_X_CELLS][MAP_Y_CELLS];
  engine->mapTests->makeVisionBlockerArray(monster->pos, blockers);
  if(monster->checkIfSeeActor(*(engine->player), blockers)) {
    return engine->player->getStatusEffectsHandler()->allowSee() == true;
  }
  return false;
}

//FEAR---------------------------------------------------------------------------------
void SpellFear::specificCast(const SpellData& d, Engine* const eng) {
  Actor* actor = eng->mapTests->getActorAtPos(d.targetCell_);

  if(actor != NULL) {

    if(actor == eng->player) {
      eng->log->addMessage("My mind is besieged by terror.");
    }

    actor->getStatusEffectsHandler()->attemptAddEffect(new StatusTerrified(eng->dice(3, 6)));
  }
}

void SpellFear::specificMonsterCast(Monster* const monster, Engine* const eng) {
  specificCast(SpellData(monster, eng->player->pos), eng);
}

bool SpellFear::isGoodForMonsterNow(const Monster* const monster, Engine* const engine) {
  bool blockers[MAP_X_CELLS][MAP_Y_CELLS];
  engine->mapTests->makeVisionBlockerArray(monster->pos, blockers);
  return monster->checkIfSeeActor(*(engine->player), blockers);
}

//SLOW---------------------------------------------------------------------------------
void SpellSlow::specificCast(const SpellData& d, Engine* const eng) {
  Actor* actor = eng->mapTests->getActorAtPos(d.targetCell_);

  if(actor != NULL) {

    if(actor == eng->player) {
      eng->log->addMessage("I start to feel bogged down.");
    }

    actor->getStatusEffectsHandler()->attemptAddEffect(new StatusSlowed(eng->dice(3, 6)));
  }
}

void SpellSlow::specificMonsterCast(Monster* const monster, Engine* const eng) {
  specificCast(SpellData(monster, eng->player->pos), eng);
}

bool SpellSlow::isGoodForMonsterNow(const Monster* const monster, Engine* const engine) {
  bool blockers[MAP_X_CELLS][MAP_Y_CELLS];
  engine->mapTests->makeVisionBlockerArray(monster->pos, blockers);
  return monster->checkIfSeeActor(*(engine->player), blockers);
}

//DISEASE---------------------------------------------------------------------------------
void SpellDisease::specificCast(const SpellData& d, Engine* const eng) {
  Actor* actor = eng->mapTests->getActorAtPos(d.targetCell_);

  if(actor != NULL) {

    if(actor == eng->player) {
      eng->log->addMessage("A disease is starting to afflict my body!", clrMessageBad);
    }

    actor->getStatusEffectsHandler()->attemptAddEffect(new StatusDiseased(eng));
  }
}

void SpellDisease::specificMonsterCast(Monster* const monster, Engine* const eng) {
  specificCast(SpellData(monster, eng->player->pos), eng);
}

bool SpellDisease::isGoodForMonsterNow(const Monster* const monster, Engine* const engine) {
  bool blockers[MAP_X_CELLS][MAP_Y_CELLS];
  engine->mapTests->makeVisionBlockerArray(monster->pos, blockers);
  return monster->checkIfSeeActor(*(engine->player), blockers);
}

//SUMMON RANDOM---------------------------------------------------------------------------
void SpellSummonRandom::specificCast(const SpellData& d, Engine* const eng) {
  vector<ActorDevNames_t> summonCandidates;
  for(unsigned int i = 1; i < endOfActorDevNames; i++) {
    const ActorDefinition& def = eng->actorData->actorDefinitions[i];
    if(def.canBeSummoned == true) {
      //Monster summoned must be equal or lower level to the caster.
      //(No checks needed for available summons, since wolves (lvl 2) can be summoned)
      if(def.monsterLvl <= d.caster_->getDef()->monsterLvl) {
        summonCandidates.push_back(static_cast<ActorDevNames_t>(i));
      }
    }
  }
  const ActorDevNames_t actorSummoned = summonCandidates.at(eng->dice(1, summonCandidates.size()) - 1);
  Monster* monster = dynamic_cast<Monster*>(eng->actorFactory->spawnActor(actorSummoned, d.targetCell_));
  monster->playerAwarenessCounter = monster->getDef()->nrTurnsAwarePlayer;
  if(eng->map->playerVision[d.targetCell_.x][d.targetCell_.y] == true) {
    eng->log->addMessage(monster->getNameA() + " appears.");
  }
}

void SpellSummonRandom::specificMonsterCast(Monster* const monster, Engine* const eng) {
  coord summonPos(monster->pos);

  vector<coord> freePositionsSeenByPlayer;
  for(int x = max(0, eng->player->pos.x - FOV_STANDARD_RADI_INT); x < min(MAP_X_CELLS, eng->player->pos.x + FOV_STANDARD_RADI_INT); x++) {
    for(int y = max(0, eng->player->pos.y - FOV_STANDARD_RADI_INT); y < min(MAP_Y_CELLS, eng->player->pos.y + FOV_STANDARD_RADI_INT); y++) {
      if(eng->map->playerVision[x][y]) {
        freePositionsSeenByPlayer.push_back(coord(x, y));
      }
    }
  }

  bool blockers[MAP_X_CELLS][MAP_Y_CELLS];
//	eng->mapTests->makeVisionBlockerArray(blockers);
  eng->mapTests->makeMoveBlockerArrayForMoveType(moveType_walk, blockers);

  for(int i = 0; i < static_cast<int>(freePositionsSeenByPlayer.size()); i++) {
    const coord pos = freePositionsSeenByPlayer.at(i);
    if(blockers[pos.x][pos.y]) {
      freePositionsSeenByPlayer.erase(freePositionsSeenByPlayer.begin() + i);
      i--;
    }
  }
  //First try to summon inside players FOV
  if(freePositionsSeenByPlayer.size() > 0) {
    summonPos = freePositionsSeenByPlayer.at(eng->dice(1, freePositionsSeenByPlayer.size()) - 1);
  } else {
    vector<coord> freeCellsVector;
    eng->mapTests->makeMapVectorFromArray(blockers, freeCellsVector);
    if(freeCellsVector.size() > 0) {
      sort(freeCellsVector.begin(), freeCellsVector.end(), IsCloserToOrigin(monster->pos, eng));
      summonPos = freeCellsVector.at(0);
    }
  }

  specificCast(SpellData(monster, summonPos), eng);
}

bool SpellSummonRandom::isGoodForMonsterNow(const Monster* const monster, Engine* const engine) {
  const int CHANCE_FOR_OK_IF_NOT_SEE_PLAYER = 5;

  bool blockers[MAP_X_CELLS][MAP_Y_CELLS];
  engine->mapTests->makeVisionBlockerArray(monster->pos, blockers);
  const bool MONSTER_SEES_PLAYER = monster->checkIfSeeActor(*(engine->player), blockers);
  return MONSTER_SEES_PLAYER || (engine->dice(1, 100) < CHANCE_FOR_OK_IF_NOT_SEE_PLAYER);
}

void Spell::cast(const SpellData& d, Engine* const eng) {
  specificCast(d, eng);
  eng->gameTime->letNextAct();
}

void Spell::monsterCast(Monster* const monster, Engine* const eng) {
  if(eng->map->playerVision[monster->pos.x][monster->pos.y] == true) {
    const string SPELL_MESSAGE = monster->getDef()->spellCastMessage;
    eng->log->addMessage(SPELL_MESSAGE);
  }

  monster->spellCoolDownCurrent = monster->getDef()->spellCooldownTurns;
  specificMonsterCast(monster, eng);
  eng->gameTime->letNextAct();
}

//HEAL SELF ---------------------------------------------------------------------------
void SpellHealSelf::specificCast(const SpellData& d, Engine* const eng) {
  (void)eng;
  d.caster_->restoreHP(999, true);
}

void SpellHealSelf::specificMonsterCast(Monster* const monster, Engine* const eng) {
  specificCast(SpellData(monster), eng);
}

bool SpellHealSelf::isGoodForMonsterNow(const Monster* const monster, Engine* const engine) {
  (void)engine;
  return monster->getHp() < monster->getHpMax();
}
