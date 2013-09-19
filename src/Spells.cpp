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
#include "DungeonClimb.h"
#include "FeatureTrap.h"
#include "Inventory.h"
#include "PlayerSpellsHandler.h"
#include "ItemScroll.h"
#include "ItemArmor.h"

Spell* SpellHandler::getRandomSpellForMonster() {
  vector<Spells_t> candidates;
  for(int i = 0; i < endOfSpells; i++) {
    Spell* const spell = getSpellFromId(Spells_t(i));
    if(spell->isLearnableForMonsters()) {
      candidates.push_back(Spells_t(i));
    }
    delete spell;
  }
  const int ELEMENT = eng->dice.range(0, candidates.size() - 1);
  return getSpellFromId(candidates.at(ELEMENT));
}

Spell* SpellHandler::getSpellFromId(const Spells_t spellId) const {
  switch(spellId) {
    case spell_enfeeble:        return new SpellEnfeeble;       break;
    case spell_disease:         return new SpellDisease;        break;
    case spell_azathothsBlast:  return new SpellAzathothsBlast; break;
    case spell_summonRandom:    return new SpellSummonRandom;   break;
    case spell_healSelf:        return new SpellHealSelf;       break;
    case spell_knockBack:       return new SpellKnockBack;      break;
    case spell_teleport:        return new SpellTeleport;       break;
    case spell_mayhem:          return new SpellMayhem;         break;
    case spell_pestilence:      return new SpellPestilence;     break;
    case spell_descent:         return new SpellDescent;        break;
    case spell_detectItems:     return new SpellDetectItems;    break;
    case spell_detectTraps:     return new SpellDetectTraps;    break;
    case spell_identify:        return new SpellIdentify;       break;
    case spell_clairvoyance:    return new SpellClairvoyance;   break;
    case spell_opening:         return new SpellOpening;        break;
    case spell_mthPower:        return new SpellMthPower;       break;
    case spell_bless:           return new SpellBless;          break;

    case endOfSpells: {} break;
  }
  trace << "[WARNING] Found no spell for ID: " << spellId;
  trace << ", in SpellHandler::getSpellFromId()" << endl;
  return NULL;
}

int Spell::getMaxSpiCost(const bool IS_BASE_COST_ONLY, Actor* const caster,
                         Engine* const eng) const {
  int cost = getSpecificSpiCost();

  if(IS_BASE_COST_ONLY == false) {
    for(int dy = -1; dy <= 1; dy++) {
      for(int dx = -1; dx <= 1; dx++) {
        const Pos pos(caster->pos + Pos(dx, dy));
        if(eng->map->featuresStatic[pos.x][pos.y]->getId() == feature_altar) {
          cost -= 1;
          dy = 999;
          dx = 999;
        }
      }
    }

    PropHandler* propHandeler = caster->getPropHandler();

    if(propHandeler->hasProp(propBlessed))  {cost -= 1;}
    if(propHandeler->allowSee() == false)   {cost -= 1;}
    if(propHandeler->hasProp(propCursed))   {cost += 3;}

    if(caster == eng->player) {
      cost -= eng->player->getMth() / CAST_FROM_MEMORY_MTH_BON_DIV;
    }
  }

  return max(1, cost);
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

    SpellCastRetData ret = specificCast(caster, eng);

    if(IS_INTRINSIC) {
      caster->hitSpi(eng->dice(1, getMaxSpiCost(false, caster, eng)));
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

    if(spotedEnemies.empty() == false) {
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
        spotedEnemies.at(i)->hit(eng->dice(1, 8), dmgType_physical);
        eng->soundEmitter->emitSound(
          Sound("I hear a roaring blast", true, spotedEnemies.at(i)->pos,
                true, true));
      }
      return SpellCastRetData(true);
    } else {
      return SpellCastRetData(false);
    }
  } else {
    eng->log->addMsg("I am struck by a roaring blast!", clrMessageBad);
    eng->renderer->drawBlastAnimationAtPositionsWithPlayerVision(
      vector<Pos>(1, eng->player->pos), clrRedLgt);
    eng->player->getPropHandler()->tryApplyProp(
      new PropParalyzed(eng, propTurnsSpecified, 1));
    eng->player->hit(eng->dice(1, 8), dmgType_physical);
    eng->soundEmitter->emitSound(
      Sound("", true, eng->player->pos, true, true));
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
            if(f->isMoveTypePassable(moveType_walk)) {
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

  eng->soundEmitter->emitSound(
    Sound("", true, eng->player->pos, true, true));

  return SpellCastRetData(true);
}

//------------------------------------------------------------ PESTILENCE
SpellCastRetData SpellPestilence::specificCast(
  Actor* const caster, Engine* const eng) {
  (void)caster;
  bool blockers[MAP_X_CELLS][MAP_Y_CELLS];
  eng->mapTests->makeMoveBlockerArrayForMoveType(moveType_walk, blockers);

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

//------------------------------------------------------------ DESCENT
SpellCastRetData SpellDescent::specificCast(
  Actor* const caster, Engine* const eng) {
  (void)caster;
  if(eng->map->getDLVL() < FIRST_CAVERN_LEVEL - 1) {
    eng->dungeonClimb->travelDown(1);
    eng->log->addMsg("I sink downwards!");
  } else {
    eng->log->addMsg("I feel a faint sinking sensation.");
  }
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

//------------------------------------------------------------ IDENTIFY
SpellCastRetData SpellIdentify::specificCast(
  Actor * const caster, Engine * const eng) {
  (void)caster;

  Inventory* const inv = eng->player->getInventory();

  vector<Item*> itemIdentifyCandidates;

  vector<InventorySlot>* slots = inv->getSlots();
  for(unsigned int i = 0; i < slots->size(); i++) {
    Item* const item = slots->at(i).item;
    if(item != NULL) {
      const ItemData& d = item->getData();
      if(d.isIdentified == false) {
        itemIdentifyCandidates.push_back(item);
      }
    }
  }
  vector<Item*>* backpack = inv->getGeneral();
  for(unsigned int i = 0; i < backpack->size(); i++) {
    Item* const item = backpack->at(i);
    if(item->getData().id != item_scrollOfIdentify) {
      const ItemData& d = item->getData();
      if(d.isIdentified == false) {
        itemIdentifyCandidates.push_back(item);
      }
    }
  }

  const unsigned int NR_ELEMENTS = itemIdentifyCandidates.size();
  if(NR_ELEMENTS == 0) {
    return SpellCastRetData(false);
  } else {
    Item* const item =
      itemIdentifyCandidates.at(
        eng->dice.range(0, NR_ELEMENTS - 1));

    const string itemNameBefore =
      eng->itemDataHandler->getItemRef(*item, itemRef_a, true);

    item->identify(true);

    const string itemNameAfter =
      eng->itemDataHandler->getItemRef(*item, itemRef_a, true);

    eng->log->addMsg("I gain intuitions about " + itemNameBefore + "...");
    eng->log->addMsg("It is identified as " + itemNameAfter + "!");

    return SpellCastRetData(true);
  }
}

//------------------------------------------------------------ CLAIRVOYANCE
SpellCastRetData SpellClairvoyance::specificCast(
  Actor * const caster, Engine * const eng) {
  (void)caster;
  eng->player->getPropHandler()->tryApplyProp(
    new PropClairvoyant(eng, propTurnsStandard), true, false);
  return SpellCastRetData(true);
}

//------------------------------------------------------------ OPENING
SpellCastRetData SpellOpening::specificCast(
  Actor * const caster, Engine * const eng) {

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
    if(featuresOpenedPositions.size() == 1) {
      eng->log->addMsg("An object was opened.");
    }
    if(featuresOpenedPositions.size() > 1) {
      eng->log->addMsg("Some objects were opened.");
    }
    return SpellCastRetData(true);
  } else {
    return SpellCastRetData(false);
  }
}

//------------------------------------------------------------ MTH POWER
SpellCastRetData SpellMthPower::specificCast(
  Actor * const caster, Engine * const eng) {
  (void)caster;

  vector<MthPowerAction_t> possibleActions;
  getPossibleActions(possibleActions, eng);

  if(possibleActions.empty()) {
    eng->log->addMsg("I fail to channel the spell for any purpose.");
  } else {
    const int ELEMENT = eng->dice.range(0, possibleActions.size() - 1);
    doAction(possibleActions.at(ELEMENT), eng);
  }
  return SpellCastRetData(true);
}

void SpellMthPower::getPossibleActions(
  vector<MthPowerAction_t>& possibleActions, Engine * const eng) const {

  possibleActions.resize(0);

  vector<Actor*> spotedEnemies;
  eng->player->getSpotedEnemies(spotedEnemies);

  const int MTH = eng->player->getMth();

  if(MTH >= 35) {
    if(spotedEnemies.empty() == false) {
      possibleActions.push_back(mthPowerAction_slayMonsters);
    }
  }

  for(int y = 1; y < MAP_Y_CELLS; y++) {
    for(int x = 1; x < MAP_X_CELLS; x++) {
      if(eng->map->featuresStatic[x][y]->getId() == feature_stairsDown) {
        if(eng->map->explored[x][y] == false) {
          possibleActions.push_back(mthPowerAction_findStairs);
        }
      }
    }
  }

//  bool canAnySpellBeRestored = false;
//  const unsigned int NR_OF_SPELLS =
//    eng->playerPowersHandler->getNrOfSpells();
//  for(unsigned int i = 0; i < NR_OF_SPELLS; i++) {
//    Scroll* const scroll =  eng->playerPowersHandler->getScrollAt(i);
//    const ItemDef& d = scroll->getData();
//    if(
//      d.isScrollLearnable &&
//      d.isScrollLearned &&
//      d.castFromMemoryCurrentBaseChance < CAST_FROM_MEMORY_CHANCE_LIM &&
//      d.id != item_thaumaturgicAlteration) {
//      canAnySpellBeRestored = true;
//    }
//  }
//  if(canAnySpellBeRestored) {
//    possibleActions.push_back(mthPowerAction_sorcery);
//  }

  if(eng->player->getHp() < eng->player->getHpMax(true)) {
    possibleActions.push_back(mthPowerAction_heal);
  }

  Item* item = eng->player->getInventory()->getItemInSlot(slot_wielded);
  if(item != NULL) {
    const ItemData& d = item->getData();
    if(d.isMeleeWeapon && d.isRangedWeapon == false) {
      Weapon* const weapon = dynamic_cast<Weapon*>(item);
      if(weapon->meleeDmgPlus < 3) {
        possibleActions.push_back(mthPowerAction_improveWeapon);
      }
    }
  }

  item = eng->player->getInventory()->getItemInSlot(slot_armorBody);
  if(item != NULL) {
    Armor* const armor = dynamic_cast<Armor*>(item);
    if(armor->getDurability() < 100) {
      possibleActions.push_back(mthPowerAction_mendArmor);
    }
  }

//  if(eng->player->getPropHandler()->hasAnyBadEffect()) {
//    possibleActions.push_back(mthPowerAction_purgeEffects);
//  }
}

void SpellMthPower::doAction(const MthPowerAction_t action,
                             Engine * const eng) const {
  switch(action) {
    case mthPowerAction_slayMonsters: {
      vector<Actor*> spotedEnemies;
      eng->player->getSpotedEnemies(spotedEnemies);

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
        spotedEnemies.at(i)->hit(25, dmgType_physical);
      }

      eng->renderer->drawMapAndInterface(true);
    } break;

    case mthPowerAction_heal: {
      bool visionBlockers[MAP_X_CELLS][MAP_Y_CELLS];
      eng->mapTests->makeVisionBlockerArray(
        eng->player->pos, visionBlockers);
      eng->player->getPropHandler()->endAppliedProp(
        propDiseased, visionBlockers);
      eng->player->restoreHp(999, true);
    } break;

    case mthPowerAction_findStairs: {
      trace << "ThaumaturgicAlteration: Find stairs" << endl;
      for(int y = 1; y < MAP_Y_CELLS - 1; y++) {
        for(int x = 1; x < MAP_X_CELLS - 1; x++) {
          const FeatureStatic* const f = eng->map->featuresStatic[x][y];
          if(f->getId() == feature_stairsDown) {
            for(int dy = -1; dy <= 1; dy++) {
              for(int dx = -1; dx <= 1; dx++) {
                eng->map->playerVision[x + dx][y + dy] = true;
                eng->map->explored[x + dx][y + dy] = true;
              }
            }
          }
        }
      }
      eng->log->addMsg("The way forward is revealed!");
      eng->renderer->drawMapAndInterface(true);
      eng->player->updateFov();
      eng->renderer->drawMapAndInterface(true);
    } break;

//    case mthPowerAction_sorcery: {
//      eng->log->addMsg("My magic is restored!");
//      const unsigned int NR_OF_SCROLLS =
//        eng->playerPowersHandler->getNrOfSpells();
//      for(unsigned int i = 0; i < NR_OF_SCROLLS; i++) {
//        Scroll* const scroll = eng->playerPowersHandler->getScrollAt(i);
//        const ItemDef& d = scroll->getData();
//        if(
//          d.isScrollLearnable &&
//          d.isScrollLearned   &&
//          d.id != item_thaumaturgicAlteration) {
//          scroll->setCastFromMemoryCurrentBaseChance(
//            CAST_FROM_MEMORY_CHANCE_LIM);
//        }
//      }
//    } break;

    case mthPowerAction_mendArmor: {
      eng->log->addMsg("My armor is whole!");
      Item* const item =
        eng->player->getInventory()->getItemInSlot(slot_armorBody);
      Armor* const armor = dynamic_cast<Armor*>(item);
      armor->setMaxDurability();
      eng->renderer->drawMapAndInterface(true);
    } break;

    case mthPowerAction_improveWeapon: {
      eng->log->addMsg("My weapon is deadlier!");
      Item* const item =
        eng->player->getInventory()->getItemInSlot(slot_wielded);
      Weapon* const weapon = dynamic_cast<Weapon*>(item);
      weapon->meleeDmgPlus++;
    } break;

//    case mthPowerAction_purgeEffects: {
//      bool visionBlockers[MAP_X_CELLS][MAP_Y_CELLS];
//      eng->mapTests->makeVisionBlockerArray(eng->player->pos,
//                                            visionBlockers, 9999);
//
//      StatusHandler* const statusHandler =
//        eng->player->getPropHandler();
//      statusHandler->endEffectsOfAbility(
//        ability_resistStatusMind, visionBlockers);
//    } break;
  }
}

//------------------------------------------------------------ BLESS
SpellCastRetData SpellBless::specificCast(
  Actor * const caster, Engine * const eng) {

  caster->getPropHandler()->tryApplyProp(
    new PropBlessed(eng, propTurnsStandard));

  return SpellCastRetData(true);
}

bool SpellBless::isGoodForMonsterToCastNow(
  Monster * const monster, Engine * const eng) {
  (void)eng;
  return monster->getPropHandler()->hasProp(propBlessed) == false;
}

//------------------------------------------------------------ TELEPORT
SpellCastRetData SpellTeleport::specificCast(
  Actor * const caster, Engine * const eng) {

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
  Monster * const monster, Engine * const eng) {
  bool blockers[MAP_X_CELLS][MAP_Y_CELLS];
  eng->mapTests->makeVisionBlockerArray(monster->pos, blockers);
  return monster->checkIfSeeActor(*(eng->player), blockers)  &&
         monster->getHp() <= (monster->getHpMax(true) / 2)      &&
         eng->dice.coinToss();
}

//------------------------------------------------------------ KNOCKBACK
SpellCastRetData SpellKnockBack::specificCast(
  Actor * const caster, Engine * const eng) {
  if(caster == eng->player) {

  } else {
    eng->log->addMsg("A force pushes me!", clrMessageBad);
    eng->knockBack->tryKnockBack(eng->player, caster->pos, false);
  }
  return SpellCastRetData(false);
}

bool SpellKnockBack::isGoodForMonsterToCastNow(
  Monster * const monster, Engine * const eng) {
  bool blockers[MAP_X_CELLS][MAP_Y_CELLS];
  eng->mapTests->makeVisionBlockerArray(monster->pos, blockers);
  return monster->checkIfSeeActor(*(eng->player), blockers);
}

//------------------------------------------------------------ ENFEEBLE
SpellCastRetData SpellEnfeeble::specificCast(
  Actor * const caster, Engine * const eng) {
  Prop* const prop = getProp(eng);

  if(caster == eng->player) {
//    eng->player->
  } else {

  }
}

bool SpellEnfeeble::isGoodForMonsterToCastNow(
  Monster * const monster, Engine * const eng) {
  bool blockers[MAP_X_CELLS][MAP_Y_CELLS];
  eng->mapTests->makeVisionBlockerArray(monster->pos, blockers);
  return monster->checkIfSeeActor(*(eng->player), blockers);
}

Prop* SpellEnfeeble::getProp(Engine * const eng) const {
  const int RND = eng->dice.range(1, 5);
  switch(RND) {
    case 1: {return new PropConfused(eng, propTurnsStandard);}
    case 2: {return new PropParalyzed(eng, propTurnsStandard);}
    case 3: {return new PropSlowed(eng, propTurnsStandard);}
    case 4: {return new PropBlind(eng, propTurnsStandard);}
    case 5: {return new PropTerrified(eng, propTurnsStandard);}
  }
  return NULL;
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
  Actor * const caster, Engine * const eng) {
  if(caster == eng->player) {

  } else {
    eng->log->addMsg(
      "A disease is starting to afflict my body!", clrMessageBad);
    eng->player->getPropHandler()->tryApplyProp(
      new PropDiseased(eng, propTurnsStandard));
  }
}

bool SpellDisease::isGoodForMonsterToCastNow(
  Monster * const monster, Engine * const eng) {
  bool blockers[MAP_X_CELLS][MAP_Y_CELLS];
  eng->mapTests->makeVisionBlockerArray(monster->pos, blockers);
  return monster->checkIfSeeActor(*(eng->player), blockers);
}

//------------------------------------------------------------ SUMMON RANDOM
SpellCastRetData SpellSummonRandom::specificCast(
  Actor * const caster, Engine * const eng) {

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
  eng->mapTests->makeMoveBlockerArrayForMoveType(moveType_walk, blockers);

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
  Monster * const monster, Engine * const eng) {

  bool blockers[MAP_X_CELLS][MAP_Y_CELLS];
  eng->mapTests->makeVisionBlockerArray(monster->pos, blockers);
  const bool IS_PLAYER_SEEN =
    monster->checkIfSeeActor(*(eng->player), blockers);
  return IS_PLAYER_SEEN || (eng->dice.percentile() < 5);
}

//------------------------------------------------------------ HEAL SELF
SpellCastRetData SpellHealSelf::specificCast(
  Actor * const caster, Engine * const eng) {
  return SpellCastRetData(caster->restoreHp(999, true));
}

bool SpellHealSelf::isGoodForMonsterToCastNow(
  Monster * const monster, Engine * const engine) {
  (void)engine;
  return monster->getHp() < monster->getHpMax(true);
}
