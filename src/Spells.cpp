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
#include "PlayerBonuses.h"

Spell* SpellHandler::getRandomSpellForMonster() {
  vector<SpellId> candidates;
  for(int i = 0; i < endOfSpellId; i++) {
    Spell* const spell = getSpellFromId(SpellId(i));
    if(spell->isAvailForAllMonsters()) {
      candidates.push_back(SpellId(i));
    }
    delete spell;
  }
  const int ELEMENT = eng.dice.range(0, candidates.size() - 1);
  return getSpellFromId(candidates.at(ELEMENT));
}

Spell* SpellHandler::getSpellFromId(const SpellId spellId) const {
  switch(spellId) {
    case spell_enfeeble:            return new SpellEnfeeble;
    case spell_disease:             return new SpellDisease;
    case spell_darkbolt:            return new SpellDarkbolt;
    case spell_azathothsWrath:      return new SpellAzathothsWrath;
    case spell_summonRandom:        return new SpellSummonRandom;
    case spell_healSelf:            return new SpellHealSelf;
    case spell_knockBack:           return new SpellKnockBack;
    case spell_teleport:            return new SpellTeleport;
    case spell_mayhem:              return new SpellMayhem;
    case spell_pestilence:          return new SpellPestilence;
    case spell_detectItems:         return new SpellDetectItems;
    case spell_detectTraps:         return new SpellDetectTraps;
    case spell_detectMonsters:      return new SpellDetectMonsters;
    case spell_opening:             return new SpellOpening;
    case spell_sacrificeLife:       return new SpellSacrificeLife;
    case spell_sacrificeSpirit:     return new SpellSacrificeSpirit;
    case spell_cloudMinds:          return new SpellCloudMinds;
    case spell_bless:               return new SpellBless;
    case spell_miGoHypnosis:        return new SpellMiGoHypnosis;

    case endOfSpellId: {} break;
  }
  trace << "[WARNING] Found no spell for ID: " << spellId;
  trace << ", in SpellHandler::getSpellFromId()" << endl;
  return NULL;
}

Range Spell::getSpiCost(const bool IS_BASE_COST_ONLY, Actor* const caster,
                        Engine& eng) const {
  int costMax = getMaxSpiCost_();

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

    if(caster == eng.player) {
      bool isWarlock      = false;
      bool isBloodSorc    = false;
      bool isSeer         = false;

      for(TraitId id : eng.playerBonHandler->traitsPicked_) {
        switch(id) {
          case traitWarlock:        isWarlock     = true; break;
          case traitBloodSorcerer:  isBloodSorc   = true; break;
          case traitSeer:           isSeer        = true; break;
          default: {} break;
        }
      }

      if(isBloodSorc) costMax--;

      switch(getId()) {
        case spell_darkbolt:       {if(isWarlock)  costMax--;}    break;
        case spell_azathothsWrath: {if(isWarlock)  costMax--;}    break;
        case spell_mayhem:         {if(isWarlock)  costMax--;}    break;
        case spell_detectMonsters: {if(isSeer)     costMax--;}    break;
        case spell_detectItems:    {if(isSeer)     costMax -= 3;} break;
        case spell_detectTraps:    {if(isSeer)     costMax -= 3;} break;
        default: {} break;
      }
    }

    PropHandler& propHlr = caster->getPropHandler();

    vector<PropId> props;
    propHlr.getAllActivePropIds(props);

    if(propHlr.allowSee() == false) costMax--;

    if(find(props.begin(), props.end(), propBlessed) != props.end()) {
      costMax -= 1;
    }

    if(find(props.begin(), props.end(), propCursed) != props.end()) {
      costMax += 3;
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
      const ShockSrc shockSrc = IS_INTRINSIC ?
                                shockSrc_castIntrSpell :
                                shockSrc_useStrangeItem;
      const int SHOCK_VALUE = IS_INTRINSIC ? getShockValueIntrCast() : 10;
      eng.player->incrShock(SHOCK_VALUE, shockSrc);
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
      ret = cast_(caster, eng);
    }

    eng.gameTime->actorDidAct();
    trace << "Spell::cast() [DONE]" << endl;
    return ret;
  }
  trace << "Spell::cast() [DONE]" << endl;
  return SpellCastRetData(false);
}

//------------------------------------------------------------ DARKBOLT
SpellCastRetData SpellDarkbolt::cast_(
  Actor* const caster, Engine& eng) {

  Actor* target = NULL;

  vector<Actor*> spottedActors;
  caster->getSpottedEnemies(spottedActors);
  if(spottedActors.empty()) {
    return SpellCastRetData(false);
  } else {
    target = eng.basicUtils->getRandomClosestActor(caster->pos, spottedActors);
  }

  vector<Pos> line;
  eng.lineCalc->calcNewLine(caster->pos, target->pos, true, 999, false, line);
  eng.renderer->drawMapAndInterface();
  const int LINE_SIZE = line.size();
  for(int i = 1; i < LINE_SIZE; i++) {
    const Pos& pos = line.at(i);
    if(eng.config->isTilesMode) {
      eng.renderer->drawTile(tile_blast1, panel_map, pos, clrMagenta);
    } else {
      eng.renderer->drawGlyph('*', panel_map, pos, clrMagenta);
    }
    eng.renderer->updateScreen();
    eng.sdlWrapper->sleep(eng.config->delayProjectileDraw);
  }

  eng.renderer->drawBlastAnimAtPositions(
    vector<Pos> {target->pos}, clrMagenta);

  const string msgCmn = " struck by a blast!";
  bool isCharged = false;
  if(caster == eng.player) {
    eng.log->addMsg(target->getNameThe() + " is" + msgCmn, clrMsgGood);

    vector<PropId> props;
    eng.player->getPropHandler().getAllActivePropIds(props);
    isCharged =
      find(props.begin(), props.end(), propWarlockCharged) != props.end();

  } else {
    eng.log->addMsg("I am" + msgCmn, clrMsgBad);
  }

  target->getPropHandler().tryApplyProp(
    new PropParalyzed(eng, propTurnsSpecific, 2));

  Range dmgRange(3, 10);
  const int DMG = isCharged ? dmgRange.upper : eng.dice.range(dmgRange);

  target->hit(DMG, dmgType_physical, true);

  Snd snd("", endOfSfxId, IgnoreMsgIfOriginSeen::yes, target->pos, NULL,
          SndVol::low, AlertsMonsters::yes);
  eng.sndEmitter->emitSnd(snd);

  return SpellCastRetData(true);
}

bool SpellDarkbolt::isGoodForMonsterToCastNow(
  Monster* const monster, Engine& eng) {
  bool blockers[MAP_W][MAP_H];
  MapParse::parse(CellPred::BlocksVision(eng), blockers);
  return monster->isSeeingActor(*(eng.player), blockers);
}

//------------------------------------------------------------ AZATHOTHS WRATH
SpellCastRetData SpellAzathothsWrath::cast_(Actor* const caster, Engine& eng) {

  Range dmgRange(1, 8);

  const string msgEnd = "struck by a roaring blast!";

  if(caster == eng.player) {
    vector<Actor*> targets;
    eng.player->getSpottedEnemies(targets);

    if(targets.empty()) {
      return SpellCastRetData(false);
    } else {
      vector<PropId> props;
      eng.player->getPropHandler().getAllActivePropIds(props);
      const bool IS_CHARGED =
        find(props.begin(), props.end(), propWarlockCharged) != props.end();

      vector<Pos> actorPositions; actorPositions.resize(0);
      for(Actor * a : targets) {actorPositions.push_back(a->pos);}

      eng.renderer->drawBlastAnimAtPositionsWithPlayerVision(
        actorPositions, clrRedLgt);

      for(Actor * actor : targets) {
        eng.log->addMsg(actor->getNameThe() + " is " + msgEnd, clrMsgGood);
        actor->getPropHandler().tryApplyProp(
          new PropParalyzed(eng, propTurnsSpecific, 2));

        const int DMG = IS_CHARGED ? dmgRange.upper : eng.dice.range(dmgRange);

        actor->hit(DMG, dmgType_physical, false);

        Snd snd("", endOfSfxId, IgnoreMsgIfOriginSeen::yes, actor->pos, NULL,
                SndVol::high, AlertsMonsters::yes);
        eng.sndEmitter->emitSnd(snd);
      }
      return SpellCastRetData(true);
    }
  } else {
    eng.log->addMsg("I am " + msgEnd, clrMsgBad);
    eng.renderer->drawBlastAnimAtPositionsWithPlayerVision(
      vector<Pos> {eng.player->pos}, clrRedLgt);
    eng.player->getPropHandler().tryApplyProp(
      new PropParalyzed(eng, propTurnsSpecific, 1));
    eng.player->hit(eng.dice.range(dmgRange), dmgType_physical, false);
    Snd snd("", endOfSfxId, IgnoreMsgIfOriginSeen::yes, eng.player->pos, NULL,
            SndVol::high, AlertsMonsters::yes);
    eng.sndEmitter->emitSnd(snd);
  }
  return SpellCastRetData(false);
}

bool SpellAzathothsWrath::isGoodForMonsterToCastNow(
  Monster* const monster, Engine& eng) {
  bool blockers[MAP_W][MAP_H];
  MapParse::parse(CellPred::BlocksVision(eng), blockers);
  return monster->isSeeingActor(*(eng.player), blockers);
}

//------------------------------------------------------------ MAYHEM
SpellCastRetData SpellMayhem::cast_(
  Actor* const caster, Engine& eng) {
  (void)caster;

  eng.log->addMsg("Destruction rages around me!");

  const Pos& playerPos = eng.player->pos;

  const int NR_OF_SWEEPS  = 5;
  const int AREA_RADI     = FOV_STD_RADI_INT;

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
            if(f->canMoveCmn()) {
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

  for(Actor * actor : eng.gameTime->actors_) {
    if(actor != eng.player) {
      if(eng.player->isSeeingActor(*actor, NULL)) {
        actor->getPropHandler().tryApplyProp(
          new PropBurning(eng, propTurnsStd));
      }
    }
  }

  Snd snd("", endOfSfxId, IgnoreMsgIfOriginSeen::yes, eng.player->pos, NULL,
          SndVol::high, AlertsMonsters::yes);
  eng.sndEmitter->emitSnd(snd);

  return SpellCastRetData(true);
}

//------------------------------------------------------------ PESTILENCE
SpellCastRetData SpellPestilence::cast_(
  Actor* const caster, Engine& eng) {
  (void)caster;
  bool blockers[MAP_W][MAP_H];
  MapParse::parse(CellPred::BlocksMoveCmn(true, eng), blockers);

  const int RADI = 4;
  const int x0 = max(0, eng.player->pos.x - RADI);
  const int y0 = max(0, eng.player->pos.y - RADI);
  const int x1 = min(MAP_W - 1, eng.player->pos.x + RADI);
  const int y1 = min(MAP_H - 1, eng.player->pos.y + RADI);

  ActorId monsterId = endOfActorIds;
  Dice& dice = eng.dice;

  vector<Pos> positions;

  for(int x = x0; x <= x1; x++) {
    for(int y = y0; y <= y1; y++) {
      if(blockers[x][y] == false) {
        positions.push_back(Pos(x, y));
      }
    }
  }

  eng.renderer->drawBlastAnimAtPositionsWithPlayerVision(
    positions, clrMagenta);

  for(Pos & pos : positions) {
    monsterId = actor_rat;
    if(dice(1, 3) == 1) {
      if(dice.coinToss()) {
        monsterId = actor_greenSpider;
      } else {
        monsterId = dice.coinToss() ? actor_whiteSpider : actor_redSpider;
      }
    }
    eng.actorFactory->spawnActor(monsterId, pos);
  }

  eng.log->addMsg("Disgusting critters appear around me!");
  return SpellCastRetData(true);
}

//------------------------------------------------------------ DETECT ITEMS
SpellCastRetData SpellDetectItems::cast_(
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
    eng.renderer->drawBlastAnimAtPositions(
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
SpellCastRetData SpellDetectTraps::cast_(
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
    eng.renderer->drawBlastAnimAtPositions(
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

//------------------------------------------------------------ DETECT MONSTERS
SpellCastRetData SpellDetectMonsters::cast_(Actor* const caster, Engine& eng) {
  (void)caster;

  bool isSeer           = eng.playerBonHandler->hasTrait(traitSeer);
  const int MULTIPLIER  = 6 * (isSeer ? 3 : 1);

  const int MAX_DIST    = FOV_STD_RADI_INT * 2;

  const Pos playerPos   = eng.player->pos;

  bool didDetect        = false;

  for(Actor * actor : eng.gameTime->actors_) {
    if(actor != eng.player) {
      if(eng.basicUtils->chebyshevDist(playerPos, actor->pos) <= MAX_DIST) {
        dynamic_cast<Monster*>(actor)->playerBecomeAwareOfMe(MULTIPLIER);
        didDetect = true;
      }
    }
  }

  if(didDetect) {eng.log->addMsg("I detect monsters.");}

  return SpellCastRetData(didDetect);
}

//------------------------------------------------------------ OPENING
SpellCastRetData SpellOpening::cast_(
  Actor* const caster, Engine& eng) {

  (void)caster;

  vector<Pos> featuresOpenedPositions;

  for(int y = 1; y < MAP_H - 1; y++) {
    for(int x = 1; x < MAP_W - 1; x++) {
      if(eng.map->cells[x][y].isSeenByPlayer) {
        if(eng.map->cells[x][y].featureStatic->open()) {
          featuresOpenedPositions.push_back(Pos(x, y));
        }
      }
    }
  }

  if(featuresOpenedPositions.empty() == false) {
    eng.renderer->drawMapAndInterface();
    eng.player->updateFov();
    eng.renderer->drawBlastAnimAtPositions(
      featuresOpenedPositions, clrWhite);
    eng.renderer->drawMapAndInterface();
    return SpellCastRetData(true);
  } else {
    return SpellCastRetData(false);
  }
}

//------------------------------------------------------------ SACRIFICE LIFE
SpellCastRetData SpellSacrificeLife::cast_(
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
SpellCastRetData SpellSacrificeSpirit::cast_(
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
SpellCastRetData SpellCloudMinds::cast_(
  Actor* const caster, Engine& eng) {

  (void)caster;
  eng.log->addMsg("I vanish from the minds of my enemies.");

  for(Actor * actor : eng.gameTime->actors_) {
    if(actor != eng.player) {
      Monster* const monster = dynamic_cast<Monster*>(actor);
      monster->awareOfPlayerCounter_ = 0;
    }
  }
  return SpellCastRetData(true);
}

//------------------------------------------------------------ BLESS
SpellCastRetData SpellBless::cast_(
  Actor* const caster, Engine& eng) {

  caster->getPropHandler().tryApplyProp(
    new PropBlessed(eng, propTurnsStd));

  return SpellCastRetData(true);
}

bool SpellBless::isGoodForMonsterToCastNow(
  Monster* const monster, Engine& eng) {
  (void)eng;

  vector<PropId> props;
  monster->getPropHandler().getAllActivePropIds(props);
  return find(props.begin(), props.end(), propBlessed) == props.end();
}

//------------------------------------------------------------ TELEPORT
SpellCastRetData SpellTeleport::cast_(
  Actor* const caster, Engine& eng) {

  if(caster != eng.player) {
    if(eng.player->isSeeingActor(*caster, NULL)) {
      eng.log->addMsg(
        caster->getNameThe() + " disappears in a blast of smoke!");
    }
  }

  caster->teleport(false);
  return SpellCastRetData(true);
}

bool SpellTeleport::isGoodForMonsterToCastNow(
  Monster* const monster, Engine& eng) {
  bool blockers[MAP_W][MAP_H];
  MapParse::parse(CellPred::BlocksVision(eng), blockers);
  return monster->isSeeingActor(*(eng.player), blockers) &&
         monster->getHp() <= (monster->getHpMax(true) / 2) &&
         eng.dice.coinToss();
}

//------------------------------------------------------------ KNOCKBACK
SpellCastRetData SpellKnockBack::cast_(
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
  MapParse::parse(CellPred::BlocksVision(eng), blockers);
  return monster->isSeeingActor(*(eng.player), blockers);
}

//------------------------------------------------------------ ENFEEBLE
SpellCastRetData SpellEnfeeble::cast_(
  Actor* const caster, Engine& eng) {

  const PropId propId = getPropId(eng);

  if(caster == eng.player) {
    vector<Actor*> targets;
    eng.player->getSpottedEnemies(targets);

    if(targets.empty()) {
      return SpellCastRetData(false);
    } else {
      vector<Pos> actorPositions;
      actorPositions.resize(0);

      for(Actor * a : targets) {actorPositions.push_back(a->pos);}

      eng.renderer->drawBlastAnimAtPositionsWithPlayerVision(
        actorPositions, clrMagenta);

      for(Actor * actor : targets) {
        PropHandler& propHlr = actor->getPropHandler();
        Prop* const prop = propHlr.makeProp(propId, propTurnsStd);
        propHlr.tryApplyProp(prop);
      }
      return SpellCastRetData(true);
    }
  } else {
    eng.renderer->drawBlastAnimAtPositionsWithPlayerVision(
      vector<Pos>(1, eng.player->pos), clrMagenta);

    PropHandler& propHandler = eng.player->getPropHandler();
    Prop* const prop = propHandler.makeProp(
                         propId, propTurnsStd);
    propHandler.tryApplyProp(prop);

    return SpellCastRetData(false);
  }
}

bool SpellEnfeeble::isGoodForMonsterToCastNow(
  Monster* const monster, Engine& eng) {
  bool blockers[MAP_W][MAP_H];
  MapParse::parse(CellPred::BlocksVision(eng), blockers);
  return monster->isSeeingActor(*(eng.player), blockers);
}

PropId SpellEnfeeble::getPropId(Engine& eng) const {
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

//------------------------------------------------------------ DISEASE
SpellCastRetData SpellDisease::cast_(
  Actor* const caster, Engine& eng) {
  if(caster == eng.player) {
    return SpellCastRetData(true);
  } else {
    eng.log->addMsg("A disease is starting to afflict my body!", clrMsgBad);
    eng.player->getPropHandler().tryApplyProp(
      new PropDiseased(eng, propTurnsSpecific, 50));
    return SpellCastRetData(false);
  }
}

bool SpellDisease::isGoodForMonsterToCastNow(
  Monster* const monster, Engine& eng) {
  bool blockers[MAP_W][MAP_H];
  MapParse::parse(CellPred::BlocksVision(eng), blockers);
  return eng.dice.coinToss() && monster->isSeeingActor(*eng.player, blockers);
}

//------------------------------------------------------------ SUMMON RANDOM
SpellCastRetData SpellSummonRandom::cast_(
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
  MapParse::parse(CellPred::BlocksMoveCmn(true, eng), blockers);

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

  vector<ActorId> summonCandidates;
  for(int i = 1; i < endOfActorIds; i++) {
    const ActorData& data = eng.actorDataHandler->dataList[i];
    if(data.canBeSummoned) {
      if(data.spawnMinDLVL <= caster->getData().spawnMinDLVL) {
        summonCandidates.push_back(ActorId(i));
      }
    }
  }
  const int ELEMENT = eng.dice.range(1, summonCandidates.size() - 1);
  const ActorId id = summonCandidates.at(ELEMENT);
  Actor* const actor = eng.actorFactory->spawnActor(id, summonPos);
  Monster* monster = dynamic_cast<Monster*>(actor);
  monster->awareOfPlayerCounter_ = monster->getData().nrTurnsAwarePlayer;
  if(eng.map->cells[summonPos.x][summonPos.y].isSeenByPlayer) {
    eng.log->addMsg(monster->getNameA() + " appears.");
  }
  return SpellCastRetData(false);
}

bool SpellSummonRandom::isGoodForMonsterToCastNow(
  Monster* const monster, Engine& eng) {

  bool blockers[MAP_W][MAP_H];
  MapParse::parse(CellPred::BlocksVision(eng), blockers);
  return monster->isSeeingActor(*(eng.player), blockers) ||
         (eng.dice.oneIn(20));
}

//------------------------------------------------------------ HEAL SELF
SpellCastRetData SpellHealSelf::cast_(
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
SpellCastRetData SpellMiGoHypnosis::cast_(
  Actor* const caster, Engine& eng) {

  (void)caster;
  eng.log->addMsg("There is a sharp droning in my head!");

  if(eng.dice.coinToss()) {
    eng.player->getPropHandler().tryApplyProp(
      new PropFainted(eng, propTurnsSpecific, eng.dice.range(2, 10)));
  } else {
    eng.log->addMsg("I feel dizzy.");
  }

  return true;
}

bool SpellMiGoHypnosis::isGoodForMonsterToCastNow(
  Monster* const monster, Engine& eng) {

  bool blockers[MAP_W][MAP_H];
  MapParse::parse(CellPred::BlocksVision(eng), blockers);
  return monster->isSeeingActor(*(eng.player), blockers) &&
         eng.dice.oneIn(4);
}
