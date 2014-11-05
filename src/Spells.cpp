#include "Spells.h"

#include <algorithm>
#include <vector>
#include <assert.h>

#include "Init.h"
#include "Render.h"
#include "ActorMonster.h"
#include "ActorPlayer.h"
#include "Log.h"
#include "Postmortem.h"
#include "Knockback.h"
#include "Map.h"
#include "ActorFactory.h"
#include "FeatureTrap.h"
#include "PlayerSpellsHandling.h"
#include "ItemScroll.h"
#include "Inventory.h"
#include "MapParsing.h"
#include "LineCalc.h"
#include "SdlWrapper.h"
#include "PlayerBon.h"
#include "Utils.h"

using namespace std;

namespace SpellHandling
{

Spell* getRandomSpellForMon()
{

  vector<SpellId> bucket;
  for(int i = 0; i < int(SpellId::END); ++i)
  {
    Spell* const spell = mkSpellFromId(SpellId(i));
    if(spell->isAvailForAllMon())
    {
      bucket.push_back(SpellId(i));
    }
    delete spell;
  }
  const int ELEMENT = Rnd::range(0, bucket.size() - 1);
  return mkSpellFromId(bucket.at(ELEMENT));
}

Spell* mkSpellFromId(const SpellId spellId)
{
  switch(spellId)
  {
    case SpellId::slowMon:              return new SpellSlowMon; break;
    case SpellId::terrifyMon:           return new SpellTerrifyMon; break;
    case SpellId::paralyzeMon:          return new SpellParalyzeMon; break;
    case SpellId::disease:              return new SpellDisease; break;
    case SpellId::darkbolt:             return new SpellDarkbolt; break;
    case SpellId::azathothsWrath:       return new SpellAzathothsWrath; break;
    case SpellId::summonRandom:         return new SpellSummonRandom; break;
    case SpellId::healSelf:             return new SpellHealSelf; break;
    case SpellId::knockBack:            return new SpellKnockBack; break;
    case SpellId::teleport:             return new SpellTeleport; break;
    case SpellId::mayhem:               return new SpellMayhem; break;
    case SpellId::pestilence:           return new SpellPestilence; break;
    case SpellId::detItems:             return new SpellDetItems; break;
    case SpellId::detTraps:             return new SpellDetTraps; break;
    case SpellId::detMon:               return new SpellDetMon; break;
    case SpellId::opening:              return new SpellOpening; break;
    case SpellId::sacrLife:             return new SpellSacrLife; break;
    case SpellId::sacrSpi:              return new SpellSacrSpi; break;
    case SpellId::cloudMinds:           return new SpellCloudMinds; break;
    case SpellId::bless:                return new SpellBless; break;
    case SpellId::miGoHypnosis:         return new SpellMiGoHypnosis; break;
    case SpellId::immolation:           return new SpellImmolation; break;
    case SpellId::elemRes:              return new SpellElemRes; break;
    case SpellId::pharaohStaff:         return new SpellPharaohStaff; break;
    case SpellId::END: {} break;
  }
  assert(false && "No spell found for ID");
  return nullptr;
}

} //SpellHandling

Range Spell::getSpiCost(const bool IS_BASE_COST_ONLY, Actor* const caster) const
{
  int costMax = getMaxSpiCost_();

  if(!IS_BASE_COST_ONLY)
  {
    const int X0 = max(0, caster->pos.x - 1);
    const int Y0 = max(0, caster->pos.y - 1);
    const int X1 = min(MAP_W - 1, caster->pos.x + 1);
    const int Y1 = min(MAP_H - 1, caster->pos.y + 1);

    for(int y = Y0; y <= Y1; ++y)
    {
      for(int x = X0; x <= X1; ++x)
      {
        if(Map::cells[x][y].rigid->getId() == FeatureId::altar)
        {
          costMax -= 1;
          y = 9999;
          x = 9999;
        }
      }
    }

    if(caster->isPlayer())
    {
      bool isWarlock      = false;
      bool isBloodSorc    = false;
      bool isSeer         = false;

      for(Trait id : PlayerBon::traitsPicked_)
      {
        switch(id)
        {
          case Trait::warlock:        isWarlock     = true; break;
          case Trait::bloodSorcerer:  isBloodSorc   = true; break;
          case Trait::seer:           isSeer        = true; break;
          default: {} break;
        }
      }

      if(isBloodSorc)
        costMax--;

      switch(getId())
      {
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

    if(!propHlr.allowSee())
      --costMax;

    if(find(begin(props), end(props), propBlessed) != end(props))
      --costMax;

    if(find(begin(props), end(props), propCursed) != end(props))
      costMax += 3;
  }

  costMax             = max(1, costMax);
  const int COST_MIN  = max(1, costMax / 2);

  return Range(COST_MIN, costMax);
}

SpellEffectNoticed Spell::cast(Actor* const caster, const bool IS_INTRINSIC) const
{
  TRACE_FUNC_BEGIN;
  if(caster->getPropHandler().allowCastSpells(true))
  {
    if(caster->isPlayer())
    {
      TRACE << "Player casting spell" << endl;
      const ShockSrc shockSrc = IS_INTRINSIC ?
                                ShockSrc::castIntrSpell :
                                ShockSrc::useStrangeItem;
      const int SHOCK_VALUE = IS_INTRINSIC ? getShockValueIntrCast() : 10;
      Map::player->incrShock(SHOCK_VALUE, shockSrc);
      if(Map::player->isAlive())
      {
        Audio::play(SfxId::spellGeneric);
      }
    }
    else //Caster is not player
    {
      TRACE << "Monster casting spell" << endl;
      Mon* const mon = static_cast<Mon*>(caster);
      if(Map::cells[mon->pos.x][mon->pos.y].isSeenByPlayer)
      {
        const string spellStr = mon->getData().spellCastMessage;
        Log::addMsg(spellStr);
      }
      mon->spellCoolDownCur = mon->getData().spellCooldownTurns;
    }

    if(IS_INTRINSIC)
    {
      const Range cost = getSpiCost(false, caster);
      caster->hitSpi(Rnd::range(cost), false);
    }

    SpellEffectNoticed isNoticed = SpellEffectNoticed::no;

    if(caster->isAlive())
      isNoticed = cast_(caster);

    GameTime::actorDidAct();
    TRACE_FUNC_END;
    return isNoticed;
  }
  TRACE_FUNC_END;
  return SpellEffectNoticed::no;
}

//------------------------------------------------------------ DARKBOLT
SpellEffectNoticed SpellDarkbolt::cast_(Actor* const caster) const
{
  Actor* tgt = nullptr;

  vector<Actor*> spottedActors;
  caster->getSeenFoes(spottedActors);
  if(spottedActors.empty())
  {
    return SpellEffectNoticed::no;
  }
  else
  {
    tgt = Utils::getRandomClosestActor(caster->pos, spottedActors);
  }

  vector<Pos> line;
  LineCalc::calcNewLine(caster->pos, tgt->pos, true, 999, false, line);
  Render::drawMapAndInterface();
  const size_t LINE_SIZE = line.size();
  for(size_t i = 1; i < LINE_SIZE; ++i)
  {
    const Pos& p = line.at(i);
    if(Config::isTilesMode())
    {
      Render::drawTile(TileId::blast1, Panel::map, p, clrMagenta);
    }
    else
    {
      Render::drawGlyph('*', Panel::map, p, clrMagenta);
    }
    Render::updateScreen();
    SdlWrapper::sleep(Config::getDelayProjectileDraw());
  }

  Render::drawBlastAtCells(vector<Pos> {tgt->pos}, clrMagenta);

  bool    isPlayerSeeingTgt = true;
  bool    isCharged         = false;
  Clr     msgClr            = clrMsgGood;
  if(caster->isPlayer())
  {
    if(caster->isLeaderOf(*tgt))
    {
      msgClr = clrMsgBad;
    }

    vector<PropId> props;
    Map::player->getPropHandler().getAllActivePropIds(props);
    isCharged = find(begin(props), end(props), propWarlockCharged) != end(props);

  }
  else //Caster is not player
  {
    if(tgt->isPlayer() || Map::player->isLeaderOf(*tgt))
    {
      msgClr = clrMsgBad;
    }

    isPlayerSeeingTgt = Map::player->isSeeingActor(*tgt, nullptr);
  }

  if(isPlayerSeeingTgt)
  {
    string tgtRefStr = "I am";
    if(!tgt->isPlayer())
    {
      tgtRefStr = tgt->getNameThe() + " is";
    }
    Log::addMsg(tgtRefStr + " struck by a blast!", clrMsgBad);
  }

  tgt->getPropHandler().tryApplyProp(new PropParalyzed(PropTurns::specific, 2));

  Range dmgRange(4, 10);
  const int DMG = isCharged ? dmgRange.upper : Rnd::range(dmgRange);

  tgt->hit(DMG, DmgType::physical);

  SndEmit::emitSnd({"", SfxId::END, IgnoreMsgIfOriginSeen::yes, tgt->pos, nullptr,
                    SndVol::low, AlertsMon::yes
                   });

  return SpellEffectNoticed::yes;
}

bool SpellDarkbolt::allowMonCastNow(Mon& mon,
                                    const bool losBlockers[MAP_W][MAP_H]) const
{
  return mon.target                                   &&
         mon.isSeeingActor(*mon.target, losBlockers)  &&
         Rnd::oneIn(2);
}

//------------------------------------------------------------ AZATHOTHS WRATH
SpellEffectNoticed SpellAzathothsWrath::cast_(Actor* const caster) const
{
  Range dmgRange(4, 8);

  const string msgEnd = "struck by a roaring blast!";

  if(caster->isPlayer())
  {
    vector<Actor*> targets;
    Map::player->getSeenFoes(targets);

    if(targets.empty())
    {
      return SpellEffectNoticed::no;
    }
    else
    {
      vector<PropId> props;
      Map::player->getPropHandler().getAllActivePropIds(props);

      const bool IS_CHARGED =
        find(begin(props), end(props), propWarlockCharged) != end(props);

      vector<Pos> actorCells; actorCells.clear();
      for(Actor* a : targets) {actorCells.push_back(a->pos);}

      Render::drawBlastAtCellsWithVision(actorCells, clrRedLgt);

      for(Actor* actor : targets)
      {
        Log::addMsg(actor->getNameThe() + " is " + msgEnd, clrMsgGood);
        actor->getPropHandler().tryApplyProp(new PropParalyzed(PropTurns::specific, 2));

        const int DMG = IS_CHARGED ? dmgRange.upper : Rnd::range(dmgRange);

        actor->hit(DMG, DmgType::physical);

        SndEmit::emitSnd({"", SfxId::END, IgnoreMsgIfOriginSeen::yes, actor->pos,
                          nullptr, SndVol::high, AlertsMon::yes
                         });
      }
      return SpellEffectNoticed::yes;
    }
  }
  else //Caster is not player
  {
    Log::addMsg("I am " + msgEnd, clrMsgBad);

    Render::drawBlastAtCellsWithVision(vector<Pos> {Map::player->pos}, clrRedLgt);

    Map::player->getPropHandler().tryApplyProp(new PropParalyzed(PropTurns::specific, 1));

    Map::player->hit(Rnd::range(dmgRange), DmgType::physical);

    SndEmit::emitSnd({"", SfxId::END, IgnoreMsgIfOriginSeen::yes, Map::player->pos,
                      nullptr, SndVol::high, AlertsMon::yes
                     });
  }
  return SpellEffectNoticed::no;
}

bool SpellAzathothsWrath::allowMonCastNow(Mon& mon,
    const bool losBlockers[MAP_W][MAP_H]) const
{
  return mon.target && mon.isSeeingActor(*mon.target, losBlockers);
}

//------------------------------------------------------------ MAYHEM
SpellEffectNoticed SpellMayhem::cast_(Actor* const caster) const
{
  (void)caster;

  Log::addMsg("Destruction rages around me!");

  const Pos& playerPos = Map::player->pos;

  const int NR_OF_SWEEPS  = 5;
  const int AREA_RADI     = FOV_STD_RADI_INT;

  const int X0 = max(1, playerPos.x - AREA_RADI);
  const int Y0 = max(1, playerPos.y - AREA_RADI);
  const int X1 = min(MAP_W - 1, playerPos.x + AREA_RADI) - 1;
  const int Y1 = min(MAP_H - 1, playerPos.y + AREA_RADI) - 1;

  for(int i = 0; i < NR_OF_SWEEPS; ++i)
  {
    for(int y = Y0; y <= Y1; ++y)
    {
      for(int x = X0; x <= X1; ++x)
      {
        bool isAdjToWalkableCell = false;
        for(int dx = -1; dx <= 1; ++dx)
        {
          for(int dy = -1; dy <= 1; ++dy)
          {
            const Rigid* const f = Map::cells[x + dx][y + dy].rigid;
            if(f->canMoveCmn()) {isAdjToWalkableCell = true;}
          }
        }
        if(isAdjToWalkableCell && Rnd::oneIn(8))
        {
          Map::cells[x][y].rigid->hit(DmgType::physical, DmgMethod::explosion);
        }
      }
    }
  }

  for(int y = Y0; y <= Y1; ++y)
  {
    for(int x = X0; x <= X1; ++x)
    {
      auto* const f = Map::cells[x][y].rigid;
      if(f->canHaveBlood() && Rnd::oneIn(10)) {f->mkBloody();}
    }
  }

  for(auto* actor : GameTime::actors_)
  {
    if(actor != Map::player)
    {
      if(Map::player->isSeeingActor(*actor, nullptr))
      {
        actor->getPropHandler().tryApplyProp(new PropBurning(PropTurns::std));
      }
    }
  }

  SndEmit::emitSnd({"", SfxId::END, IgnoreMsgIfOriginSeen::yes, Map::player->pos,
                    nullptr, SndVol::high, AlertsMon::yes
                   });

  return SpellEffectNoticed::yes;
}

//------------------------------------------------------------ PESTILENCE
SpellEffectNoticed SpellPestilence::cast_(Actor* const caster) const
{
  (void)caster;

  const int RND = Rnd::range(1, 4);
  const ActorId monsterId = RND == 1 ? ActorId::greenSpider :
                            RND == 2 ? ActorId::redSpider   :
                            RND == 3 ? ActorId::rat         :
                            ActorId::wormMass;

  const size_t NR_MON = Rnd::range(7, 10);

  Log::addMsg("Disgusting critters appear around me!");

  ActorFactory::summonMon(Map::player->pos, {NR_MON, monsterId}, true, caster);

  return SpellEffectNoticed::yes;
}

//------------------------------------------------------------ PHARAOH STAFF LOCUSTS
bool SpellPharaohStaff::allowMonCastNow(Mon& mon,
                                        const bool losBlockers[MAP_W][MAP_H]) const
{
  return mon.target                                   &&
         mon.isSeeingActor(*mon.target, losBlockers)  &&
         Rnd::oneIn(4);
}

SpellEffectNoticed SpellPharaohStaff::cast_(Actor* const caster) const
{
  //First check for a friendly mummy and heal it (as per the spell description)
  for(Actor* const actor : GameTime::actors_)
  {
    if(actor->getData().id == ActorId::mummy)
    {
      if(static_cast<Mon*>(actor)->leader == Map::player)
      {
        actor->restoreHp(999);
        return SpellEffectNoticed::yes;
      }
    }
  }

  //This point reached means no mummy controlled, summon a new one
  ActorFactory::summonMon(caster->pos, {ActorId::mummy}, false, caster);
  return SpellEffectNoticed::yes;
}

//------------------------------------------------------------ DETECT ITEMS
SpellEffectNoticed SpellDetItems::cast_(Actor* const caster) const
{
  (void)caster;

  const int RADI    = FOV_STD_RADI_INT + 3;
  const int ORIG_X  = Map::player->pos.x;
  const int ORIG_Y  = Map::player->pos.y;
  const int X0      = max(0, ORIG_X - RADI);
  const int Y0      = max(0, ORIG_Y - RADI);
  const int X1      = min(MAP_W - 1, ORIG_X + RADI);
  const int Y1      = min(MAP_H - 1, ORIG_Y + RADI);

  vector<Pos> itemsRevealedCells;

  for(int y = Y0; y < Y1; ++y)
  {
    for(int x = X0; x <= X1; ++x)
    {
      Item* item = Map::cells[x][y].item;
      if(item)
      {
        Map::cells[x][y].isSeenByPlayer = true;
        Map::cells[x][y].isExplored = true;
        itemsRevealedCells.push_back(Pos(x, y));
      }
    }
  }
  if(!itemsRevealedCells.empty())
  {
    Render::drawMapAndInterface();
    Map::player->updateFov();
    Render::drawBlastAtCells(itemsRevealedCells, clrWhite);
    Render::drawMapAndInterface();

    if(itemsRevealedCells.size() == 1)
    {
      Log::addMsg("An item is revealed to me.");
    }
    if(itemsRevealedCells.size() > 1)
    {
      Log::addMsg("Some items are revealed to me.");
    }
    return SpellEffectNoticed::yes;
  }
  return SpellEffectNoticed::no;
}

//------------------------------------------------------------ DETECT TRAPS
SpellEffectNoticed SpellDetTraps::cast_(Actor* const caster) const
{
  (void)caster;

  vector<Pos> trapsRevealedCells;

  for(int x = 0; x < MAP_W; ++x)
  {
    for(int y = 0; y < MAP_H; ++y)
    {
      if(Map::cells[x][y].isSeenByPlayer)
      {
        auto* const f = Map::cells[x][y].rigid;
        if(f->getId() == FeatureId::trap)
        {
          auto* const trap = static_cast<Trap*>(f);
          trap->reveal(false);
          trapsRevealedCells.push_back(Pos(x, y));
        }
      }
    }
  }

  if(!trapsRevealedCells.empty())
  {
    Render::drawMapAndInterface();
    Map::player->updateFov();
    Render::drawBlastAtCells(trapsRevealedCells, clrWhite);
    Render::drawMapAndInterface();
    if(trapsRevealedCells.size() == 1)
    {
      Log::addMsg("A hidden trap is revealed to me.");
    }
    if(trapsRevealedCells.size() > 1)
    {
      Log::addMsg("Some hidden traps are revealed to me.");
    }
    return SpellEffectNoticed::yes;
  }
  return SpellEffectNoticed::no;
}

//------------------------------------------------------------ DETECT MONSTERS
SpellEffectNoticed SpellDetMon::cast_(Actor* const caster) const
{
  (void)caster;

  bool                isSeer      = PlayerBon::hasTrait(Trait::seer);
  const int           MULTIPLIER  = 6 * (isSeer ? 3 : 1);
  const int           MAX_DIST    = FOV_STD_RADI_INT * 2;
  const Pos           playerPos   = Map::player->pos;
  SpellEffectNoticed  isNoticed   = SpellEffectNoticed::no;

  for(Actor* actor : GameTime::actors_)
  {
    if(actor != Map::player)
    {
      if(Utils::kingDist(playerPos, actor->pos) <= MAX_DIST)
      {
        static_cast<Mon*>(actor)->playerBecomeAwareOfMe(MULTIPLIER);
        isNoticed = SpellEffectNoticed::yes;
      }
    }
  }

  if(isNoticed == SpellEffectNoticed::yes) {Log::addMsg("I detect monsters.");}

  return isNoticed;
}

//------------------------------------------------------------ OPENING
SpellEffectNoticed SpellOpening::cast_(Actor* const caster) const
{

  (void)caster;

  vector<Pos> featuresOpenedCells;

  for(int y = 1; y < MAP_H - 1; ++y)
  {
    for(int x = 1; x < MAP_W - 1; ++x)
    {
      if(Map::cells[x][y].isSeenByPlayer)
      {
        if(Map::cells[x][y].rigid->open())
        {
          featuresOpenedCells.push_back(Pos(x, y));
        }
      }
    }
  }

  if(featuresOpenedCells.empty())
  {
    return SpellEffectNoticed::no;
  }
  else
  {
    Render::drawMapAndInterface();
    Map::player->updateFov();
    Render::drawBlastAtCells(featuresOpenedCells, clrWhite);
    Render::drawMapAndInterface();
    return SpellEffectNoticed::yes;
  }
}

//------------------------------------------------------------ SACRIFICE LIFE
SpellEffectNoticed SpellSacrLife::cast_(Actor* const caster) const
{
  (void)caster;

  //Convert every 2 HP to 1 SPI

  const int PLAYER_HP_CUR = Map::player->getHp();

  if(PLAYER_HP_CUR > 2)
  {
    const int HP_DRAINED = ((PLAYER_HP_CUR - 1) / 2) * 2;
    Map::player->hit(HP_DRAINED, DmgType::pure);
    Map::player->restoreSpi(HP_DRAINED, true, true);
    return SpellEffectNoticed::yes;
  }
  return SpellEffectNoticed::no;
}

//------------------------------------------------------------ SACRIFICE SPIRIT
SpellEffectNoticed SpellSacrSpi::cast_(Actor* const caster) const
{
  (void)caster;

  //Convert every SPI to HP

  const int PLAYER_SPI_CUR = Map::player->getSpi();

  if(PLAYER_SPI_CUR > 0)
  {
    const int HP_DRAINED = PLAYER_SPI_CUR - 1;
    Map::player->hitSpi(HP_DRAINED, true);
    Map::player->restoreHp(HP_DRAINED, true, true);
    return SpellEffectNoticed::yes;
  }
  return SpellEffectNoticed::no;
}

//------------------------------------------------------------ ROGUE HIDE
SpellEffectNoticed SpellCloudMinds::cast_(Actor* const caster) const
{

  (void)caster;
  Log::addMsg("I vanish from the minds of my enemies.");

  for(Actor* actor : GameTime::actors_)
  {
    if(actor != Map::player)
    {
      Mon* const mon = static_cast<Mon*>(actor);
      mon->awareCounter_ = 0;
    }
  }
  return SpellEffectNoticed::yes;
}

//------------------------------------------------------------ BLESS
SpellEffectNoticed SpellBless::cast_(Actor* const caster) const
{
  caster->getPropHandler().tryApplyProp(new PropBlessed(PropTurns::std));

  return SpellEffectNoticed::yes;
}

bool SpellBless::allowMonCastNow(Mon& mon,
                                 const bool losBlockers[MAP_W][MAP_H]) const
{
  (void)losBlockers;
  vector<PropId> props;
  mon.getPropHandler().getAllActivePropIds(props);
  return mon.target &&
         find(begin(props), end(props), propBlessed) == end(props);
}

//------------------------------------------------------------ TELEPORT
SpellEffectNoticed SpellTeleport::cast_(Actor* const caster) const
{

  if(caster != Map::player)
  {
    if(Map::player->isSeeingActor(*caster, nullptr))
    {
      Log::addMsg(caster->getNameThe() + " disappears in a blast of smoke!");
    }
  }

  caster->teleport(false);
  return SpellEffectNoticed::yes;
}

bool SpellTeleport::allowMonCastNow(Mon& mon,
                                    const bool losBlockers[MAP_W][MAP_H]) const
{
  return mon.target                                   &&
         mon.isSeeingActor(*mon.target, losBlockers)  &&
         mon.getHp() <= (mon.getHpMax(true) / 2)      &&
         Rnd::coinToss();
}

//------------------------------------------------------------ ELEMENTAL RES
SpellEffectNoticed SpellElemRes::cast_(Actor* const caster) const
{
  const int DURATION = 20;
  PropHandler& propHlr = caster->getPropHandler();
  propHlr.tryApplyProp(new PropRFire(PropTurns::specific, DURATION));
  propHlr.tryApplyProp(new PropRElec(PropTurns::specific, DURATION));
  propHlr.tryApplyProp(new PropRCold(PropTurns::specific, DURATION));
  return SpellEffectNoticed::yes;
}

bool SpellElemRes::allowMonCastNow(Mon& mon,
                                   const bool losBlockers[MAP_W][MAP_H]) const
{
  return mon.target                                   &&
         mon.isSeeingActor(*mon.target, losBlockers)  &&
         Rnd::oneIn(3);
}

//------------------------------------------------------------ KNOCKBACK
SpellEffectNoticed SpellKnockBack::cast_(Actor* const caster) const
{
  Clr     msgClr            = clrMsgGood;
  bool    isPlayerSeeingTgt = true;
  string  tgtRefStr         = "me";

  if(caster->isPlayer())
  {
    //TODO Allow for player
  }
  else //Caster is not player
  {
    Mon*    casterMon = static_cast<Mon*>(caster);
    Actor*  tgt       = casterMon->target;
    assert(tgt);

    tgtRefStr         = tgt->getNameThe();

    if(Map::player->isLeaderOf(*tgt))
    {
      msgClr = clrMsgBad;
    }

    isPlayerSeeingTgt = Map::player->isSeeingActor(*tgt, nullptr);
  }

  if(isPlayerSeeingTgt) {Log::addMsg("A force pushes " + tgtRefStr + "!", msgClr);}

  KnockBack::tryKnockBack(*(Map::player), caster->pos, false);

  return SpellEffectNoticed::no;
}

bool SpellKnockBack::allowMonCastNow(Mon& mon,
                                     const bool losBlockers[MAP_W][MAP_H]) const
{
  return mon.target && mon.isSeeingActor(*mon.target, losBlockers);
}

//------------------------------------------------------------ PROP ON OTHERS
SpellEffectNoticed SpellPropOnMon::cast_(Actor* const caster) const
{

  const PropId propId = getPropId();

  if(caster->isPlayer())
  {
    vector<Actor*> targets;
    Map::player->getSeenFoes(targets);

    if(targets.empty())
    {
      return SpellEffectNoticed::no;
    }
    else
    {
      vector<Pos> actorCells;
      actorCells.clear();

      for(Actor* a : targets) {actorCells.push_back(a->pos);}

      Render::drawBlastAtCellsWithVision(actorCells, clrMagenta);

      for(Actor* actor : targets)
      {
        PropHandler& propHlr = actor->getPropHandler();
        Prop* const prop = propHlr.mkProp(propId, PropTurns::std);
        propHlr.tryApplyProp(prop);
      }
      return SpellEffectNoticed::yes;
    }
  }
  else
  {
    Render::drawBlastAtCellsWithVision(vector<Pos>(1, Map::player->pos), clrMagenta);

    PropHandler& propHandler = Map::player->getPropHandler();
    Prop* const prop = propHandler.mkProp(propId, PropTurns::std);
    propHandler.tryApplyProp(prop);

    return SpellEffectNoticed::no;
  }
}

bool SpellPropOnMon::allowMonCastNow(Mon& mon,
                                     const bool losBlockers[MAP_W][MAP_H]) const
{
  return mon.target && mon.isSeeingActor(*mon.target, losBlockers);
}

//------------------------------------------------------------ DISEASE
SpellEffectNoticed SpellDisease::cast_(Actor* const caster) const
{
  if(caster->isPlayer())
  {
    return SpellEffectNoticed::yes;
  }
  else
  {
    auto* const tgt = static_cast<Mon*>(caster)->target;
    string actorName       = "me";
    bool isPlayerSeeingTgt = true;
    if(tgt != Map::player)
    {
      if(Map::player->isSeeingActor(*tgt, nullptr))
      {
        isPlayerSeeingTgt = true;
        actorName         = tgt->getNameThe();
      }
      else
      {
        isPlayerSeeingTgt = false;
      }
    }
    if(isPlayerSeeingTgt)
    {
      Log::addMsg("A disease is starting to afflict " + actorName + "!", clrMsgBad);
    }
    tgt->getPropHandler().tryApplyProp(new PropDiseased(PropTurns::specific, 50));
    return SpellEffectNoticed::no;
  }
}

bool SpellDisease::allowMonCastNow(Mon& mon,
                                   const bool losBlockers[MAP_W][MAP_H]) const
{
  return mon.target                                   &&
         mon.isSeeingActor(*mon.target, losBlockers)  &&
         Rnd::coinToss();
}

//------------------------------------------------------------ SUMMON RANDOM
SpellEffectNoticed SpellSummonRandom::cast_(Actor* const caster) const
{
  Pos summonPos(caster->pos);

  vector<Pos> freeCellsSeenByPlayer;
  const int RADI = FOV_STD_RADI_INT;
  const Pos playerPos(Map::player->pos);
  const int X0 = max(0, playerPos.x - RADI);
  const int Y0 = max(0, playerPos.y - RADI);
  const int X1 = min(MAP_W, playerPos.x + RADI) - 1;
  const int Y1 = min(MAP_H, playerPos.y + RADI) - 1;
  for(int x = X0; x <= X1; ++x)
  {
    for(int y = Y0; y <= Y1; ++y)
    {
      if(Map::cells[x][y].isSeenByPlayer)
      {
        freeCellsSeenByPlayer.push_back(Pos(x, y));
      }
    }
  }

  bool blocked[MAP_W][MAP_H];
  MapParse::parse(CellPred::BlocksMoveCmn(true), blocked);

  for(int i = 0; i < int(freeCellsSeenByPlayer.size()); ++i)
  {
    const Pos pos(freeCellsSeenByPlayer.at(i));
    if(blocked[pos.x][pos.y])
    {
      freeCellsSeenByPlayer.erase(freeCellsSeenByPlayer.begin() + i);
      i--;
    }
  }

  if(freeCellsSeenByPlayer.empty())
  {
    vector<Pos> freeCellsVector;
    Utils::mkVectorFromBoolMap(false, blocked, freeCellsVector);
    if(!freeCellsVector.empty())
    {
      sort(freeCellsVector.begin(), freeCellsVector.end(), IsCloserToPos(caster->pos));
      summonPos = freeCellsVector.at(0);
    }
  }
  else
  {
    const int ELEMENT = Rnd::range(0, freeCellsSeenByPlayer.size() - 1);
    summonPos = freeCellsSeenByPlayer.at(ELEMENT);
  }

  vector<ActorId> summonBucket;
  for(int i = 0; i < int(ActorId::END); ++i)
  {
    const ActorDataT& data = ActorData::data[i];
    if(data.canBeSummoned)
    {
      if(data.spawnMinDLVL <= caster->getData().spawnMinDLVL)
      {
        summonBucket.push_back(ActorId(i));
      }
    }
  }
  const int ELEMENT   = Rnd::range(1, summonBucket.size() - 1);
  const ActorId id    = summonBucket.at(ELEMENT);
  Actor* const actor  = ActorFactory::mk(id, summonPos);
  Mon* mon        = static_cast<Mon*>(actor);
  mon->awareCounter_  = mon->getData().nrTurnsAwarePlayer;
  if(caster->isPlayer())
  {
    //Caster is player, set player as leader
    mon->leader = caster;
  }
  else
  {
    //Caster is a monster, set monster or monster's leader as leader
    Actor* const casterLeader = static_cast<Mon*>(caster)->leader;
    mon->leader = casterLeader ? casterLeader : caster;
  }
  if(Map::cells[summonPos.x][summonPos.y].isSeenByPlayer)
  {
    Log::addMsg(mon->getNameA() + " appears.");
  }
  return SpellEffectNoticed::no;
}

bool SpellSummonRandom::allowMonCastNow(Mon& mon,
                                        const bool losBlockers[MAP_W][MAP_H]) const
{
  return mon.target && (mon.isSeeingActor(*mon.target, losBlockers) || Rnd::oneIn(20));
}

//------------------------------------------------------------ HEAL SELF
SpellEffectNoticed SpellHealSelf::cast_(Actor* const caster) const
{
  //The spell effect is noticed if any hit points were restored
  return caster->restoreHp(999, true) ?
         SpellEffectNoticed::yes :
         SpellEffectNoticed::no;
}

bool SpellHealSelf::allowMonCastNow(Mon& mon,
                                    const bool losBlockers[MAP_W][MAP_H]) const
{
  (void)losBlockers;
  return mon.getHp() < mon.getHpMax(true);
}

//------------------------------------------------------------ MI-GO HYPNOSIS
SpellEffectNoticed SpellMiGoHypnosis::cast_(Actor* const caster) const
{
  (void)caster;
  Log::addMsg("There is a sharp droning in my head!");

  if(Rnd::coinToss())
  {
    Map::player->getPropHandler().tryApplyProp(
      new PropFainted(PropTurns::specific, Rnd::range(2, 10)));
  }
  else
  {
    Log::addMsg("I feel dizzy.");
  }

  return SpellEffectNoticed::yes;
}

bool SpellMiGoHypnosis::allowMonCastNow(Mon& mon,
                                        const bool losBlockers[MAP_W][MAP_H]) const
{
  return mon.target                                     &&
         mon.isSeeingActor(*(Map::player), losBlockers) &&
         Rnd::oneIn(4);
}

//------------------------------------------------------------ IMMOLATION
SpellEffectNoticed SpellImmolation::cast_(Actor* const caster) const
{
  (void)caster;

  Log::addMsg("Flames are rising around me!");

  Map::player->getPropHandler().tryApplyProp(
    new PropBurning(PropTurns::specific, Rnd::range(3, 4)));

  return SpellEffectNoticed::yes;
}

bool SpellImmolation::allowMonCastNow(Mon& mon,
                                      const bool losBlockers[MAP_W][MAP_H]) const
{
  return mon.target                                   &&
         mon.isSeeingActor(*mon.target, losBlockers)  &&
         Rnd::oneIn(4);
}
