#include "Spells.h"

#include <algorithm>

#include "Engine.h"

#include "Renderer.h"
#include "ActorMonster.h"
#include "ActorPlayer.h"
#include "Log.h"
#include "Postmortem.h"
#include "Knockback.h"
#include "Map.h"
#include "ActorFactory.h"
#include "FeatureTrap.h"
#include "PlayerSpellsHandler.h"
#include "ItemScroll.h"
#include "ItemArmor.h"
#include "Inventory.h"

Spell* SpellHandler::getRandomSpellForMonster() {
  vector<Spell_t> candidates;
  for(int i = 0; i < endOfSpells; i++) {
    Spell* const spell = getSpellFromId(Spell_t(i));
    if(spell->isLearnableForMonsters()) {
      candidates.push_back(Spell_t(i));
    }
    delete spell;
  }
  const int ELEMENT = eng->dice.range(0, candidates.size() - 1);
  return getSpellFromId(candidates.at(ELEMENT));
}

Spell* SpellHandler::getSpellFromId(const Spell_t spellId) const {
  switch(spellId) {
    case spell_enfeeble:            return new SpellEnfeeble;           break;
    case spell_disease:             return new SpellDisease;            break;
    case spell_azathothsBlast:      return new SpellAzathothsBlast;     break;
    case spell_summonRandom:        return new SpellSummonRandom;       break;
    case spell_healSelf:            return new SpellHealSelf;           break;
    case spell_knockBack:           return new SpellKnockBack;          break;
    case spell_teleport:            return new SpellTeleport;           break;
    case spell_mayhem:              return new SpellMayhem;             break;
    case spell_pestilence:          return new SpellPestilence;         break;
    case spell_detectItems:         return new SpellDetectItems;        break;
    case spell_detectTraps:         return new SpellDetectTraps;        break;
    case spell_clairvoyance:        return new SpellClairvoyance;       break;
    case spell_opening:             return new SpellOpening;            break;
    case spell_sacrificeLife:       return new SpellSacrificeLife;      break;
    case spell_sacrificeSpirit:     return new SpellSacrificeSpirit;    break;
    case spell_mthPower:            return new SpellMthPower;           break;
    case spell_bless:               return new SpellBless;              break;

    case endOfSpells: {} break;
  }
  trace << "[WARNING] Found no spell for ID: " << spellId;
  trace << ", in SpellHandler::getSpellFromId()" << endl;
  return NULL;
}

Range Spell::getSpiCost(const bool IS_BASE_COST_ONLY, Actor* const caster,
                        Engine* const eng) const {
  int costMax = getSpecificMaxSpiCost();

  if(IS_BASE_COST_ONLY == false) {
    for(int dy = -1; dy <= 1; dy++) {
      for(int dx = -1; dx <= 1; dx++) {
        const Pos pos(caster->pos + Pos(dx, dy));
        if(eng->map->featuresStatic[pos.x][pos.y]->getId() == feature_altar) {
          costMax -= 1;
          dy = 999;
          dx = 999;
        }
      }
    }

    PropHandler* propHandeler = caster->getPropHandler();

    if(propHandeler->hasProp(propBlessed))  {costMax -= 1;}
    if(propHandeler->allowSee() == false)   {costMax -= 1;}
    if(propHandeler->hasProp(propCursed))   {costMax += 3;}

    if(caster == eng->player) {
      costMax -= eng->player->getMth() / CAST_FROM_MEMORY_MTH_BON_DIV;
    }
  }

  costMax             = max(1, costMax);
  const int COST_MIN  = max(1, costMax / 2);

  return Range(COST_MIN, costMax);
}


SpellCastRetData Spell::cast(Actor* const caster, const bool IS_INTRINSIC,
                             Engine* const eng) {
  trace << "Spell::cast()..." << endl;
  if(caster->getPropHandler()->allowCastSpells(true)) {
    if(caster == eng->player) {
      trace << "Spell: Player casting spell" << endl;
      eng->player->incrShock(SHOCK_TAKEN_FROM_CASTING_SPELLS);
    } else {
      trace << "Spell: Monster casting spell" << endl;
      Monster* const monster = dynamic_cast<Monster*>(caster);
      if(eng->map->playerVision[monster->pos.x][monster->pos.y]) {
        const string spellStr = monster->getData()->spellCastMessage;
        eng->log->addMsg(spellStr);
      }
      monster->spellCoolDownCurrent = monster->getData()->spellCooldownTurns;
    }

    if(IS_INTRINSIC) {
      const Range cost = getSpiCost(false, caster, eng);
      caster->hitSpi(eng->dice.range(cost));
    }
    SpellCastRetData ret(false);
    if(caster->deadState == actorDeadState_alive) {
      ret = specificCast(caster, eng);
    }

    eng->gameTime->endTurnOfCurrentActor();
    trace << "Spell::cast() [DONE]" << endl;
    return ret;
  }
  trace << "Spell::cast() [DONE]" << endl;
  return SpellCastRetData(false);
}

//------------------------------------------------------------ AZATHOTHS BLAST
SpellCastRetData SpellAzathothsBlast::specificCast(
  Actor* const caster, Engine* const eng) {
  DiceParam spellDmg(1, 8, 0);
  if(caster == eng->player) {
    vector<Actor*> spotedEnemies;
    eng->player->getSpotedEnemies(spotedEnemies);

    if(spotedEnemies.empty()) {
      return SpellCastRetData(false);
    } else {
      vector<Pos> actorPositions;

      for(unsigned int i = 0; i < spotedEnemies.size(); i++) {
        actorPositions.push_back(spotedEnemies.at(i)->pos);
      }

      eng->renderer->drawBlastAnimationAtPositionsWithPlayerVision(
        actorPositions, clrRedLgt);

      for(unsigned int i = 0; i < spotedEnemies.size(); i++) {
        const string monsterName = spotedEnemies.at(i)->getNameThe();
        eng->log->addMsg(
          monsterName + " is struck by a roaring blast!", clrMessageGood);
        spotedEnemies.at(i)->getPropHandler()->tryApplyProp(
          new PropParalyzed(eng, propTurnsSpecified, 1));
        spotedEnemies.at(i)->hit(eng->dice(1, 8), dmgType_physical, true);
        Sound snd("I hear a roaring blast",
                  endOfSfx, true, spotedEnemies.at(i)->pos, true, true);
        eng->soundEmitter->emitSound(snd);
      }
      return SpellCastRetData(true);
    }
  } else {
    eng->log->addMsg("I am struck by a roaring blast!", clrMessageBad);
    eng->renderer->drawBlastAnimationAtPositionsWithPlayerVision(
      vector<Pos>(1, eng->player->pos), clrRedLgt);
    eng->player->getPropHandler()->tryApplyProp(
      new PropParalyzed(eng, propTurnsSpecified, 1));
    eng->player->hit(eng->dice(1, 8), dmgType_physical, true);
    Sound snd("", endOfSfx, true, eng->player->pos, true, true);
    eng->soundEmitter->emitSound(snd);
    return SpellCastRetData(false);
  }
}

bool SpellAzathothsBlast::isGoodForMonsterToCastNow(
  Monster* const monster, Engine* const eng) {
  bool blockers[MAP_X_CELLS][MAP_Y_CELLS];
  eng->mapTests->makeVisionBlockerArray(monster->pos, blockers);
  return monster->checkIfSeeActor(*(eng->player), blockers);
}

//------------------------------------------------------------ MAYHEM
SpellCastRetData SpellMayhem::specificCast(
  Actor* const caster, Engine* const eng) {
  (void)caster;

  eng->log->addMsg("Destruction rages around me!");

  const Pos& playerPos = eng->player->pos;

  const int NR_OF_SWEEPS  = 5;
  const int AREA_RADI     = FOV_STANDARD_RADI_INT + 1;

  const int X0 = max(1, playerPos.x - AREA_RADI);
  const int Y0 = max(1, playerPos.y - AREA_RADI);
  const int X1 = min(MAP_X_CELLS - 1, playerPos.x + AREA_RADI) - 1;
  const int Y1 = min(MAP_Y_CELLS - 1, playerPos.y + AREA_RADI) - 1;

  for(int i = 0; i < NR_OF_SWEEPS; i++) {
    for(int y = Y0; y <= Y1; y++) {
      for(int x = X0; x <= X1; x++) {
        const Pos c(x, y);
        bool isAdjToWalkableCell = false;
        for(int dy = -1; dy <= 1; dy++) {
          for(int dx = -1; dx <= 1; dx++) {
            const FeatureStatic* const f =
              eng->map->featuresStatic[x + dx][y + dy];
            if(f->isBodyTypePassable(actorBodyType_normal)) {
              isAdjToWalkableCell = true;
            }
          }
        }
        if(isAdjToWalkableCell) {
          const int CHANCE_TO_DESTROY = 10;
          if(eng->dice.percentile() < CHANCE_TO_DESTROY) {
            eng->map->switchToDestroyedFeatAt(c);
          }
        }
      }
    }
  }

  for(int y = Y0; y <= Y1; y++) {
    for(int x = X0; x <= X1; x++) {
      if(eng->map->featuresStatic[x][y]->canHaveBlood()) {
        const int CHANCE_FOR_BLOOD = 10;
        if(eng->dice.percentile() < CHANCE_FOR_BLOOD) {
          eng->map->featuresStatic[x][y]->setHasBlood(true);
        }
      }
    }
  }

  for(unsigned int i = 0; i < eng->gameTime->getLoopSize(); i++) {
    Actor* actor = eng->gameTime->getActorAt(i);
    if(actor != eng->player) {
      if(eng->player->checkIfSeeActor(*actor, NULL)) {
        actor->getPropHandler()->tryApplyProp(
          new PropBurning(eng, propTurnsStandard));
      }
    }
  }

  Sound snd("", endOfSfx, true, eng->player->pos, true, true);
  eng->soundEmitter->emitSound(snd);

  return SpellCastRetData(true);
}

//------------------------------------------------------------ PESTILENCE
SpellCastRetData SpellPestilence::specificCast(
  Actor* const caster, Engine* const eng) {
  (void)caster;
  bool blockers[MAP_X_CELLS][MAP_Y_CELLS];
  eng->mapTests->makeMoveBlockerArrayForBodyType(actorBodyType_normal, blockers);

  const int RADI = 4;
  const int x0 = max(0, eng->player->pos.x - RADI);
  const int y0 = max(0, eng->player->pos.y - RADI);
  const int x1 = min(MAP_X_CELLS - 1, eng->player->pos.x + RADI);
  const int y1 = min(MAP_Y_CELLS - 1, eng->player->pos.y + RADI);

  ActorId_t monsterId = endOfActorIds;
  Dice& dice = eng->dice;

  vector<Pos> positions;

  for(int x = x0; x <= x1; x++) {
    for(int y = y0; y <= y1; y++) {
      if(blockers[x][y] == false) {
        positions.push_back(Pos(x, y));
      }
    }
  }

  eng->renderer->drawBlastAnimationAtPositionsWithPlayerVision(
    positions, clrMagenta);

  for(unsigned int i = 0; i < positions.size(); i++) {
    monsterId = actor_rat;
    if(dice(1, 3) == 1) {
      if(dice.coinToss()) {
        monsterId = actor_greenSpider;
      } else {
        monsterId = dice.coinToss() ? actor_whiteSpider : actor_redSpider;
      }
    }
    eng->actorFactory->spawnActor(monsterId, positions.at(i));
  }

  eng->log->addMsg("Disgusting critters appear around me!");
  return SpellCastRetData(true);
}

//------------------------------------------------------------ DETECT ITEMS
SpellCastRetData SpellDetectItems::specificCast(
  Actor* const caster, Engine* const eng) {
  (void)caster;

  const int RADI    = FOV_STANDARD_RADI_INT + 3;
  const int ORIG_X  = eng->player->pos.x;
  const int ORIG_Y  = eng->player->pos.y;
  const int X0      = max(0, ORIG_X - RADI);
  const int Y0      = max(0, ORIG_Y - RADI);
  const int X1      = min(MAP_X_CELLS - 1, ORIG_X + RADI);
  const int Y1      = min(MAP_Y_CELLS - 1, ORIG_Y + RADI);

  vector<Pos> itemsRevealedPositions;

  for(int y = Y0; y < Y1; y++) {
    for(int x = X0; x <= X1; x++) {
      Item* item = eng->map->items[x][y];
      if(item != NULL) {
        eng->map->playerVision[x][y] = true;
        eng->map->explored[x][y] = true;
        itemsRevealedPositions.push_back(Pos(x, y));
      }
    }
  }
  if(itemsRevealedPositions.empty() == false) {
    eng->renderer->drawMapAndInterface();
    eng->player->updateFov();
    eng->renderer->drawBlastAnimationAtPositions(
      itemsRevealedPositions, clrWhite);
    eng->renderer->drawMapAndInterface();

    if(itemsRevealedPositions.size() == 1) {
      eng->log->addMsg("An item is revealed to me.");
    }
    if(itemsRevealedPositions.size() > 1) {
      eng->log->addMsg("Some items are revealed to me.");
    }
    return SpellCastRetData(true);
  }
  return SpellCastRetData(false);
}

//------------------------------------------------------------ DETECT TRAPS
SpellCastRetData SpellDetectTraps::specificCast(
  Actor* const caster, Engine* const eng) {

  (void)caster;

  vector<Pos> trapsRevealedPositions;

  for(int x = 0; x < MAP_X_CELLS; x++) {
    for(int y = 0; y < MAP_Y_CELLS; y++) {
      if(eng->map->playerVision[x][y]) {
        FeatureStatic* const f = eng->map->featuresStatic[x][y];
        if(f->getId() == feature_trap) {
          Trap* const trap = dynamic_cast<Trap*>(f);
          trap->reveal(false);
          trapsRevealedPositions.push_back(Pos(x, y));
        }
      }
    }
  }

  if(trapsRevealedPositions.empty() == false) {
    eng->renderer->drawMapAndInterface();
    eng->player->updateFov();
    eng->renderer->drawBlastAnimationAtPositions(
      trapsRevealedPositions, clrWhite);
    eng->renderer->drawMapAndInterface();
    if(trapsRevealedPositions.size() == 1) {
      eng->log->addMsg("A hidden trap is revealed to me.");
    }
    if(trapsRevealedPositions.size() > 1) {
      eng->log->addMsg("Some hidden traps are revealed to me.");
    }
    return SpellCastRetData(true);
  }
  return SpellCastRetData(false);
}

//------------------------------------------------------------ CLAIRVOYANCE
SpellCastRetData SpellClairvoyance::specificCast(
  Actor* const caster, Engine* const eng) {
  (void)caster;
  eng->player->getPropHandler()->tryApplyProp(
    new PropClairvoyant(eng, propTurnsStandard), true, false);
  return SpellCastRetData(true);
}

//------------------------------------------------------------ OPENING
SpellCastRetData SpellOpening::specificCast(
  Actor* const caster, Engine* const eng) {

  (void)caster;

  vector<Pos> featuresOpenedPositions;

  for(int y = 1; y < MAP_Y_CELLS - 1; y++) {
    for(int x = 1; x < MAP_X_CELLS - 1; x++) {
      if(eng->map->playerVision[x][y]) {
        if(eng->map->featuresStatic[x][y]->openFeature()) {
          featuresOpenedPositions.push_back(Pos(x, y));
        }
      }
    }
  }

  if(featuresOpenedPositions.empty() == false) {
    eng->renderer->drawMapAndInterface();
    eng->player->updateFov();
    eng->renderer->drawBlastAnimationAtPositions(
      featuresOpenedPositions, clrWhite);
    eng->renderer->drawMapAndInterface();
//    if(featuresOpenedPositions.size() == 1) {
//      eng->log->addMsg("An object was opened.");
//    }
//    if(featuresOpenedPositions.size() > 1) {
//      eng->log->addMsg("Some objects were opened.");
//    }
    return SpellCastRetData(true);
  } else {
    return SpellCastRetData(false);
  }
}

//------------------------------------------------------------ SACRIFICE LIFE
SpellCastRetData SpellSacrificeLife::specificCast(
  Actor* const caster, Engine* const eng) {
  (void)caster;

  //Convert every 2 HP to 1 SPI

  const int PLAYER_HP_CUR = eng->player->getHp();

  if(PLAYER_HP_CUR > 2) {
    const int HP_DRAINED = ((PLAYER_HP_CUR - 1) / 2) * 2;
    eng->player->hit(HP_DRAINED, dmgType_pure, false);
    eng->player->restoreSpi(HP_DRAINED, true, true);
    return SpellCastRetData(true);
  }
  return SpellCastRetData(false);
}

//------------------------------------------------------------ SACRIFICE SPIRIT
SpellCastRetData SpellSacrificeSpirit::specificCast(
  Actor* const caster, Engine* const eng) {
  (void)caster;

  //Convert every SPI to HP

  const int PLAYER_SPI_CUR = eng->player->getSpi();

  if(PLAYER_SPI_CUR > 0) {
    const int HP_DRAINED = PLAYER_SPI_CUR - 1;
    eng->player->hitSpi(HP_DRAINED);
    eng->player->restoreHp(HP_DRAINED, true, true);
    return SpellCastRetData(true);
  }
  return SpellCastRetData(false);
}

//------------------------------------------------------------ MTH POWER
SpellCastRetData SpellMthPower::specificCast(
  Actor* const caster, Engine* const eng) {
  (void)caster;

  eng->log->addMsg("I summon chaotic forces...");

  if(doSpecialAction(eng) == false) {
    castRandomOtherSpell(eng);
  };

  return SpellCastRetData(true);
}

bool SpellMthPower::doSpecialAction(Engine* const eng) const {
  trace << "SpellMthPower::doSpecialAction()" << endl;

  if(eng->dice.coinToss()) {

    vector<Actor*> spotedEnemies;
    eng->player->getSpotedEnemies(spotedEnemies);

    const int MTH = eng->player->getMth();

    //Slay enemies
    if(eng->dice.oneIn(2) && MTH >= 35 && spotedEnemies.empty() == false) {
      vector<Pos> actorPositions;
      for(unsigned int i = 0; i < spotedEnemies.size(); i++) {
        actorPositions.push_back(spotedEnemies.at(i)->pos);
      }

      eng->renderer->drawBlastAnimationAtPositionsWithPlayerVision(
        actorPositions, clrYellow);

      for(unsigned int i = 0; i < spotedEnemies.size(); i++) {
        const string monsterName = spotedEnemies.at(i)->getNameThe();
        eng->log->addMsg(
          monsterName + " is crushed by an unseen force!", clrMessageGood);
        spotedEnemies.at(i)->hit(25, dmgType_physical, true);
      }

      eng->renderer->drawMapAndInterface(true);
      return true;
    }

    //Heal
    if(
      eng->dice.oneIn(3) &&
      eng->player->getHp() < eng->player->getHpMax(true)) {
      bool visionBlockers[MAP_X_CELLS][MAP_Y_CELLS];
      eng->mapTests->makeVisionBlockerArray(
        eng->player->pos, visionBlockers);
      eng->player->getPropHandler()->endAppliedProp(
        propDiseased, visionBlockers);
      eng->player->restoreHp(999, true);
      return true;
    }

    //Find stairs
    for(int y = 1; y < MAP_Y_CELLS; y++) {
      for(int x = 1; x < MAP_X_CELLS; x++) {
        if(
          eng->map->featuresStatic[x][y]->getId() == feature_stairsDown &&
          eng->map->explored[x][y] == false) {

          trace << "SpellMthPower: Find stairs" << endl;
          for(int dy = -1; dy <= 1; dy++) {
            for(int dx = -1; dx <= 1; dx++) {
              eng->map->playerVision[x + dx][y + dy] = true;
              eng->map->explored[x + dx][y + dy] = true;
            }
          }
          eng->log->addMsg("The way forward is revealed!");
          eng->renderer->drawMapAndInterface(true);
          eng->player->updateFov();
          eng->renderer->drawMapAndInterface(true);
          return true;
        }
      }
    }

    //Improve weapon
    Item* const item =
      eng->player->getInventory()->getItemInSlot(slot_wielded);
    if(item != NULL) {
      const ItemData& d = item->getData();
      if(d.isMeleeWeapon && d.isRangedWeapon == false) {
        Weapon* const weapon = dynamic_cast<Weapon*>(item);
        if(weapon->meleeDmgPlus == 0) {
          eng->log->addMsg("My weapon is deadlier!");
          weapon->meleeDmgPlus++;
          return true;
        }
      }
    }
  }
  trace << "SpellMthPower: No special action taken" << endl;
  return false;
}

void SpellMthPower::castRandomOtherSpell(Engine* const eng) const {
  trace << "SpellMthPower::castRandomOtherSpell()..." << endl;
  vector<Spell*> spellCandidates;
  for(int i = 0; i < endOfSpells; i++) {
    if(i != spell_mthPower) {
      Spell* const spell = eng->spellHandler->getSpellFromId(Spell_t(i));
      if(spell->isLearnableForPlayer()) {
        spellCandidates.push_back(spell);
      } else {
        delete spell;
      }
    }
  }

  Spell* spellToCast = NULL;
  const int ELEMENT = eng->dice.range(0, spellCandidates.size() - 1);
  spellToCast = spellCandidates.at(ELEMENT);

  spellToCast->cast(eng->player, false, eng);
  trace << "SpellMthPower::castRandomOtherSpell() [DONE]" << endl;
}

//------------------------------------------------------------ BLESS
SpellCastRetData SpellBless::specificCast(
  Actor* const caster, Engine* const eng) {

  caster->getPropHandler()->tryApplyProp(
    new PropBlessed(eng, propTurnsStandard));

  return SpellCastRetData(true);
}

bool SpellBless::isGoodForMonsterToCastNow(
  Monster* const monster, Engine* const eng) {
  (void)eng;
  return monster->getPropHandler()->hasProp(propBlessed) == false;
}

//------------------------------------------------------------ TELEPORT
SpellCastRetData SpellTeleport::specificCast(
  Actor* const caster, Engine* const eng) {

  if(caster != eng->player) {
    if(eng->player->checkIfSeeActor(*caster, NULL)) {
      eng->log->addMsg(
        caster->getNameThe() + " dissapears in a blast of smoke!");
    }
  }

  caster->teleport(false);
  return SpellCastRetData(true);
}

bool SpellTeleport::isGoodForMonsterToCastNow(
  Monster* const monster, Engine* const eng) {
  bool blockers[MAP_X_CELLS][MAP_Y_CELLS];
  eng->mapTests->makeVisionBlockerArray(monster->pos, blockers);
  return monster->checkIfSeeActor(*(eng->player), blockers)  &&
         monster->getHp() <= (monster->getHpMax(true) / 2)      &&
         eng->dice.coinToss();
}

//------------------------------------------------------------ KNOCKBACK
SpellCastRetData SpellKnockBack::specificCast(
  Actor* const caster, Engine* const eng) {
  if(caster == eng->player) {

  } else {
    eng->log->addMsg("A force pushes me!", clrMessageBad);
    eng->knockBack->tryKnockBack(eng->player, caster->pos, false);
  }
  return SpellCastRetData(false);
}

bool SpellKnockBack::isGoodForMonsterToCastNow(
  Monster* const monster, Engine* const eng) {
  bool blockers[MAP_X_CELLS][MAP_Y_CELLS];
  eng->mapTests->makeVisionBlockerArray(monster->pos, blockers);
  return monster->checkIfSeeActor(*(eng->player), blockers);
}

//------------------------------------------------------------ ENFEEBLE
SpellCastRetData SpellEnfeeble::specificCast(
  Actor* const caster, Engine* const eng) {

  const PropId_t propId = getPropId(eng);

  if(caster == eng->player) {
    vector<Actor*> spotedEnemies;
    eng->player->getSpotedEnemies(spotedEnemies);

    if(spotedEnemies.empty()) {
      return SpellCastRetData(false);
    } else {
      vector<Pos> actorPositions;
      actorPositions.resize(0);

      for(unsigned int i = 0; i < spotedEnemies.size(); i++) {
        actorPositions.push_back(spotedEnemies.at(i)->pos);
      }

      eng->renderer->drawBlastAnimationAtPositionsWithPlayerVision(
        actorPositions, clrMagenta);

      for(unsigned int i = 0; i < spotedEnemies.size(); i++) {
        PropHandler* const propHandler = spotedEnemies.at(i)->getPropHandler();
        Prop* const prop = propHandler->makePropFromId(
                             propId, propTurnsStandard);
        propHandler->tryApplyProp(prop);
      }
      return SpellCastRetData(true);
    }
  } else {
    eng->renderer->drawBlastAnimationAtPositionsWithPlayerVision(
      vector<Pos>(1, eng->player->pos), clrMagenta);

    PropHandler* const propHandler = eng->player->getPropHandler();
    Prop* const prop = propHandler->makePropFromId(
                         propId, propTurnsStandard);
    eng->player->getPropHandler()->tryApplyProp(prop);

    return SpellCastRetData(false);
  }
}

bool SpellEnfeeble::isGoodForMonsterToCastNow(
  Monster* const monster, Engine* const eng) {
  bool blockers[MAP_X_CELLS][MAP_Y_CELLS];
  eng->mapTests->makeVisionBlockerArray(monster->pos, blockers);
  return monster->checkIfSeeActor(*(eng->player), blockers);
}

PropId_t SpellEnfeeble::getPropId(Engine* const eng) const {
  const int RND = eng->dice.range(1, 5);
  switch(RND) {
    case 1: {return propConfused;}
    case 2: {return propParalysed;}
    case 3: {return propSlowed;}
    case 4: {return propBlind;}
    case 5: {return propTerrified;}
  }
  return endOfPropIds;
}

//------------------------------------------------------------ CONFUSE
//SpellCastRetData SpellConfuse::specificCast(Actor* const caster, Engine* const eng) {
//  Actor* actor = eng->mapTests->getActorAtPos(d.targetCell_);
//
//  if(actor != NULL) {
//
//    if(actor == eng->player) {
//      eng->log->addMsg("My mind is reeling!");
//    }
//
//    actor->getPropHandler()->tryApplyProp(new StatusConfused(eng));
//  }
//}
//
//void SpellConfuse::specificMonsterCast(
//  Monster* const monster, Engine* const eng) {
//  specificCast(SpellCastData(monster, eng->player->pos), eng);
//}
//
//bool SpellConfuse::isGoodForMonsterToCastNow(
//  Monster* const monster, Engine* const eng) {
//  bool blockers[MAP_X_CELLS][MAP_Y_CELLS];
//  engine->mapTests->makeVisionBlockerArray(monster->pos, blockers);
//  if(monster->checkIfSeeActor(*(engine->player), blockers)) {
//    return engine->player->getPropHandler()->allowSee();
//  }
//  return false;
//}

//------------------------------------------------------------ WEAKNESS
//SpellCastRetData SpellWeakness::specificCast(Actor* const caster, Engine* const eng) {
//  Actor* actor = eng->mapTests->getActorAtPos(d.targetCell_);
//
//  if(actor != NULL) {
//
//    if(actor == eng->player) {
//      eng->log->addMsg("Something is draining me physically!");
//    }
//
//    actor->getPropHandler()->tryApplyProp(new StatusWeak(eng));
//  }
//}
//
//void SpellWeakness::specificMonsterCast(
//  Monster* const monster, Engine* const eng) {
//  specificCast(SpellCastData(monster, eng->player->pos), eng);
//}
//
//bool SpellWeakness::isGoodForMonsterToCastNow(
//  Monster* const monster, Engine* const eng) {
//  bool blockers[MAP_X_CELLS][MAP_Y_CELLS];
//  engine->mapTests->makeVisionBlockerArray(monster->pos, blockers);
//  if(monster->checkIfSeeActor(*(engine->player), blockers)) {
//    return engine->player->getPropHandler()->allowSee();
//  }
//  return false;
//}

//------------------------------------------------------------ BLIND
//SpellCastRetData SpellBlind::specificCast(Actor* const caster, Engine* const eng) {
//  Actor* actor = eng->mapTests->getActorAtPos(d.targetCell_);
//
//  if(actor != NULL) {
//
//    if(actor == eng->player) {
//      eng->log->addMsg("Scales starts to grow over my eyes!");
//    }
//
//    actor->getPropHandler()->tryApplyProp(new PropBlind(eng->dice(3, 6)));
//  }
//}
//
//void SpellBlind::specificMonsterCast(
//  Monster* const monster, Engine* const eng) {
//  specificCast(SpellCastData(monster, eng->player->pos), eng);
//}
//
//bool SpellBlind::isGoodForMonsterToCastNow(
//  Monster* const monster, Engine* const eng) {
//  bool blockers[MAP_X_CELLS][MAP_Y_CELLS];
//  engine->mapTests->makeVisionBlockerArray(monster->pos, blockers);
//  if(monster->checkIfSeeActor(*(engine->player), blockers)) {
//    return engine->player->getPropHandler()->allowSee() ;
//  }
//  return false;
//}

//------------------------------------------------------------ FEAR
//SpellCastRetData SpellFear::specificCast(Actor* const caster, Engine* const eng) {
//  Actor* actor = eng->mapTests->getActorAtPos(d.targetCell_);
//
//  if(actor != NULL) {
//
//    if(actor == eng->player) {
//      eng->log->addMsg("My mind is besieged by terror.");
//    }
//
//    actor->getPropHandler()->tryApplyProp(new PropTerrified(eng->dice(3, 6)));
//  }
//}
//
//void SpellFear::specificMonsterCast(
//  Monster* const monster, Engine* const eng) {
//  specificCast(SpellCastData(monster, eng->player->pos), eng);
//}
//
//bool SpellFear::isGoodForMonsterToCastNow(
//  Monster* const monster, Engine* const eng) {
//  bool blockers[MAP_X_CELLS][MAP_Y_CELLS];
//  engine->mapTests->makeVisionBlockerArray(monster->pos, blockers);
//  return monster->checkIfSeeActor(*(engine->player), blockers);
//}

//------------------------------------------------------------ SLOW
//SpellCastRetData SpellSlow::specificCast(Actor* const caster, Engine* const eng) {
//  Actor* actor = eng->mapTests->getActorAtPos(d.targetCell_);
//
//  if(actor != NULL) {
//
//    if(actor == eng->player) {
//      eng->log->addMsg("I start to feel bogged down.");
//    }
//
//    actor->getPropHandler()->tryApplyProp(new StatusSlowed(eng->dice(3, 6)));
//  }
//}
//
//void SpellSlow::specificMonsterCast(
//  Monster* const monster, Engine* const eng) {
//  specificCast(SpellCastData(monster, eng->player->pos), eng);
//}
//
//bool SpellSlow::isGoodForMonsterToCastNow(
//  Monster* const monster, Engine* const eng) {
//  bool blockers[MAP_X_CELLS][MAP_Y_CELLS];
//  engine->mapTests->makeVisionBlockerArray(monster->pos, blockers);
//  return monster->checkIfSeeActor(*(engine->player), blockers);
//}

//------------------------------------------------------------ DISEASE
SpellCastRetData SpellDisease::specificCast(
  Actor* const caster, Engine* const eng) {
  if(caster == eng->player) {
    return SpellCastRetData(true);
  } else {
    eng->log->addMsg(
      "A disease is starting to afflict my body!", clrMessageBad);
    eng->player->getPropHandler()->tryApplyProp(
      new PropDiseased(eng, propTurnsStandard));
    return SpellCastRetData(false);
  }
}

bool SpellDisease::isGoodForMonsterToCastNow(
  Monster* const monster, Engine* const eng) {
  bool blockers[MAP_X_CELLS][MAP_Y_CELLS];
  eng->mapTests->makeVisionBlockerArray(monster->pos, blockers);
  return monster->checkIfSeeActor(*(eng->player), blockers);
}

//------------------------------------------------------------ SUMMON RANDOM
SpellCastRetData SpellSummonRandom::specificCast(
  Actor* const caster, Engine* const eng) {

  Pos summonPos(caster->pos);

  vector<Pos> freePositionsSeenByPlayer;
  const int RADI = FOV_STANDARD_RADI_INT;
  const Pos playerPos(eng->player->pos);
  const int X0 = max(0, playerPos.x - RADI);
  const int Y0 = max(0, playerPos.y - RADI);
  const int X1 = min(MAP_X_CELLS, playerPos.x + RADI) - 1;
  const int Y1 = min(MAP_Y_CELLS, playerPos.y + RADI) - 1;
  for(int x = X0; x <= X1; x++) {
    for(int y = Y0; y <= Y1; y++) {
      if(eng->map->playerVision[x][y]) {
        freePositionsSeenByPlayer.push_back(Pos(x, y));
      }
    }
  }

  bool blockers[MAP_X_CELLS][MAP_Y_CELLS];
  eng->mapTests->makeMoveBlockerArrayForBodyType(actorBodyType_normal, blockers);

  for(int i = 0; i < int(freePositionsSeenByPlayer.size()); i++) {
    const Pos pos(freePositionsSeenByPlayer.at(i));
    if(blockers[pos.x][pos.y]) {
      freePositionsSeenByPlayer.erase(freePositionsSeenByPlayer.begin() + i);
      i--;
    }
  }

  if(freePositionsSeenByPlayer.empty()) {
    vector<Pos> freeCellsVector;
    eng->mapTests->makeBoolVectorFromMapArray(blockers, freeCellsVector);
    if(freeCellsVector.empty() == false) {
      sort(freeCellsVector.begin(), freeCellsVector.end(),
           IsCloserToOrigin(caster->pos, eng));
      summonPos = freeCellsVector.at(0);
    }
  } else {
    const int ELEMENT =
      eng->dice.range(0, freePositionsSeenByPlayer.size() - 1);
    summonPos = freePositionsSeenByPlayer.at(ELEMENT);
  }

  vector<ActorId_t> summonCandidates;
  for(int i = 1; i < endOfActorIds; i++) {
    const ActorData& data = eng->actorDataHandler->dataList[i];
    if(data.canBeSummoned) {
      if(data.spawnMinDLVL <= caster->getData()->spawnMinDLVL) {
        summonCandidates.push_back(ActorId_t(i));
      }
    }
  }
  const int ELEMENT = eng->dice.range(1, summonCandidates.size() - 1);
  const ActorId_t id = summonCandidates.at(ELEMENT);
  Actor* const actor = eng->actorFactory->spawnActor(id, summonPos);
  Monster* monster = dynamic_cast<Monster*>(actor);
  monster->playerAwarenessCounter = monster->getData()->nrTurnsAwarePlayer;
  if(eng->map->playerVision[summonPos.x][summonPos.y]) {
    eng->log->addMsg(monster->getNameA() + " appears.");
  }
  return SpellCastRetData(false);
}

bool SpellSummonRandom::isGoodForMonsterToCastNow(
  Monster* const monster, Engine* const eng) {

  bool blockers[MAP_X_CELLS][MAP_Y_CELLS];
  eng->mapTests->makeVisionBlockerArray(monster->pos, blockers);
  const bool IS_PLAYER_SEEN =
    monster->checkIfSeeActor(*(eng->player), blockers);
  return IS_PLAYER_SEEN || (eng->dice.percentile() < 5);
}

//------------------------------------------------------------ HEAL SELF
SpellCastRetData SpellHealSelf::specificCast(
  Actor* const caster, Engine* const eng) {

  (void)eng;
  return SpellCastRetData(caster->restoreHp(999, true));
}

bool SpellHealSelf::isGoodForMonsterToCastNow(
  Monster* const monster, Engine* const eng) {

  (void)eng;
  return monster->getHp() < monster->getHpMax(true);
}
