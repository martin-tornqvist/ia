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
#include "MapParsing.h"
#include "LineCalc.h"
#include "SdlWrapper.h"

Spell* SpellHandler::getRandomSpellForMonster() {
  vector<Spell_t> candidates;
  for(int i = 0; i < endOfSpells; i++) {
    Spell* const spell = getSpellFromId(Spell_t(i));
    if(spell->isAvailForAllMonsters()) {
      candidates.push_back(Spell_t(i));
    }
    delete spell;
  }
  const int ELEMENT = eng.dice.range(0, candidates.size() - 1);
  return getSpellFromId(candidates.at(ELEMENT));
}

Spell* SpellHandler::getSpellFromId(const Spell_t spellId) const {
  switch(spellId) {
    case spell_enfeeble:            return new SpellEnfeeble;
    case spell_disease:             return new SpellDisease;
    case spell_azathothsWrath:      return new SpellAzathothsWrath;
    case spell_summonRandom:        return new SpellSummonRandom;
    case spell_healSelf:            return new SpellHealSelf;
    case spell_knockBack:           return new SpellKnockBack;
    case spell_teleport:            return new SpellTeleport;
    case spell_mayhem:              return new SpellMayhem;
    case spell_pestilence:          return new SpellPestilence;
    case spell_detectItems:         return new SpellDetectItems;
    case spell_detectTraps:         return new SpellDetectTraps;
    case spell_clairvoyance:        return new SpellClairvoyance;
    case spell_opening:             return new SpellOpening;
    case spell_sacrificeLife:       return new SpellSacrificeLife;
    case spell_sacrificeSpirit:     return new SpellSacrificeSpirit;
    case spell_rogueHide:           return new SpellRogueHide;
    case spell_mthPower:            return new SpellMthPower;
    case spell_bless:               return new SpellBless;
    case spell_miGoHypnosis:        return new SpellMiGoHypnosis;

    case endOfSpells: {} break;
  }
  trace << "[WARNING] Found no spell for ID: " << spellId;
  trace << ", in SpellHandler::getSpellFromId()" << endl;
  return NULL;
}

Range Spell::getSpiCost(const bool IS_BASE_COST_ONLY, Actor* const caster,
                        Engine& eng) const {
  int costMax = getSpecificMaxSpiCost();

  if(IS_BASE_COST_ONLY == false) {
    const int X0 = max(0, caster->pos.x - 1);
    const int Y0 = max(0, caster->pos.y - 1);
    const int X1 = min(MAP_W - 1, caster->pos.x + 1);
    const int Y1 = min(MAP_H - 1, caster->pos.y + 1);

    for(int y = Y0; y <= Y1; y++) {
      for(int x = X0; x <= X1; x++) {
        if(eng.map->cells[x][y].featureStatic->getId() == feature_altar) {
          costMax -= 1;
          y = 9999;
          x = 9999;
        }
      }
    }

    PropHandler& propHandler = caster->getPropHandler();

    if(propHandler.hasProp(propBlessed))  {costMax -= 1;}
    if(propHandler.allowSee() == false)   {costMax -= 1;}
    if(propHandler.hasProp(propCursed))   {costMax += 3;}

    if(caster == eng.player && eng.player->getMth() >= MTH_LVL_SPELLS_SPI_BON) {
      costMax--;
    }
  }

  costMax             = max(1, costMax);
  const int COST_MIN  = max(1, costMax / 2);

  return Range(COST_MIN, costMax);
}

SpellCastRetData Spell::cast(Actor* const caster, const bool IS_INTRINSIC,
                             Engine& eng) {
  trace << "Spell::cast()..." << endl;
  if(caster->getPropHandler().allowCastSpells(true)) {
    if(caster == eng.player) {
      trace << "Spell: Player casting spell" << endl;
      eng.player->incrShock(SHOCK_TAKEN_FROM_CASTING_SPELLS);
    } else {
      trace << "Spell: Monster casting spell" << endl;
      Monster* const monster = dynamic_cast<Monster*>(caster);
      if(eng.map->cells[monster->pos.x][monster->pos.y].isSeenByPlayer) {
        const string spellStr = monster->getData().spellCastMessage;
        eng.log->addMsg(spellStr);
      }
      monster->spellCoolDownCurrent = monster->getData().spellCooldownTurns;
    }

    if(IS_INTRINSIC) {
      const Range cost = getSpiCost(false, caster, eng);
      caster->hitSpi(eng.dice.range(cost));
    }
    SpellCastRetData ret(false);
    if(caster->deadState == actorDeadState_alive) {
      ret = specificCast(caster, eng);
    }

    eng.gameTime->actorDidAct();
    trace << "Spell::cast() [DONE]" << endl;
    return ret;
  }
  trace << "Spell::cast() [DONE]" << endl;
  return SpellCastRetData(false);
}

//------------------------------------------------------------ AZATHOTHS WRATH
SpellCastRetData SpellAzathothsWrath::specificCast(
  Actor* const caster, Engine& eng) {

  Actor* target = NULL;

  //If player casting, try to use player's current target instead of random
  const bool IS_PLAYER_CASTING = caster == eng.player;
  if(IS_PLAYER_CASTING) {
    if(eng.player->target != NULL) {
      if(eng.player->checkIfSeeActor(*eng.player->target, NULL)) {
        target = eng.player->target;
      }
    }
  }

  if(target == NULL) {
    vector<Actor*> targetCandidates;
    caster->getSpottedEnemies(targetCandidates);
    if(targetCandidates.empty()) {
      return SpellCastRetData(false);
    } else {
      const int ELEMENT = eng.dice.range(0, targetCandidates.size() - 1);
      target = targetCandidates.at(ELEMENT);
      if(IS_PLAYER_CASTING) {eng.player->target = target;}
    }
  }

  vector<Pos> line;
  eng.lineCalc->calcNewLine(caster->pos, target->pos, true, 999, false, line);
  eng.renderer->drawMapAndInterface();
  const int LINE_SIZE = line.size();
  for(int i = 1; i < LINE_SIZE; i++) {
    const Pos& pos = line.at(i);
    if(eng.config->isTilesMode) {
      eng.renderer->drawTile(tile_blastAnimation1, panel_map, pos, clrRedLgt);
    } else {
      eng.renderer->drawGlyph('*', panel_map, pos, clrRedLgt);
    }
    eng.renderer->updateScreen();
    eng.sdlWrapper->sleep(eng.config->delayProjectileDraw);
  }

  eng.renderer->drawBlastAnimationAtPositions(
    vector<Pos> {target->pos}, clrRedLgt);

  const string msgCmn = " struck by a roaring blast!";
  if(IS_PLAYER_CASTING) {
    eng.log->addMsg(target->getNameThe() + " is" + msgCmn, clrMsgGood);
  } else {
    eng.log->addMsg("I am" + msgCmn, clrMsgBad);
  }

  target->getPropHandler().tryApplyProp(
    new PropParalyzed(eng, propTurnsSpecified, 1));
  target->hit(eng.dice.range(1, 10), dmgType_physical, true);

  return SpellCastRetData(true);
}

bool SpellAzathothsWrath::isGoodForMonsterToCastNow(
  Monster* const monster, Engine& eng) {
  bool blockers[MAP_W][MAP_H];
  MapParser::parse(CellPredBlocksVision(eng), blockers);
  return monster->checkIfSeeActor(*(eng.player), blockers);
}

//------------------------------------------------------------ MAYHEM
SpellCastRetData SpellMayhem::specificCast(
  Actor* const caster, Engine& eng) {
  (void)caster;

  eng.log->addMsg("Destruction rages around me!");

  const Pos& playerPos = eng.player->pos;

  const int NR_OF_SWEEPS  = 5;
  const int AREA_RADI     = FOV_STD_RADI_INT + 1;

  const int X0 = max(1, playerPos.x - AREA_RADI);
  const int Y0 = max(1, playerPos.y - AREA_RADI);
  const int X1 = min(MAP_W - 1, playerPos.x + AREA_RADI) - 1;
  const int Y1 = min(MAP_H - 1, playerPos.y + AREA_RADI) - 1;

  for(int i = 0; i < NR_OF_SWEEPS; i++) {
    for(int y = Y0; y <= Y1; y++) {
      for(int x = X0; x <= X1; x++) {
        const Pos c(x, y);
        bool isAdjToWalkableCell = false;
        for(int dy = -1; dy <= 1; dy++) {
          for(int dx = -1; dx <= 1; dx++) {
            const FeatureStatic* const f =
              eng.map->cells[x + dx][y + dy].featureStatic;
            if(f->isBodyTypePassable(bodyType_normal)) {
              isAdjToWalkableCell = true;
            }
          }
        }
        if(isAdjToWalkableCell) {
          const int CHANCE_TO_DESTROY = 10;
          if(eng.dice.percentile() < CHANCE_TO_DESTROY) {
            eng.map->switchToDestroyedFeatAt(c);
          }
        }
      }
    }
  }

  for(int y = Y0; y <= Y1; y++) {
    for(int x = X0; x <= X1; x++) {
      FeatureStatic* const f =
        eng.map->cells[x][y].featureStatic;
      if(f->canHaveBlood()) {
        const int CHANCE_FOR_BLOOD = 10;
        if(eng.dice.percentile() < CHANCE_FOR_BLOOD) {
          f->setHasBlood(true);
        }
      }
    }
  }

  const int NR_ACTORS = eng.gameTime->getNrActors();
  for(int i = 0; i < NR_ACTORS; i++) {
    Actor& actor = eng.gameTime->getActorAtElement(i);
    if(&actor != eng.player) {
      if(eng.player->checkIfSeeActor(actor, NULL)) {
        actor.getPropHandler().tryApplyProp(
          new PropBurning(eng, propTurnsStandard));
      }
    }
  }

  Sound snd("", endOfSfx, true, eng.player->pos, true, true);
  eng.soundEmitter->emitSound(snd);

  return SpellCastRetData(true);
}

//------------------------------------------------------------ PESTILENCE
SpellCastRetData SpellPestilence::specificCast(
  Actor* const caster, Engine& eng) {
  (void)caster;
  bool blockers[MAP_W][MAP_H];
  MapParser::parse(CellPredBlocksBodyType(bodyType_normal, true, eng),
                   blockers);

  const int RADI = 4;
  const int x0 = max(0, eng.player->pos.x - RADI);
  const int y0 = max(0, eng.player->pos.y - RADI);
  const int x1 = min(MAP_W - 1, eng.player->pos.x + RADI);
  const int y1 = min(MAP_H - 1, eng.player->pos.y + RADI);

  ActorId_t monsterId = endOfActorIds;
  Dice& dice = eng.dice;

  vector<Pos> positions;

  for(int x = x0; x <= x1; x++) {
    for(int y = y0; y <= y1; y++) {
      if(blockers[x][y] == false) {
        positions.push_back(Pos(x, y));
      }
    }
  }

  eng.renderer->drawBlastAnimationAtPositionsWithPlayerVision(
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
    eng.actorFactory->spawnActor(monsterId, positions.at(i));
  }

  eng.log->addMsg("Disgusting critters appear around me!");
  return SpellCastRetData(true);
}

//------------------------------------------------------------ DETECT ITEMS
SpellCastRetData SpellDetectItems::specificCast(
  Actor* const caster, Engine& eng) {
  (void)caster;

  const int RADI    = FOV_STD_RADI_INT + 3;
  const int ORIG_X  = eng.player->pos.x;
  const int ORIG_Y  = eng.player->pos.y;
  const int X0      = max(0, ORIG_X - RADI);
  const int Y0      = max(0, ORIG_Y - RADI);
  const int X1      = min(MAP_W - 1, ORIG_X + RADI);
  const int Y1      = min(MAP_H - 1, ORIG_Y + RADI);

  vector<Pos> itemsRevealedPositions;

  for(int y = Y0; y < Y1; y++) {
    for(int x = X0; x <= X1; x++) {
      Item* item = eng.map->cells[x][y].item;
      if(item != NULL) {
        eng.map->cells[x][y].isSeenByPlayer = true;
        eng.map->cells[x][y].isExplored = true;
        itemsRevealedPositions.push_back(Pos(x, y));
      }
    }
  }
  if(itemsRevealedPositions.empty() == false) {
    eng.renderer->drawMapAndInterface();
    eng.player->updateFov();
    eng.renderer->drawBlastAnimationAtPositions(
      itemsRevealedPositions, clrWhite);
    eng.renderer->drawMapAndInterface();

    if(itemsRevealedPositions.size() == 1) {
      eng.log->addMsg("An item is revealed to me.");
    }
    if(itemsRevealedPositions.size() > 1) {
      eng.log->addMsg("Some items are revealed to me.");
    }
    return SpellCastRetData(true);
  }
  return SpellCastRetData(false);
}

//------------------------------------------------------------ DETECT TRAPS
SpellCastRetData SpellDetectTraps::specificCast(
  Actor* const caster, Engine& eng) {

  (void)caster;

  vector<Pos> trapsRevealedPositions;

  for(int x = 0; x < MAP_W; x++) {
    for(int y = 0; y < MAP_H; y++) {
      if(eng.map->cells[x][y].isSeenByPlayer) {
        FeatureStatic* const f = eng.map->cells[x][y].featureStatic;
        if(f->getId() == feature_trap) {
          Trap* const trap = dynamic_cast<Trap*>(f);
          trap->reveal(false);
          trapsRevealedPositions.push_back(Pos(x, y));
        }
      }
    }
  }

  if(trapsRevealedPositions.empty() == false) {
    eng.renderer->drawMapAndInterface();
    eng.player->updateFov();
    eng.renderer->drawBlastAnimationAtPositions(
      trapsRevealedPositions, clrWhite);
    eng.renderer->drawMapAndInterface();
    if(trapsRevealedPositions.size() == 1) {
      eng.log->addMsg("A hidden trap is revealed to me.");
    }
    if(trapsRevealedPositions.size() > 1) {
      eng.log->addMsg("Some hidden traps are revealed to me.");
    }
    return SpellCastRetData(true);
  }
  return SpellCastRetData(false);
}

//------------------------------------------------------------ CLAIRVOYANCE
SpellCastRetData SpellClairvoyance::specificCast(
  Actor* const caster, Engine& eng) {
  (void)caster;
  eng.player->getPropHandler().tryApplyProp(
    new PropClairvoyant(eng, propTurnsStandard), true, false);
  return SpellCastRetData(true);
}

//------------------------------------------------------------ OPENING
SpellCastRetData SpellOpening::specificCast(
  Actor* const caster, Engine& eng) {

  (void)caster;

  vector<Pos> featuresOpenedPositions;

  for(int y = 1; y < MAP_H - 1; y++) {
    for(int x = 1; x < MAP_W - 1; x++) {
      if(eng.map->cells[x][y].isSeenByPlayer) {
        if(eng.map->cells[x][y].featureStatic->openFeature()) {
          featuresOpenedPositions.push_back(Pos(x, y));
        }
      }
    }
  }

  if(featuresOpenedPositions.empty() == false) {
    eng.renderer->drawMapAndInterface();
    eng.player->updateFov();
    eng.renderer->drawBlastAnimationAtPositions(
      featuresOpenedPositions, clrWhite);
    eng.renderer->drawMapAndInterface();
//    if(featuresOpenedPositions.size() == 1) {
//      eng.log->addMsg("An object was opened.");
//    }
//    if(featuresOpenedPositions.size() > 1) {
//      eng.log->addMsg("Some objects were opened.");
//    }
    return SpellCastRetData(true);
  } else {
    return SpellCastRetData(false);
  }
}

//------------------------------------------------------------ SACRIFICE LIFE
SpellCastRetData SpellSacrificeLife::specificCast(
  Actor* const caster, Engine& eng) {
  (void)caster;

  //Convert every 2 HP to 1 SPI

  const int PLAYER_HP_CUR = eng.player->getHp();

  if(PLAYER_HP_CUR > 2) {
    const int HP_DRAINED = ((PLAYER_HP_CUR - 1) / 2) * 2;
    eng.player->hit(HP_DRAINED, dmgType_pure, false);
    eng.player->restoreSpi(HP_DRAINED, true, true);
    return SpellCastRetData(true);
  }
  return SpellCastRetData(false);
}

//------------------------------------------------------------ SACRIFICE SPIRIT
SpellCastRetData SpellSacrificeSpirit::specificCast(
  Actor* const caster, Engine& eng) {
  (void)caster;

  //Convert every SPI to HP

  const int PLAYER_SPI_CUR = eng.player->getSpi();

  if(PLAYER_SPI_CUR > 0) {
    const int HP_DRAINED = PLAYER_SPI_CUR - 1;
    eng.player->hitSpi(HP_DRAINED);
    eng.player->restoreHp(HP_DRAINED, true, true);
    return SpellCastRetData(true);
  }
  return SpellCastRetData(false);
}

//------------------------------------------------------------ ROGUE HIDE
SpellCastRetData SpellRogueHide::specificCast(
  Actor* const caster, Engine& eng) {

  (void)caster;
  eng.log->addMsg("I am unseen.");

  const int NR_ACTORS = eng.gameTime->getNrActors();
  for(int i = 0; i < NR_ACTORS; i++) {
    Actor* const actor = &eng.gameTime->getActorAtElement(i);
    if(actor != eng.player) {
      Monster* const monster = dynamic_cast<Monster*>(actor);
      monster->playerAwarenessCounter = 0;
    }
  }
  return SpellCastRetData(true);
}

//------------------------------------------------------------ MTH POWER
SpellCastRetData SpellMthPower::specificCast(
  Actor* const caster, Engine& eng) {
  (void)caster;

  eng.log->addMsg("I summon chaotic forces...");

  if(doSpecialAction(eng) == false) {
    castRandomOtherSpell(eng);
  };

  return SpellCastRetData(true);
}

bool SpellMthPower::doSpecialAction(Engine& eng) const {
  trace << "SpellMthPower::doSpecialAction()" << endl;

  if(eng.dice.coinToss()) {

    vector<Actor*> SpottedEnemies;
    eng.player->getSpottedEnemies(SpottedEnemies);

    const int MTH = eng.player->getMth();

    //Slay enemies
    if(eng.dice.oneIn(2) && MTH >= 35 && SpottedEnemies.empty() == false) {
      vector<Pos> actorPositions;
      for(unsigned int i = 0; i < SpottedEnemies.size(); i++) {
        actorPositions.push_back(SpottedEnemies.at(i)->pos);
      }

      eng.renderer->drawBlastAnimationAtPositionsWithPlayerVision(
        actorPositions, clrYellow);

      for(unsigned int i = 0; i < SpottedEnemies.size(); i++) {
        const string monsterName = SpottedEnemies.at(i)->getNameThe();
        eng.log->addMsg(
          monsterName + " is crushed by an unseen force!", clrMsgGood);
        SpottedEnemies.at(i)->hit(25, dmgType_physical, true);
      }

      eng.renderer->drawMapAndInterface(true);
      return true;
    }

    //Heal
    if(
      eng.dice.oneIn(3) &&
      eng.player->getHp() < eng.player->getHpMax(true)) {
      bool visionBlockers[MAP_W][MAP_H];
      MapParser::parse(CellPredBlocksVision(eng), visionBlockers);
      eng.player->getPropHandler().endAppliedProp(
        propDiseased, visionBlockers);
      eng.player->restoreHp(999, true);
      return true;
    }

    //Find stairs
    for(int y = 1; y < MAP_H; y++) {
      for(int x = 1; x < MAP_W; x++) {
        if(
          eng.map->cells[x][y].featureStatic->getId() == feature_stairsDown &&
          eng.map->cells[x][y].isExplored == false) {

          trace << "SpellMthPower: Find stairs" << endl;
          for(int dy = -1; dy <= 1; dy++) {
            for(int dx = -1; dx <= 1; dx++) {
              eng.map->cells[x + dx][y + dy].isSeenByPlayer = true;
              eng.map->cells[x + dx][y + dy].isExplored = true;
            }
          }
          eng.log->addMsg("The way forward is revealed!");
          eng.renderer->drawMapAndInterface(true);
          eng.player->updateFov();
          eng.renderer->drawMapAndInterface(true);
          return true;
        }
      }
    }

    //Improve weapon
    Item* const item =
      eng.player->getInv().getItemInSlot(slot_wielded);
    if(item != NULL) {
      const ItemData& d = item->getData();
      if(d.isMeleeWeapon && d.isRangedWeapon == false) {
        Weapon* const weapon = dynamic_cast<Weapon*>(item);
        if(weapon->meleeDmgPlus == 0) {
          eng.log->addMsg("My weapon is deadlier!");
          weapon->meleeDmgPlus++;
          return true;
        }
      }
    }
  }
  trace << "SpellMthPower: No special action taken" << endl;
  return false;
}

void SpellMthPower::castRandomOtherSpell(Engine& eng) const {
  trace << "SpellMthPower::castRandomOtherSpell()..." << endl;
  vector<Spell*> spellCandidates;
  for(int i = 0; i < endOfSpells; i++) {
    if(i != spell_mthPower) {
      Spell* const spell = eng.spellHandler->getSpellFromId(Spell_t(i));
      if(spell->isAvailForPlayer()) {
        spellCandidates.push_back(spell);
      } else {
        delete spell;
      }
    }
  }

  Spell* spellToCast = NULL;
  const int ELEMENT = eng.dice.range(0, spellCandidates.size() - 1);
  spellToCast = spellCandidates.at(ELEMENT);

  spellToCast->cast(eng.player, false, eng);
  trace << "SpellMthPower::castRandomOtherSpell() [DONE]" << endl;
}

//------------------------------------------------------------ BLESS
SpellCastRetData SpellBless::specificCast(
  Actor* const caster, Engine& eng) {

  caster->getPropHandler().tryApplyProp(
    new PropBlessed(eng, propTurnsStandard));

  return SpellCastRetData(true);
}

bool SpellBless::isGoodForMonsterToCastNow(
  Monster* const monster, Engine& eng) {
  (void)eng;
  return monster->getPropHandler().hasProp(propBlessed) == false;
}

//------------------------------------------------------------ TELEPORT
SpellCastRetData SpellTeleport::specificCast(
  Actor* const caster, Engine& eng) {

  if(caster != eng.player) {
    if(eng.player->checkIfSeeActor(*caster, NULL)) {
      eng.log->addMsg(
        caster->getNameThe() + " dissapears in a blast of smoke!");
    }
  }

  caster->teleport(false);
  return SpellCastRetData(true);
}

bool SpellTeleport::isGoodForMonsterToCastNow(
  Monster* const monster, Engine& eng) {
  bool blockers[MAP_W][MAP_H];
  MapParser::parse(CellPredBlocksVision(eng), blockers);
  return monster->checkIfSeeActor(*(eng.player), blockers) &&
         monster->getHp() <= (monster->getHpMax(true) / 2) &&
         eng.dice.coinToss();
}

//------------------------------------------------------------ KNOCKBACK
SpellCastRetData SpellKnockBack::specificCast(
  Actor* const caster, Engine& eng) {
  if(caster == eng.player) {

  } else {
    eng.log->addMsg("A force pushes me!", clrMsgBad);
    eng.knockBack->tryKnockBack(*(eng.player), caster->pos, false);
  }
  return SpellCastRetData(false);
}

bool SpellKnockBack::isGoodForMonsterToCastNow(
  Monster* const monster, Engine& eng) {
  bool blockers[MAP_W][MAP_H];
  MapParser::parse(CellPredBlocksVision(eng), blockers);
  return monster->checkIfSeeActor(*(eng.player), blockers);
}

//------------------------------------------------------------ ENFEEBLE
SpellCastRetData SpellEnfeeble::specificCast(
  Actor* const caster, Engine& eng) {

  const PropId_t propId = getPropId(eng);

  if(caster == eng.player) {
    vector<Actor*> SpottedEnemies;
    eng.player->getSpottedEnemies(SpottedEnemies);

    if(SpottedEnemies.empty()) {
      return SpellCastRetData(false);
    } else {
      vector<Pos> actorPositions;
      actorPositions.resize(0);

      for(unsigned int i = 0; i < SpottedEnemies.size(); i++) {
        actorPositions.push_back(SpottedEnemies.at(i)->pos);
      }

      eng.renderer->drawBlastAnimationAtPositionsWithPlayerVision(
        actorPositions, clrMagenta);

      for(unsigned int i = 0; i < SpottedEnemies.size(); i++) {
        PropHandler& propHandler = SpottedEnemies.at(i)->getPropHandler();
        Prop* const prop = propHandler.makePropFromId(
                             propId, propTurnsStandard);
        propHandler.tryApplyProp(prop);
      }
      return SpellCastRetData(true);
    }
  } else {
    eng.renderer->drawBlastAnimationAtPositionsWithPlayerVision(
      vector<Pos>(1, eng.player->pos), clrMagenta);

    PropHandler& propHandler = eng.player->getPropHandler();
    Prop* const prop = propHandler.makePropFromId(
                         propId, propTurnsStandard);
    propHandler.tryApplyProp(prop);

    return SpellCastRetData(false);
  }
}

bool SpellEnfeeble::isGoodForMonsterToCastNow(
  Monster* const monster, Engine& eng) {
  bool blockers[MAP_W][MAP_H];
  MapParser::parse(CellPredBlocksVision(eng), blockers);
  return monster->checkIfSeeActor(*(eng.player), blockers);
}

PropId_t SpellEnfeeble::getPropId(Engine& eng) const {
  const int RND = eng.dice.range(1, 5);
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
//SpellCastRetData SpellConfuse::specificCast(Actor* const caster, Engine& eng) {
//  Actor* actor = eng.basicUtils->getActorAtPos(d.targetCell_);
//
//  if(actor != NULL) {
//
//    if(actor == eng.player) {
//      eng.log->addMsg("My mind is reeling!");
//    }
//
//    actor->getPropHandler().tryApplyProp(new StatusConfused(eng));
//  }
//}
//
//void SpellConfuse::specificMonsterCast(
//  Monster* const monster, Engine& eng) {
//  specificCast(SpellCastData(monster, eng.player->pos), eng);
//}
//
//bool SpellConfuse::isGoodForMonsterToCastNow(
//  Monster* const monster, Engine& eng) {
//  bool blockers[MAP_W][MAP_H];
//  engine.mapTests->makeVisionBlockerArray(monster->pos, blockers);
//  if(monster->checkIfSeeActor(*(engine.player), blockers)) {
//    return engine.player->getPropHandler().allowSee();
//  }
//  return false;
//}

//------------------------------------------------------------ WEAKNESS
//SpellCastRetData SpellWeakness::specificCast(Actor* const caster, Engine& eng) {
//  Actor* actor = eng.basicUtils->getActorAtPos(d.targetCell_);
//
//  if(actor != NULL) {
//
//    if(actor == eng.player) {
//      eng.log->addMsg("Something is draining me physically!");
//    }
//
//    actor->getPropHandler().tryApplyProp(new StatusWeak(eng));
//  }
//}
//
//void SpellWeakness::specificMonsterCast(
//  Monster* const monster, Engine& eng) {
//  specificCast(SpellCastData(monster, eng.player->pos), eng);
//}
//
//bool SpellWeakness::isGoodForMonsterToCastNow(
//  Monster* const monster, Engine& eng) {
//  bool blockers[MAP_W][MAP_H];
//  engine.mapTests->makeVisionBlockerArray(monster->pos, blockers);
//  if(monster->checkIfSeeActor(*(engine.player), blockers)) {
//    return engine.player->getPropHandler().allowSee();
//  }
//  return false;
//}

//------------------------------------------------------------ BLIND
//SpellCastRetData SpellBlind::specificCast(Actor* const caster, Engine& eng) {
//  Actor* actor = eng.basicUtils->getActorAtPos(d.targetCell_);
//
//  if(actor != NULL) {
//
//    if(actor == eng.player) {
//      eng.log->addMsg("Scales starts to grow over my eyes!");
//    }
//
//    actor->getPropHandler().tryApplyProp(new PropBlind(eng.dice(3, 6)));
//  }
//}
//
//void SpellBlind::specificMonsterCast(
//  Monster* const monster, Engine& eng) {
//  specificCast(SpellCastData(monster, eng.player->pos), eng);
//}
//
//bool SpellBlind::isGoodForMonsterToCastNow(
//  Monster* const monster, Engine& eng) {
//  bool blockers[MAP_W][MAP_H];
//  engine.mapTests->makeVisionBlockerArray(monster->pos, blockers);
//  if(monster->checkIfSeeActor(*(engine.player), blockers)) {
//    return engine.player->getPropHandler().allowSee() ;
//  }
//  return false;
//}

//------------------------------------------------------------ FEAR
//SpellCastRetData SpellFear::specificCast(Actor* const caster, Engine& eng) {
//  Actor* actor = eng.basicUtils->getActorAtPos(d.targetCell_);
//
//  if(actor != NULL) {
//
//    if(actor == eng.player) {
//      eng.log->addMsg("My mind is besieged by terror.");
//    }
//
//    actor->getPropHandler().tryApplyProp(new PropTerrified(eng.dice(3, 6)));
//  }
//}
//
//void SpellFear::specificMonsterCast(
//  Monster* const monster, Engine& eng) {
//  specificCast(SpellCastData(monster, eng.player->pos), eng);
//}
//
//bool SpellFear::isGoodForMonsterToCastNow(
//  Monster* const monster, Engine& eng) {
//  bool blockers[MAP_W][MAP_H];
//  engine.mapTests->makeVisionBlockerArray(monster->pos, blockers);
//  return monster->checkIfSeeActor(*(engine.player), blockers);
//}

//------------------------------------------------------------ SLOW
//SpellCastRetData SpellSlow::specificCast(Actor* const caster, Engine& eng) {
//  Actor* actor = eng.basicUtils->getActorAtPos(d.targetCell_);
//
//  if(actor != NULL) {
//
//    if(actor == eng.player) {
//      eng.log->addMsg("I start to feel bogged down.");
//    }
//
//    actor->getPropHandler().tryApplyProp(new StatusSlowed(eng.dice(3, 6)));
//  }
//}
//
//void SpellSlow::specificMonsterCast(
//  Monster* const monster, Engine& eng) {
//  specificCast(SpellCastData(monster, eng.player->pos), eng);
//}
//
//bool SpellSlow::isGoodForMonsterToCastNow(
//  Monster* const monster, Engine& eng) {
//  bool blockers[MAP_W][MAP_H];
//  engine.mapTests->makeVisionBlockerArray(monster->pos, blockers);
//  return monster->checkIfSeeActor(*(engine.player), blockers);
//}

//------------------------------------------------------------ DISEASE
SpellCastRetData SpellDisease::specificCast(
  Actor* const caster, Engine& eng) {
  if(caster == eng.player) {
    return SpellCastRetData(true);
  } else {
    eng.log->addMsg(
      "A disease is starting to afflict my body!", clrMsgBad);
    eng.player->getPropHandler().tryApplyProp(
      new PropDiseased(eng, propTurnsStandard));
    return SpellCastRetData(false);
  }
}

bool SpellDisease::isGoodForMonsterToCastNow(
  Monster* const monster, Engine& eng) {
  bool blockers[MAP_W][MAP_H];
  MapParser::parse(CellPredBlocksVision(eng), blockers);
  return monster->checkIfSeeActor(*(eng.player), blockers);
}

//------------------------------------------------------------ SUMMON RANDOM
SpellCastRetData SpellSummonRandom::specificCast(
  Actor* const caster, Engine& eng) {

  Pos summonPos(caster->pos);

  vector<Pos> freePositionsSeenByPlayer;
  const int RADI = FOV_STD_RADI_INT;
  const Pos playerPos(eng.player->pos);
  const int X0 = max(0, playerPos.x - RADI);
  const int Y0 = max(0, playerPos.y - RADI);
  const int X1 = min(MAP_W, playerPos.x + RADI) - 1;
  const int Y1 = min(MAP_H, playerPos.y + RADI) - 1;
  for(int x = X0; x <= X1; x++) {
    for(int y = Y0; y <= Y1; y++) {
      if(eng.map->cells[x][y].isSeenByPlayer) {
        freePositionsSeenByPlayer.push_back(Pos(x, y));
      }
    }
  }

  bool blockers[MAP_W][MAP_H];
  MapParser::parse(CellPredBlocksBodyType(bodyType_normal, true, eng),
                   blockers);

  for(int i = 0; i < int(freePositionsSeenByPlayer.size()); i++) {
    const Pos pos(freePositionsSeenByPlayer.at(i));
    if(blockers[pos.x][pos.y]) {
      freePositionsSeenByPlayer.erase(freePositionsSeenByPlayer.begin() + i);
      i--;
    }
  }

  if(freePositionsSeenByPlayer.empty()) {
    vector<Pos> freeCellsVector;
    eng.basicUtils->makeVectorFromBoolMap(false, blockers, freeCellsVector);
    if(freeCellsVector.empty() == false) {
      sort(freeCellsVector.begin(), freeCellsVector.end(),
           IsCloserToOrigin(caster->pos, eng));
      summonPos = freeCellsVector.at(0);
    }
  } else {
    const int ELEMENT =
      eng.dice.range(0, freePositionsSeenByPlayer.size() - 1);
    summonPos = freePositionsSeenByPlayer.at(ELEMENT);
  }

  vector<ActorId_t> summonCandidates;
  for(int i = 1; i < endOfActorIds; i++) {
    const ActorData& data = eng.actorDataHandler->dataList[i];
    if(data.canBeSummoned) {
      if(data.spawnMinDLVL <= caster->getData().spawnMinDLVL) {
        summonCandidates.push_back(ActorId_t(i));
      }
    }
  }
  const int ELEMENT = eng.dice.range(1, summonCandidates.size() - 1);
  const ActorId_t id = summonCandidates.at(ELEMENT);
  Actor* const actor = eng.actorFactory->spawnActor(id, summonPos);
  Monster* monster = dynamic_cast<Monster*>(actor);
  monster->playerAwarenessCounter = monster->getData().nrTurnsAwarePlayer;
  if(eng.map->cells[summonPos.x][summonPos.y].isSeenByPlayer) {
    eng.log->addMsg(monster->getNameA() + " appears.");
  }
  return SpellCastRetData(false);
}

bool SpellSummonRandom::isGoodForMonsterToCastNow(
  Monster* const monster, Engine& eng) {

  bool blockers[MAP_W][MAP_H];
  MapParser::parse(CellPredBlocksVision(eng), blockers);
  return monster->checkIfSeeActor(*(eng.player), blockers) ||
         (eng.dice.oneIn(20));
}

//------------------------------------------------------------ HEAL SELF
SpellCastRetData SpellHealSelf::specificCast(
  Actor* const caster, Engine& eng) {
  (void)eng;
  return SpellCastRetData(caster->restoreHp(999, true));
}

bool SpellHealSelf::isGoodForMonsterToCastNow(
  Monster* const monster, Engine& eng) {
  (void)eng;
  return monster->getHp() < monster->getHpMax(true);
}

//------------------------------------------------------------ MI-GO HYPNOSIS
SpellCastRetData SpellMiGoHypnosis::specificCast(
  Actor* const caster, Engine& eng) {

  (void)caster;
  eng.log->addMsg("There is a sharp droning in my head!");

  if(eng.dice.coinToss()) {
    eng.player->getPropHandler().tryApplyProp(
      new PropFainted(eng, propTurnsSpecified, eng.dice.range(2, 10)));
  } else {
    eng.log->addMsg("I feel dizzy.");
  }

  return true;
}

bool SpellMiGoHypnosis::isGoodForMonsterToCastNow(
  Monster* const monster, Engine& eng) {

  bool blockers[MAP_W][MAP_H];
  MapParser::parse(CellPredBlocksVision(eng), blockers);
  return monster->checkIfSeeActor(*(eng.player), blockers) &&
         eng.dice.oneIn(4);
}
