#include "Spells.h"

#include <algorithm>

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
#include "PlayerBon.h"
#include "Utils.h"

namespace SpellHandling {

Spell* getRandomSpellForMonster() {

  vector<SpellId> candidates;
  for(int i = 0; i < int(SpellId::endOfSpellId); i++) {
    Spell* const spell = getSpellFromId(SpellId(i));
    if(spell->isAvailForAllMonsters()) {
      candidates.push_back(SpellId(i));
    }
    delete spell;
  }
  const int ELEMENT = Rnd::range(0, candidates.size() - 1);
  return getSpellFromId(candidates.at(ELEMENT));
}

Spell* getSpellFromId(const SpellId spellId) {
  switch(spellId) {
    case SpellId::slowEnemies:        return new SpellSlowEnemies; break;
    case SpellId::terrifyEnemies:     return new SpellTerrifyMon; break;
    case SpellId::paralyzeEnemies:    return new SpellParalyzeEnemies; break;
    case SpellId::disease:            return new SpellDisease; break;
    case SpellId::darkbolt:           return new SpellDarkbolt; break;
    case SpellId::azathothsWrath:     return new SpellAzathothsWrath; break;
    case SpellId::summonRandom:       return new SpellSummonRandom; break;
    case SpellId::healSelf:           return new SpellHealSelf; break;
    case SpellId::knockBack:          return new SpellKnockBack; break;
    case SpellId::teleport:           return new SpellTeleport; break;
    case SpellId::mayhem:             return new SpellMayhem; break;
    case SpellId::pestilence:         return new SpellPestilence; break;
    case SpellId::detItems:           return new SpellDetItems; break;
    case SpellId::detTraps:           return new SpellDetTraps; break;
    case SpellId::detMon:             return new SpellDetMon; break;
    case SpellId::opening:            return new SpellOpening; break;
    case SpellId::sacrLife:           return new SpellSacrLife; break;
    case SpellId::sacrSpi:            return new SpellSacrSpi; break;
    case SpellId::cloudMinds:         return new SpellCloudMinds; break;
    case SpellId::bless:              return new SpellBless; break;
    case SpellId::miGoHypnosis:       return new SpellMiGoHypnosis; break;
    case SpellId::immolation:         return new SpellImmolation; break;
    case SpellId::elemRes:            return new SpellElemRes; break;
    case SpellId::endOfSpellId: {} break;
  }
  assert(false && "No spell found for ID");
  return NULL;
}

} //SpellHandling

Range Spell::getSpiCost(const bool IS_BASE_COST_ONLY,
                        Actor* const caster) const {
  int costMax = getMaxSpiCost_();

  if(IS_BASE_COST_ONLY == false) {
    const int X0 = max(0, caster->pos.x - 1);
    const int Y0 = max(0, caster->pos.y - 1);
    const int X1 = min(MAP_W - 1, caster->pos.x + 1);
    const int Y1 = min(MAP_H - 1, caster->pos.y + 1);

    for(int y = Y0; y <= Y1; y++) {
      for(int x = X0; x <= X1; x++) {
        if(Map::cells[x][y].featureStatic->getId() == FeatureId::altar) {
          costMax -= 1;
          y = 9999;
          x = 9999;
        }
      }
    }

    if(caster == Map::player) {
      bool isWarlock      = false;
      bool isBloodSorc    = false;
      bool isSeer         = false;

      for(Trait id : PlayerBon::traitsPicked_) {
        switch(id) {
          case Trait::warlock:        isWarlock     = true; break;
          case Trait::bloodSorcerer:  isBloodSorc   = true; break;
          case Trait::seer:           isSeer        = true; break;
          default: {} break;
        }
      }

      if(isBloodSorc) costMax--;

      switch(getId()) {
        case SpellId::darkbolt:       {if(isWarlock)  costMax--;}    break;
        case SpellId::azathothsWrath: {if(isWarlock)  costMax--;}    break;
        case SpellId::mayhem:         {if(isWarlock)  costMax--;}    break;
        case SpellId::detMon:         {if(isSeer)     costMax--;}    break;
        case SpellId::detItems:       {if(isSeer)     costMax -= 3;} break;
        case SpellId::detTraps:       {if(isSeer)     costMax -= 3;} break;
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

SpellCastRetData Spell::cast(Actor* const caster,
                             const bool IS_INTRINSIC) const {
  trace << "SpellId::cast()..." << endl;
  if(caster->getPropHandler().allowCastSpells(true)) {
    if(caster == Map::player) {
      trace << "Spell: Player casting spell" << endl;
      const ShockSrc shockSrc = IS_INTRINSIC ?
                                ShockSrc::castIntrSpell :
                                ShockSrc::useStrangeItem;
      const int SHOCK_VALUE = IS_INTRINSIC ? getShockValueIntrCast() : 10;
      Map::player->incrShock(SHOCK_VALUE, shockSrc);
      if(Map::player->deadState == ActorDeadState::alive) {
        Audio::play(SfxId::spellGeneric);
      }
    } else {
      trace << "Spell: Monster casting spell" << endl;
      Monster* const monster = dynamic_cast<Monster*>(caster);
      if(Map::cells[monster->pos.x][monster->pos.y].isSeenByPlayer) {
        const string spellStr = monster->getData().spellCastMessage;
        Log::addMsg(spellStr);
      }
      monster->spellCoolDownCurrent = monster->getData().spellCooldownTurns;
    }

    if(IS_INTRINSIC) {
      const Range cost = getSpiCost(false, caster);
      caster->hitSpi(Rnd::range(cost), false);
    }
    SpellCastRetData ret(false);
    if(caster->deadState == ActorDeadState::alive) {
      ret = cast_(caster);
    }

    GameTime::actorDidAct();
    trace << "SpellId::cast() [DONE]" << endl;
    return ret;
  }
  trace << "SpellId::cast() [DONE]" << endl;
  return SpellCastRetData(false);
}

//------------------------------------------------------------ DARKBOLT
SpellCastRetData SpellDarkbolt::cast_(Actor* const caster) const {
  Actor* target = NULL;

  vector<Actor*> spottedActors;
  caster->getSpottedEnemies(spottedActors);
  if(spottedActors.empty()) {
    return SpellCastRetData(false);
  } else {
    target = Utils::getRandomClosestActor(caster->pos, spottedActors);
  }

  vector<Pos> line;
  LineCalc::calcNewLine(caster->pos, target->pos, true, 999, false, line);
  Renderer::drawMapAndInterface();
  const int LINE_SIZE = line.size();
  for(int i = 1; i < LINE_SIZE; i++) {
    const Pos& pos = line.at(i);
    if(Config::isTilesMode()) {
      Renderer::drawTile(TileId::blast1, Panel::map, pos, clrMagenta);
    } else {
      Renderer::drawGlyph('*', Panel::map, pos, clrMagenta);
    }
    Renderer::updateScreen();
    SdlWrapper::sleep(Config::getDelayProjectileDraw());
  }

  Renderer::drawBlastAnimAtPositions(
    vector<Pos> {target->pos}, clrMagenta);

  const string msgCmn = " struck by a blast!";
  bool isCharged = false;
  if(caster == Map::player) {
    Log::addMsg(target->getNameThe() + " is" + msgCmn, clrMsgGood);

    vector<PropId> props;
    Map::player->getPropHandler().getAllActivePropIds(props);
    isCharged =
      find(props.begin(), props.end(), propWarlockCharged) != props.end();

  } else {
    Log::addMsg("I am" + msgCmn, clrMsgBad);
  }

  target->getPropHandler().tryApplyProp(
    new PropParalyzed(propTurnsSpecific, 2));

  Range dmgRange(4, 10);
  const int DMG = isCharged ? dmgRange.upper : Rnd::range(dmgRange);

  target->hit(DMG, DmgType::physical, true);

  Snd snd("", SfxId::endOfSfxId, IgnoreMsgIfOriginSeen::yes, target->pos, NULL,
          SndVol::low, AlertsMonsters::yes);
  SndEmit::emitSnd(snd);

  return SpellCastRetData(true);
}

bool SpellDarkbolt::isGoodForMonsterToCastNow(
  Monster* const monster) {
  bool blockers[MAP_W][MAP_H];
  MapParse::parse(CellPred::BlocksVision(), blockers);
  return monster->isSeeingActor(*(Map::player), blockers) && Rnd::oneIn(2);
}

//------------------------------------------------------------ AZATHOTHS WRATH
SpellCastRetData SpellAzathothsWrath::cast_(
  Actor* const caster) const {

  Range dmgRange(2, 8);

  const string msgEnd = "struck by a roaring blast!";

  if(caster == Map::player) {
    vector<Actor*> targets;
    Map::player->getSpottedEnemies(targets);

    if(targets.empty()) {
      return SpellCastRetData(false);
    } else {
      vector<PropId> props;
      Map::player->getPropHandler().getAllActivePropIds(props);
      const bool IS_CHARGED =
        find(props.begin(), props.end(), propWarlockCharged) != props.end();

      vector<Pos> actorPositions; actorPositions.resize(0);
      for(Actor * a : targets) {actorPositions.push_back(a->pos);}

      Renderer::drawBlastAnimAtPositionsWithPlayerVision(
        actorPositions, clrRedLgt);

      for(Actor * actor : targets) {
        Log::addMsg(actor->getNameThe() + " is " + msgEnd, clrMsgGood);
        actor->getPropHandler().tryApplyProp(
          new PropParalyzed(propTurnsSpecific, 2));

        const int DMG = IS_CHARGED ? dmgRange.upper : Rnd::range(dmgRange);

        actor->hit(DMG, DmgType::physical, false);

        Snd snd("", SfxId::endOfSfxId, IgnoreMsgIfOriginSeen::yes, actor->pos, NULL,
                SndVol::high, AlertsMonsters::yes);
        SndEmit::emitSnd(snd);
      }
      return SpellCastRetData(true);
    }
  } else {
    Log::addMsg("I am " + msgEnd, clrMsgBad);
    Renderer::drawBlastAnimAtPositionsWithPlayerVision(
      vector<Pos> {Map::player->pos}, clrRedLgt);
    Map::player->getPropHandler().tryApplyProp(
      new PropParalyzed(propTurnsSpecific, 1));
    Map::player->hit(Rnd::range(dmgRange), DmgType::physical, false);
    Snd snd("", SfxId::endOfSfxId, IgnoreMsgIfOriginSeen::yes, Map::player->pos, NULL,
            SndVol::high, AlertsMonsters::yes);
    SndEmit::emitSnd(snd);
  }
  return SpellCastRetData(false);
}

bool SpellAzathothsWrath::isGoodForMonsterToCastNow(
  Monster* const monster) {
  bool blockers[MAP_W][MAP_H];
  MapParse::parse(CellPred::BlocksVision(), blockers);
  return monster->isSeeingActor(*(Map::player), blockers);
}

//------------------------------------------------------------ MAYHEM
SpellCastRetData SpellMayhem::cast_(
  Actor* const caster) const {
  (void)caster;

  Log::addMsg("Destruction rages around me!");

  const Pos& playerPos = Map::player->pos;

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
              Map::cells[x + dx][y + dy].featureStatic;
            if(f->canMoveCmn()) {
              isAdjToWalkableCell = true;
            }
          }
        }
        if(isAdjToWalkableCell) {
          const int CHANCE_TO_DESTROY = 10;
          if(Rnd::percentile() < CHANCE_TO_DESTROY) {
            Map::switchToDestroyedFeatAt(c);
          }
        }
      }
    }
  }

  for(int y = Y0; y <= Y1; y++) {
    for(int x = X0; x <= X1; x++) {
      FeatureStatic* const f =
        Map::cells[x][y].featureStatic;
      if(f->canHaveBlood()) {
        const int CHANCE_FOR_BLOOD = 10;
        if(Rnd::percentile() < CHANCE_FOR_BLOOD) {
          f->setHasBlood(true);
        }
      }
    }
  }

  for(Actor * actor : GameTime::actors_) {
    if(actor != Map::player) {
      if(Map::player->isSeeingActor(*actor, NULL)) {
        actor->getPropHandler().tryApplyProp(
          new PropBurning(propTurnsStd));
      }
    }
  }

  Snd snd("", SfxId::endOfSfxId, IgnoreMsgIfOriginSeen::yes, Map::player->pos,
          NULL, SndVol::high, AlertsMonsters::yes);
  SndEmit::emitSnd(snd);

  return SpellCastRetData(true);
}

//------------------------------------------------------------ PESTILENCE
SpellCastRetData SpellPestilence::cast_(
  Actor* const caster) const {
  (void)caster;

  const int RND = Rnd::range(1, 4);
  const ActorId monsterId = RND == 1 ? actor_greenSpider  :
                            RND == 2 ? actor_redSpider    :
                            RND == 3 ? actor_rat          : actor_wormMass;

  const size_t NR_MON = Rnd::range(7, 10);

  Log::addMsg("Disgusting critters appear around me!");

  ActorFactory::summonMonsters(
    Map::player->pos, vector<ActorId> {NR_MON, monsterId}, true);

  return SpellCastRetData(true);
}

//------------------------------------------------------------ DETECT ITEMS
SpellCastRetData SpellDetItems::cast_(Actor* const caster) const {
  (void)caster;

  const int RADI    = FOV_STD_RADI_INT + 3;
  const int ORIG_X  = Map::player->pos.x;
  const int ORIG_Y  = Map::player->pos.y;
  const int X0      = max(0, ORIG_X - RADI);
  const int Y0      = max(0, ORIG_Y - RADI);
  const int X1      = min(MAP_W - 1, ORIG_X + RADI);
  const int Y1      = min(MAP_H - 1, ORIG_Y + RADI);

  vector<Pos> itemsRevealedPositions;

  for(int y = Y0; y < Y1; y++) {
    for(int x = X0; x <= X1; x++) {
      Item* item = Map::cells[x][y].item;
      if(item != NULL) {
        Map::cells[x][y].isSeenByPlayer = true;
        Map::cells[x][y].isExplored = true;
        itemsRevealedPositions.push_back(Pos(x, y));
      }
    }
  }
  if(itemsRevealedPositions.empty() == false) {
    Renderer::drawMapAndInterface();
    Map::player->updateFov();
    Renderer::drawBlastAnimAtPositions(
      itemsRevealedPositions, clrWhite);
    Renderer::drawMapAndInterface();

    if(itemsRevealedPositions.size() == 1) {
      Log::addMsg("An item is revealed to me.");
    }
    if(itemsRevealedPositions.size() > 1) {
      Log::addMsg("Some items are revealed to me.");
    }
    return SpellCastRetData(true);
  }
  return SpellCastRetData(false);
}

//------------------------------------------------------------ DETECT TRAPS
SpellCastRetData SpellDetTraps::cast_(Actor* const caster) const {
  (void)caster;

  vector<Pos> trapsRevealedPositions;

  for(int x = 0; x < MAP_W; x++) {
    for(int y = 0; y < MAP_H; y++) {
      if(Map::cells[x][y].isSeenByPlayer) {
        FeatureStatic* const f = Map::cells[x][y].featureStatic;
        if(f->getId() == FeatureId::trap) {
          Trap* const trap = dynamic_cast<Trap*>(f);
          trap->reveal(false);
          trapsRevealedPositions.push_back(Pos(x, y));
        }
      }
    }
  }

  if(trapsRevealedPositions.empty() == false) {
    Renderer::drawMapAndInterface();
    Map::player->updateFov();
    Renderer::drawBlastAnimAtPositions(
      trapsRevealedPositions, clrWhite);
    Renderer::drawMapAndInterface();
    if(trapsRevealedPositions.size() == 1) {
      Log::addMsg("A hidden trap is revealed to me.");
    }
    if(trapsRevealedPositions.size() > 1) {
      Log::addMsg("Some hidden traps are revealed to me.");
    }
    return SpellCastRetData(true);
  }
  return SpellCastRetData(false);
}

//------------------------------------------------------------ DETECT MONSTERS
SpellCastRetData SpellDetMon::cast_(Actor* const caster) const {
  (void)caster;

  bool isSeer           = PlayerBon::hasTrait(Trait::seer);
  const int MULTIPLIER  = 6 * (isSeer ? 3 : 1);

  const int MAX_DIST    = FOV_STD_RADI_INT * 2;

  const Pos playerPos   = Map::player->pos;

  bool didDetect        = false;

  for(Actor * actor : GameTime::actors_) {
    if(actor != Map::player) {
      if(Utils::kingDist(playerPos, actor->pos) <= MAX_DIST) {
        dynamic_cast<Monster*>(actor)->playerBecomeAwareOfMe(MULTIPLIER);
        didDetect = true;
      }
    }
  }

  if(didDetect) {Log::addMsg("I detect monsters.");}

  return SpellCastRetData(didDetect);
}

//------------------------------------------------------------ OPENING
SpellCastRetData SpellOpening::cast_(
  Actor* const caster) const {

  (void)caster;

  vector<Pos> featuresOpenedPositions;

  for(int y = 1; y < MAP_H - 1; y++) {
    for(int x = 1; x < MAP_W - 1; x++) {
      if(Map::cells[x][y].isSeenByPlayer) {
        if(Map::cells[x][y].featureStatic->open()) {
          featuresOpenedPositions.push_back(Pos(x, y));
        }
      }
    }
  }

  if(featuresOpenedPositions.empty() == false) {
    Renderer::drawMapAndInterface();
    Map::player->updateFov();
    Renderer::drawBlastAnimAtPositions(
      featuresOpenedPositions, clrWhite);
    Renderer::drawMapAndInterface();
    return SpellCastRetData(true);
  } else {
    return SpellCastRetData(false);
  }
}

//------------------------------------------------------------ SACRIFICE LIFE
SpellCastRetData SpellSacrLife::cast_(Actor* const caster) const {
  (void)caster;

  //Convert every 2 HP to 1 SPI

  const int PLAYER_HP_CUR = Map::player->getHp();

  if(PLAYER_HP_CUR > 2) {
    const int HP_DRAINED = ((PLAYER_HP_CUR - 1) / 2) * 2;
    Map::player->hit(HP_DRAINED, DmgType::pure, false);
    Map::player->restoreSpi(HP_DRAINED, true, true);
    return SpellCastRetData(true);
  }
  return SpellCastRetData(false);
}

//------------------------------------------------------------ SACRIFICE SPIRIT
SpellCastRetData SpellSacrSpi::cast_(Actor* const caster) const {
  (void)caster;

  //Convert every SPI to HP

  const int PLAYER_SPI_CUR = Map::player->getSpi();

  if(PLAYER_SPI_CUR > 0) {
    const int HP_DRAINED = PLAYER_SPI_CUR - 1;
    Map::player->hitSpi(HP_DRAINED, true);
    Map::player->restoreHp(HP_DRAINED, true, true);
    return SpellCastRetData(true);
  }
  return SpellCastRetData(false);
}

//------------------------------------------------------------ ROGUE HIDE
SpellCastRetData SpellCloudMinds::cast_(
  Actor* const caster) const {

  (void)caster;
  Log::addMsg("I vanish from the minds of my enemies.");

  for(Actor * actor : GameTime::actors_) {
    if(actor != Map::player) {
      Monster* const monster = dynamic_cast<Monster*>(actor);
      monster->awareOfPlayerCounter_ = 0;
    }
  }
  return SpellCastRetData(true);
}

//------------------------------------------------------------ BLESS
SpellCastRetData SpellBless::cast_(
  Actor* const caster) const {

  caster->getPropHandler().tryApplyProp(
    new PropBlessed(propTurnsStd));

  return SpellCastRetData(true);
}

bool SpellBless::isGoodForMonsterToCastNow(
  Monster* const monster) {
  (void)eng;

  vector<PropId> props;
  monster->getPropHandler().getAllActivePropIds(props);
  return find(props.begin(), props.end(), propBlessed) == props.end();
}

//------------------------------------------------------------ TELEPORT
SpellCastRetData SpellTeleport::cast_(
  Actor* const caster) const {

  if(caster != Map::player) {
    if(Map::player->isSeeingActor(*caster, NULL)) {
      Log::addMsg(
        caster->getNameThe() + " disappears in a blast of smoke!");
    }
  }

  caster->teleport(false);
  return SpellCastRetData(true);
}

bool SpellTeleport::isGoodForMonsterToCastNow(
  Monster* const monster) {
  bool blockers[MAP_W][MAP_H];
  MapParse::parse(CellPred::BlocksVision(), blockers);
  return monster->isSeeingActor(*(Map::player), blockers) &&
         monster->getHp() <= (monster->getHpMax(true) / 2) &&
         Rnd::coinToss();
}

//------------------------------------------------------------ ELEMENTAL RES
SpellCastRetData SpellElemRes::cast_(Actor* const caster) const {
  const int DURATION = 20;
  PropRCold* rCold = new PropRCold(propTurnsSpecific, DURATION);
  PropRElec* rElec = new PropRElec(propTurnsSpecific, DURATION);
  PropRFire* rFire = new PropRFire(propTurnsSpecific, DURATION);
  PropHandler& propHlr = caster->getPropHandler();
  propHlr.tryApplyProp(rCold);
  propHlr.tryApplyProp(rElec);
  propHlr.tryApplyProp(rFire);
  return SpellCastRetData(true);
}

bool SpellElemRes::isGoodForMonsterToCastNow(
  Monster* const monster) {
  bool blockers[MAP_W][MAP_H];
  MapParse::parse(CellPred::BlocksVision(), blockers);
  return monster->isSeeingActor(*(Map::player), blockers) && Rnd::oneIn(3);
}

//------------------------------------------------------------ KNOCKBACK
SpellCastRetData SpellKnockBack::cast_(
  Actor* const caster) const {
  if(caster == Map::player) {

  } else {
    Log::addMsg("A force pushes me!", clrMsgBad);
    KnockBack::tryKnockBack(*(Map::player), caster->pos, false);
  }
  return SpellCastRetData(false);
}

bool SpellKnockBack::isGoodForMonsterToCastNow(
  Monster* const monster) {
  bool blockers[MAP_W][MAP_H];
  MapParse::parse(CellPred::BlocksVision(), blockers);
  return monster->isSeeingActor(*(Map::player), blockers);
}

//------------------------------------------------------------ PROP ON OTHERS
SpellCastRetData SpellPropOnEnemies::cast_(
  Actor* const caster) const {

  const PropId propId = getPropId();

  if(caster == Map::player) {
    vector<Actor*> targets;
    Map::player->getSpottedEnemies(targets);

    if(targets.empty()) {
      return SpellCastRetData(false);
    } else {
      vector<Pos> actorPositions;
      actorPositions.resize(0);

      for(Actor * a : targets) {actorPositions.push_back(a->pos);}

      Renderer::drawBlastAnimAtPositionsWithPlayerVision(
        actorPositions, clrMagenta);

      for(Actor * actor : targets) {
        PropHandler& propHlr = actor->getPropHandler();
        Prop* const prop = propHlr.makeProp(propId, propTurnsStd);
        propHlr.tryApplyProp(prop);
      }
      return SpellCastRetData(true);
    }
  } else {
    Renderer::drawBlastAnimAtPositionsWithPlayerVision(
      vector<Pos>(1, Map::player->pos), clrMagenta);

    PropHandler& propHandler = Map::player->getPropHandler();
    Prop* const prop = propHandler.makeProp(
                         propId, propTurnsStd);
    propHandler.tryApplyProp(prop);

    return SpellCastRetData(false);
  }
}

bool SpellPropOnEnemies::isGoodForMonsterToCastNow(
  Monster* const monster) {
  bool blockers[MAP_W][MAP_H];
  MapParse::parse(CellPred::BlocksVision(), blockers);
  return monster->isSeeingActor(*(Map::player), blockers);
}

//------------------------------------------------------------ DISEASE
SpellCastRetData SpellDisease::cast_(
  Actor* const caster) const {
  if(caster == Map::player) {
    return SpellCastRetData(true);
  } else {
    Log::addMsg("A disease is starting to afflict my body!", clrMsgBad);
    Map::player->getPropHandler().tryApplyProp(
      new PropDiseased(propTurnsSpecific, 50));
    return SpellCastRetData(false);
  }
}

bool SpellDisease::isGoodForMonsterToCastNow(
  Monster* const monster) {
  bool blockers[MAP_W][MAP_H];
  MapParse::parse(CellPred::BlocksVision(), blockers);
  return Rnd::coinToss() && monster->isSeeingActor(*Map::player, blockers);
}

//------------------------------------------------------------ SUMMON RANDOM
SpellCastRetData SpellSummonRandom::cast_(
  Actor* const caster) const {

  Pos summonPos(caster->pos);

  vector<Pos> freePositionsSeenByPlayer;
  const int RADI = FOV_STD_RADI_INT;
  const Pos playerPos(Map::player->pos);
  const int X0 = max(0, playerPos.x - RADI);
  const int Y0 = max(0, playerPos.y - RADI);
  const int X1 = min(MAP_W, playerPos.x + RADI) - 1;
  const int Y1 = min(MAP_H, playerPos.y + RADI) - 1;
  for(int x = X0; x <= X1; x++) {
    for(int y = Y0; y <= Y1; y++) {
      if(Map::cells[x][y].isSeenByPlayer) {
        freePositionsSeenByPlayer.push_back(Pos(x, y));
      }
    }
  }

  bool blockers[MAP_W][MAP_H];
  MapParse::parse(CellPred::BlocksMoveCmn(true), blockers);

  for(int i = 0; i < int(freePositionsSeenByPlayer.size()); i++) {
    const Pos pos(freePositionsSeenByPlayer.at(i));
    if(blockers[pos.x][pos.y]) {
      freePositionsSeenByPlayer.erase(freePositionsSeenByPlayer.begin() + i);
      i--;
    }
  }

  if(freePositionsSeenByPlayer.empty()) {
    vector<Pos> freeCellsVector;
    Utils::makeVectorFromBoolMap(false, blockers, freeCellsVector);
    if(freeCellsVector.empty() == false) {
      sort(freeCellsVector.begin(), freeCellsVector.end(),
           IsCloserToOrigin(caster->pos));
      summonPos = freeCellsVector.at(0);
    }
  } else {
    const int ELEMENT = Rnd::range(0, freePositionsSeenByPlayer.size() - 1);
    summonPos = freePositionsSeenByPlayer.at(ELEMENT);
  }

  vector<ActorId> summonCandidates;
  for(int i = 1; i < endOfActorIds; i++) {
    const ActorDataT& data = ActorData::dataList[i];
    if(data.canBeSummoned) {
      if(data.spawnMinDLVL <= caster->getData().spawnMinDLVL) {
        summonCandidates.push_back(ActorId(i));
      }
    }
  }
  const int ELEMENT = Rnd::range(1, summonCandidates.size() - 1);
  const ActorId id = summonCandidates.at(ELEMENT);
  Actor* const actor = ActorFactory::spawnActor(id, summonPos);
  Monster* monster = dynamic_cast<Monster*>(actor);
  monster->awareOfPlayerCounter_ = monster->getData().nrTurnsAwarePlayer;
  if(Map::cells[summonPos.x][summonPos.y].isSeenByPlayer) {
    Log::addMsg(monster->getNameA() + " appears.");
  }
  return SpellCastRetData(false);
}

bool SpellSummonRandom::isGoodForMonsterToCastNow(
  Monster* const monster) {

  bool blockers[MAP_W][MAP_H];
  MapParse::parse(CellPred::BlocksVision(), blockers);
  return monster->isSeeingActor(*(Map::player), blockers) ||
         (Rnd::oneIn(20));
}

//------------------------------------------------------------ HEAL SELF
SpellCastRetData SpellHealSelf::cast_(
  Actor* const caster) const {
  (void)eng;
  return SpellCastRetData(caster->restoreHp(999, true));
}

bool SpellHealSelf::isGoodForMonsterToCastNow(
  Monster* const monster) {
  (void)eng;
  return monster->getHp() < monster->getHpMax(true);
}

//------------------------------------------------------------ MI-GO HYPNOSIS
SpellCastRetData SpellMiGoHypnosis::cast_(
  Actor* const caster) const {

  (void)caster;
  Log::addMsg("There is a sharp droning in my head!");

  if(Rnd::coinToss()) {
    Map::player->getPropHandler().tryApplyProp(
      new PropFainted(propTurnsSpecific, Rnd::range(2, 10)));
  } else {
    Log::addMsg("I feel dizzy.");
  }

  return true;
}

bool SpellMiGoHypnosis::isGoodForMonsterToCastNow(
  Monster* const monster) {

  bool blockers[MAP_W][MAP_H];
  MapParse::parse(CellPred::BlocksVision(), blockers);
  return monster->isSeeingActor(*(Map::player), blockers) &&
         Rnd::oneIn(4);
}

//------------------------------------------------------------ IMMOLATION
SpellCastRetData SpellImmolation::cast_(
  Actor* const caster) const {

  (void)caster;

  Log::addMsg("Flames are rising around me!");

  Map::player->getPropHandler().tryApplyProp(
    new PropBurning(propTurnsSpecific, Rnd::range(3, 4)));

  return true;
}

bool SpellImmolation::isGoodForMonsterToCastNow(
  Monster* const monster) {

  bool blockers[MAP_W][MAP_H];
  MapParse::parse(CellPred::BlocksVision(), blockers);
  return monster->isSeeingActor(*(Map::player), blockers) &&
         Rnd::oneIn(4);
}
