#include "Spells.h"

#include <algorithm>
#include <vector>
#include <assert.h>

#include "Init.h"
#include "Render.h"
#include "ActorMon.h"
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
#include "DungeonMaster.h"

using namespace std;

namespace
{

const int SUMMON_HOSTILE_ONE_IN_N = 7;

} //namespace

namespace SpellHandling
{

Spell* getRandomSpellForMon()
{
  vector<SpellId> bucket;
  for (int i = 0; i < int(SpellId::END); ++i)
  {
    Spell* const spell = mkSpellFromId(SpellId(i));
    if (spell->isAvailForAllMon()) {bucket.push_back(SpellId(i));}
    delete spell;
  }
  const int ELEMENT = Rnd::range(0, bucket.size() - 1);
  return mkSpellFromId(bucket[ELEMENT]);
}

Spell* mkSpellFromId(const SpellId spellId)
{
  switch (spellId)
  {
    case SpellId::slowMon:              return new SpellSlowMon;
    case SpellId::terrifyMon:           return new SpellTerrifyMon;
    case SpellId::paralyzeMon:          return new SpellParalyzeMon;
    case SpellId::disease:              return new SpellDisease;
    case SpellId::darkbolt:             return new SpellDarkbolt;
    case SpellId::azaWrath:             return new SpellAzaWrath;
    case SpellId::summonMon:            return new SpellSummonMon;
    case SpellId::healSelf:             return new SpellHealSelf;
    case SpellId::knockBack:            return new SpellKnockBack;
    case SpellId::teleport:             return new SpellTeleport;
    case SpellId::mayhem:               return new SpellMayhem;
    case SpellId::pest:                 return new SpellPest;
    case SpellId::detItems:             return new SpellDetItems;
    case SpellId::detTraps:             return new SpellDetTraps;
    case SpellId::detMon:               return new SpellDetMon;
    case SpellId::opening:              return new SpellOpening;
    case SpellId::sacrLife:             return new SpellSacrLife;
    case SpellId::sacrSpi:              return new SpellSacrSpi;
    case SpellId::cloudMinds:           return new SpellCloudMinds;
    case SpellId::bless:                return new SpellBless;
    case SpellId::miGoHypno:            return new SpellMiGoHypno;
    case SpellId::burn:                 return new SpellBurn;
    case SpellId::elemRes:              return new SpellElemRes;
    case SpellId::pharaohStaff:         return new SpellPharaohStaff;
    case SpellId::light:                return new SpellLight;
    case SpellId::END: {}
      break;
  }
  assert(false && "No spell found for ID");
  return nullptr;
}

} //SpellHandling

Range Spell::getSpiCost(const bool IS_BASE_COST_ONLY, Actor* const caster) const
{
  int costMax = getMaxSpiCost_();

  if (caster == Map::player && !IS_BASE_COST_ONLY)
  {
    const int X0 = max(0, caster->pos.x - 1);
    const int Y0 = max(0, caster->pos.y - 1);
    const int X1 = min(MAP_W - 1, caster->pos.x + 1);
    const int Y1 = min(MAP_H - 1, caster->pos.y + 1);

    for (int x = X0; x <= X1; ++x)
    {
      for (int y = Y0; y <= Y1; ++y)
      {
        if (Map::cells[x][y].rigid->getId() == FeatureId::altar)
        {
          costMax -= 1;
        }
      }
    }

    bool IS_WARLOCK     = PlayerBon::traitsPicked[int(Trait::warlock)];
    bool IS_BLOOD_SORC  = PlayerBon::traitsPicked[int(Trait::bloodSorcerer)];
    bool IS_SEER        = PlayerBon::traitsPicked[int(Trait::seer)];
    bool IS_SUMMONER    = PlayerBon::traitsPicked[int(Trait::summoner)];

    if (IS_BLOOD_SORC) {costMax--;}

    switch (getId())
    {
      case SpellId::darkbolt:       if (IS_WARLOCK)  --costMax;     break;
      case SpellId::azaWrath:       if (IS_WARLOCK)  --costMax;     break;
      case SpellId::mayhem:         if (IS_WARLOCK)  --costMax;     break;
      case SpellId::detMon:         if (IS_SEER)     --costMax;     break;
      case SpellId::detItems:       if (IS_SEER)     costMax -= 3;  break;
      case SpellId::detTraps:       if (IS_SEER)     costMax -= 3;  break;
      case SpellId::summonMon:      if (IS_SUMMONER) --costMax;     break;
      case SpellId::pest:           if (IS_SUMMONER) --costMax;     break;
      case SpellId::pharaohStaff:   if (IS_SUMMONER) --costMax;     break;
      default: {} break;
    }

    PropHandler& propHlr = caster->getPropHandler();

    bool props[int(PropId::END)];
    propHlr.getPropIds(props);

    if (!propHlr.allowSee()) {--costMax;}
    if (props[int(PropId::blessed)])  {--costMax;}
    if (props[int(PropId::cursed)])   {costMax += 3;}
  }

  costMax             = max(1, costMax);
  const int COST_MIN  = max(1, costMax / 2);

  return Range(COST_MIN, costMax);
}

SpellEffectNoticed Spell::cast(Actor* const caster, const bool IS_INTRINSIC) const
{
  TRACE_FUNC_BEGIN;
  if (caster->getPropHandler().allowCastSpells(true))
  {
    if (caster->isPlayer())
    {
      TRACE << "Player casting spell" << endl;
      const ShockSrc shockSrc = IS_INTRINSIC ?
                                ShockSrc::castIntrSpell :
                                ShockSrc::useStrangeItem;
      const int SHOCK_VALUE = IS_INTRINSIC ? getShockLvlIntrCast() : 10;
      Map::player->incrShock(SHOCK_VALUE, shockSrc);
      if (Map::player->isAlive())
      {
        Audio::play(SfxId::spellGeneric);
      }
    }
    else //Caster is not player
    {
      TRACE << "Monster casting spell" << endl;
      Mon* const mon = static_cast<Mon*>(caster);
      if (Map::cells[mon->pos.x][mon->pos.y].isSeenByPlayer)
      {
        const string spellStr = mon->getData().spellCastMsg;
        if (!spellStr.empty())
        {
          Log::addMsg(spellStr);
        }
      }
      mon->spellCoolDownCur_ = mon->getData().spellCooldownTurns;
    }

    if (IS_INTRINSIC)
    {
      const Range cost = getSpiCost(false, caster);
      caster->hitSpi(Rnd::range(cost), false);
    }

    SpellEffectNoticed isNoticed = SpellEffectNoticed::no;

    if (caster->isAlive())
    {
      isNoticed = cast_(caster);
    }

    GameTime::tick();
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

  vector<Actor*> seenActors;
  caster->getSeenFoes(seenActors);
  if (seenActors.empty())
  {
    return SpellEffectNoticed::no;
  }

  tgt = Utils::getRandomClosestActor(caster->pos, seenActors);

  vector<Pos> line;
  LineCalc::calcNewLine(caster->pos, tgt->pos, true, 999, false, line);
  Render::drawMapAndInterface();
  const size_t LINE_SIZE = line.size();
  for (size_t i = 1; i < LINE_SIZE; ++i)
  {
    const Pos& p = line[i];
    if (Config::isTilesMode())
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

  bool    isWarlockCharged  = false;
  Clr     msgClr            = clrMsgGood;
  string  tgtStr            = "I am";

  if (tgt->isPlayer())
  {
    msgClr = clrMsgBad;
  }
  else //Target is monster
  {
    tgtStr = tgt->getNameThe() + " is";

    if (Map::player->isLeaderOf(tgt)) {msgClr = clrWhite;}
  }

  if (caster->isPlayer())
  {
    bool props[int(PropId::END)];
    Map::player->getPropHandler().getPropIds(props);
    isWarlockCharged = props[int(PropId::warlockCharged)];
  }

  if (Map::player->isSeeingActor(*tgt, nullptr))
  {
    Log::addMsg(tgtStr + " struck by a blast!", msgClr);
  }

  tgt->getPropHandler().tryApplyProp(new PropParalyzed(PropTurns::specific, 2));

  Range dmgRange(4, 10);
  const int DMG = isWarlockCharged ? dmgRange.upper : Rnd::range(dmgRange);

  tgt->hit(DMG, DmgType::physical);

  SndEmit::emitSnd({"", SfxId::END, IgnoreMsgIfOriginSeen::yes, tgt->pos, nullptr,
                    SndVol::low, AlertsMon::yes
                   });

  return SpellEffectNoticed::yes;
}

bool SpellDarkbolt::allowMonCastNow(Mon& mon) const
{
  return mon.tgt_ && Rnd::oneIn(2);
}

//------------------------------------------------------------ AZATHOTHS WRATH
SpellEffectNoticed SpellAzaWrath::cast_(Actor* const caster) const
{
  Range dmgRange(4, 8);
  bool  isWarlockCharged = false;

  vector<Actor*>  tgts;
  caster->getSeenFoes(tgts);

  if (tgts.empty())
  {
    return SpellEffectNoticed::no;
  }

  //This point reached means targets are available
  if (caster->isPlayer())
  {
    bool props[int(PropId::END)];
    Map::player->getPropHandler().getPropIds(props);

    isWarlockCharged = props[int(PropId::warlockCharged)];
  }

  Render::drawBlastAtSeenActors(tgts, clrRedLgt);

  for (Actor* tgt : tgts)
  {
    string  tgtStr  = "I am";
    Clr     msgClr  = clrMsgGood;
    if (tgt->isPlayer())
    {
      msgClr = clrMsgBad;
    }
    else //Target is monster
    {
      tgtStr = tgt->getNameThe();
      if (Map::player->isLeaderOf(tgt)) {msgClr = clrWhite;}
    }

    if (Map::player->isSeeingActor(*tgt, nullptr))
    {
      Log::addMsg(tgtStr + " is struck by a roaring blast!", msgClr);
    }

    tgt->getPropHandler().tryApplyProp(new PropParalyzed(PropTurns::specific, 2));

    const int DMG = isWarlockCharged ? dmgRange.upper : Rnd::range(dmgRange);

    tgt->hit(DMG, DmgType::physical);

    SndEmit::emitSnd({"", SfxId::END, IgnoreMsgIfOriginSeen::yes, tgt->pos, nullptr,
                      SndVol::high, AlertsMon::yes
                     });
  }

  return SpellEffectNoticed::yes;
}

bool SpellAzaWrath::allowMonCastNow(Mon& mon) const
{
  return mon.tgt_;
}

//------------------------------------------------------------ MAYHEM
SpellEffectNoticed SpellMayhem::cast_(Actor* const caster) const
{
  const bool IS_PLAYER = caster->isPlayer();

  if ( Map::player->isSeeingActor(*caster, nullptr))
  {
    string casterName = IS_PLAYER ? "me" : caster->getNameThe();
    Log::addMsg("Destruction rages around " + casterName + "!");
  }


  const Pos& casterPos = caster->pos;

  const int NR_SWEEPS = 5;
  const int RADI      = FOV_STD_RADI_INT;

  const int X0 = max(1, casterPos.x - RADI);
  const int Y0 = max(1, casterPos.y - RADI);
  const int X1 = min(MAP_W - 1, casterPos.x + RADI) - 1;
  const int Y1 = min(MAP_H - 1, casterPos.y + RADI) - 1;

  for (int i = 0; i < NR_SWEEPS; ++i)
  {
    for (int y = Y0; y <= Y1; ++y)
    {
      for (int x = X0; x <= X1; ++x)
      {
        bool isAdjToWalkableCell = false;
        for (int dx = -1; dx <= 1; ++dx)
        {
          for (int dy = -1; dy <= 1; ++dy)
          {
            const Rigid* const f = Map::cells[x + dx][y + dy].rigid;
            if (f->canMoveCmn()) {isAdjToWalkableCell = true;}
          }
        }
        if (isAdjToWalkableCell && Rnd::oneIn(8))
        {
          Map::cells[x][y].rigid->hit(DmgType::physical, DmgMethod::explosion);
        }
      }
    }
  }

  for (int y = Y0; y <= Y1; ++y)
  {
    for (int x = X0; x <= X1; ++x)
    {
      auto* const f = Map::cells[x][y].rigid;

      if (f->canHaveBlood() && Rnd::oneIn(10))
      {
        f->mkBloody();
      }

      if (Rnd::oneIn(4))
      {
        f->hit(DmgType::fire, DmgMethod::elemental, nullptr);
      }
    }
  }

  vector<Actor*> seenFoes;
  caster->getSeenFoes(seenFoes);

  for (auto* actor : seenFoes)
  {
    actor->getPropHandler().tryApplyProp(new PropBurning(PropTurns::std));
  }

  SndEmit::emitSnd({"", SfxId::END, IgnoreMsgIfOriginSeen::yes, casterPos, nullptr,
                    SndVol::high, AlertsMon::yes
                   });

  return SpellEffectNoticed::yes;
}

bool SpellMayhem::allowMonCastNow(Mon& mon) const
{
  return mon.awareCounter_ > 0  &&
         Rnd::coinToss()        &&
         (mon.tgt_ || Rnd::oneIn(20));
}

//------------------------------------------------------------ PESTILENCE
SpellEffectNoticed SpellPest::cast_(Actor* const caster) const
{
  const int RND = Rnd::range(1, 3);
  const ActorId monsterId = RND == 1 ? ActorId::greenSpider :
                            RND == 2 ? ActorId::redSpider   :
                            ActorId::rat;

  const size_t NR_MON = Rnd::range(7, 10);

  Actor*  leader                  = nullptr;
  bool    didPlayerSummonHostile  = false;

  if (caster->isPlayer())
  {
    const int N             = SUMMON_HOSTILE_ONE_IN_N *
                              (PlayerBon::traitsPicked[int(Trait::summoner)] ? 2 : 1);
    didPlayerSummonHostile  = Rnd::oneIn(N);
    leader                  = didPlayerSummonHostile ? nullptr : caster;
  }
  else //Caster is monster
  {
    Actor* const casterLeader = static_cast<Mon*>(caster)->leader_;
    leader                    = casterLeader ? casterLeader : caster;
  }

  vector<Mon*> monSummoned;

  ActorFactory::summonMon(caster->pos, {NR_MON, monsterId}, true, leader, &monSummoned);

  bool isAnySeenByPlayer = false;

  const int NR_TURNS_SUMMONED = Rnd::range(40, 70);

  for (Mon* const mon : monSummoned)
  {
    mon->nrTurnsUntilUnsummoned_ = NR_TURNS_SUMMONED;
    if (Map::player->isSeeingActor(*mon, nullptr))
    {
      isAnySeenByPlayer = true;
    }
  }

  if (isAnySeenByPlayer)
  {
    string casterStr = "me";

    if (!caster->isPlayer())
    {
      casterStr = caster->getNameThe();
    }

    Log::addMsg("Disgusting critters appear around " + casterStr + "!");

    if (didPlayerSummonHostile)
    {
      Log::addMsg("They are hostile!", clrMsgNote, true, true);
    }

    return SpellEffectNoticed::yes;
  }

  return SpellEffectNoticed::no;
}

bool SpellPest::allowMonCastNow(Mon& mon) const
{
  return mon.tgt_         &&
         Rnd::coinToss()  &&
         (mon.tgt_ || Rnd::oneIn(20));
}

//------------------------------------------------------------ PHARAOH STAFF
SpellEffectNoticed SpellPharaohStaff::cast_(Actor* const caster) const
{
  //First check for a friendly mummy and heal it (as per the spell description)
  for (Actor* const actor : GameTime::actors_)
  {
    if (actor->getData().id == ActorId::mummy)
    {
      if (caster->isLeaderOf(actor))
      {
        actor->restoreHp(999);
        return SpellEffectNoticed::yes;
      }
    }
  }

  //This point reached means no mummy controlled, summon a new one
  Actor*  leader                  = nullptr;
  bool    didPlayerSummonHostile  = false;

  if (caster->isPlayer())
  {
    const int N             = SUMMON_HOSTILE_ONE_IN_N *
                              (PlayerBon::traitsPicked[int(Trait::summoner)] ? 2 : 1);
    didPlayerSummonHostile  = Rnd::oneIn(N);
    leader                  = didPlayerSummonHostile ? nullptr : caster;
  }
  else //Caster is monster
  {
    Actor* const casterLeader = static_cast<Mon*>(caster)->leader_;
    leader                    = casterLeader ? casterLeader : caster;
  }

  vector<Mon*> summonedMon;

  ActorFactory::summonMon(caster->pos, {ActorId::mummy}, false, leader, &summonedMon);

  const Mon* const mon = summonedMon[0];

  if (Map::player->isSeeingActor(*mon, nullptr))
  {
    Log::addMsg(mon->getNameA() + " appears!");

    if (didPlayerSummonHostile)
    {
      Log::addMsg("It is hostile!", clrMsgNote, true, true);
    }

    return SpellEffectNoticed::yes;
  }

  return SpellEffectNoticed::no;
}

bool SpellPharaohStaff::allowMonCastNow(Mon& mon) const
{
  return mon.tgt_ && Rnd::oneIn(4);
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

  for (int y = Y0; y < Y1; ++y)
  {
    for (int x = X0; x <= X1; ++x)
    {
      Item* item = Map::cells[x][y].item;
      if (item)
      {
        Map::cells[x][y].isSeenByPlayer = true;
        Map::cells[x][y].isExplored     = true;
        itemsRevealedCells.push_back(Pos(x, y));
      }
    }
  }
  if (!itemsRevealedCells.empty())
  {
    Render::drawMapAndInterface();
    Map::player->updateFov();
    Render::drawBlastAtCells(itemsRevealedCells, clrWhite);
    Render::drawMapAndInterface();

    if (itemsRevealedCells.size() == 1)
    {
      Log::addMsg("An item is revealed to me.");
    }
    if (itemsRevealedCells.size() > 1)
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

  for (int x = 0; x < MAP_W; ++x)
  {
    for (int y = 0; y < MAP_H; ++y)
    {
      if (Map::cells[x][y].isSeenByPlayer)
      {
        auto* const f = Map::cells[x][y].rigid;
        if (f->getId() == FeatureId::trap)
        {
          auto* const trap = static_cast<Trap*>(f);
          trap->reveal(false);
          trapsRevealedCells.push_back(Pos(x, y));
        }
      }
    }
  }

  if (!trapsRevealedCells.empty())
  {
    Render::drawMapAndInterface();
    Map::player->updateFov();
    Render::drawBlastAtCells(trapsRevealedCells, clrWhite);
    Render::drawMapAndInterface();
    if (trapsRevealedCells.size() == 1)
    {
      Log::addMsg("A hidden trap is revealed to me.");
    }
    if (trapsRevealedCells.size() > 1)
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

  bool                isSeer      = PlayerBon::traitsPicked[int(Trait::seer)];
  const int           MULTIPLIER  = 6 * (isSeer ? 3 : 1);
  const int           MAX_DIST    = FOV_STD_RADI_INT * 2;
  const Pos           playerPos   = Map::player->pos;
  SpellEffectNoticed  isNoticed   = SpellEffectNoticed::no;

  for (Actor* actor : GameTime::actors_)
  {
    if (actor != Map::player)
    {
      if (Utils::kingDist(playerPos, actor->pos) <= MAX_DIST)
      {
        static_cast<Mon*>(actor)->playerBecomeAwareOfMe(MULTIPLIER);
        isNoticed = SpellEffectNoticed::yes;
      }
    }
  }

  if (isNoticed == SpellEffectNoticed::yes) {Log::addMsg("I detect monsters.");}

  return isNoticed;
}

//------------------------------------------------------------ OPENING
SpellEffectNoticed SpellOpening::cast_(Actor* const caster) const
{

  (void)caster;

  bool isAnyOpened = false;

  for (int y = 1; y < MAP_H - 1; ++y)
  {
    for (int x = 1; x < MAP_W - 1; ++x)
    {
      const auto& cell = Map::cells[x][y];

      if (cell.isSeenByPlayer)
      {
        DidOpen didOpen = cell.rigid->open(nullptr);

        if (didOpen == DidOpen::yes)
        {
          isAnyOpened = true;
        }
      }
    }
  }

  if (!isAnyOpened)
  {
    return SpellEffectNoticed::no;
  }

  Map::player->updateFov();
  Render::drawMapAndInterface();
  return SpellEffectNoticed::yes;
}

//------------------------------------------------------------ SACRIFICE LIFE
SpellEffectNoticed SpellSacrLife::cast_(Actor* const caster) const
{
  (void)caster;

  //Convert every 2 HP to 1 SPI

  const int PLAYER_HP_CUR = Map::player->getHp();

  if (PLAYER_HP_CUR > 2)
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

  //Convert all SPI to HP

  const int PLAYER_SPI_CUR = Map::player->getSpi();

  if (PLAYER_SPI_CUR > 0)
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

  for (Actor* actor : GameTime::actors_)
  {
    if (actor != Map::player)
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

//------------------------------------------------------------ LIGHT
SpellEffectNoticed SpellLight::cast_(Actor* const caster) const
{
  caster->getPropHandler().tryApplyProp(new PropRadiant(PropTurns::std));
  return SpellEffectNoticed::yes;
}

//------------------------------------------------------------ TELEPORT
SpellEffectNoticed SpellTeleport::cast_(Actor* const caster) const
{
  caster->teleport();
  return SpellEffectNoticed::yes;
}

bool SpellTeleport::allowMonCastNow(Mon& mon) const
{
  return mon.tgt_                                 &&
         mon.getHp() <= (mon.getHpMax(true) / 2)  &&
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

bool SpellElemRes::allowMonCastNow(Mon& mon) const
{
  return mon.tgt_ && Rnd::oneIn(3);
}

//------------------------------------------------------------ KNOCKBACK
SpellEffectNoticed SpellKnockBack::cast_(Actor* const caster) const
{
  Clr     msgClr    = clrMsgGood;
  string  tgtStr    = "me";
  Mon*    casterMon = static_cast<Mon*>(caster);
  Actor*  tgt       = casterMon->tgt_;
  assert(tgt);

  if (tgt->isPlayer())
  {
    msgClr = clrMsgBad;
  }
  else
  {
    tgtStr = tgt->getNameThe();
    if (Map::player->isLeaderOf(tgt)) {msgClr = clrWhite;}
  }

  if (Map::player->isSeeingActor(*tgt, nullptr))
  {
    Log::addMsg("A force pushes " + tgtStr + "!", msgClr);
  }

  KnockBack::tryKnockBack(*tgt, caster->pos, false);

  return SpellEffectNoticed::no;
}

bool SpellKnockBack::allowMonCastNow(Mon& mon) const
{
  return mon.tgt_;
}

//------------------------------------------------------------ PROP ON OTHERS
SpellEffectNoticed SpellPropOnMon::cast_(Actor* const caster) const
{
  const PropId propId = getPropId();

  vector<Actor*> tgts;
  caster->getSeenFoes(tgts);

  if (tgts.empty())
  {
    return SpellEffectNoticed::no;
  }

  Render::drawBlastAtSeenActors(tgts, clrMagenta);

  for (Actor* const actor : tgts)
  {
    PropHandler&  propHlr = actor->getPropHandler();
    Prop* const   prop    = propHlr.mkProp(propId, PropTurns::std);
    propHlr.tryApplyProp(prop);
  }

  return SpellEffectNoticed::yes;
}

bool SpellPropOnMon::allowMonCastNow(Mon& mon) const
{
  return mon.tgt_;
}

//------------------------------------------------------------ DISEASE
SpellEffectNoticed SpellDisease::cast_(Actor* const caster) const
{
  auto* const tgt   = static_cast<Mon*>(caster)->tgt_;
  string actorName  = "me";

  if (!tgt->isPlayer()) {actorName = tgt->getNameThe();}

  if (Map::player->isSeeingActor(*tgt, nullptr))
  {
    Log::addMsg("A disease is starting to afflict " + actorName + "!");
  }

  tgt->getPropHandler().tryApplyProp(new PropDiseased(PropTurns::specific, 50));
  return SpellEffectNoticed::no;
}

bool SpellDisease::allowMonCastNow(Mon& mon) const
{
  return mon.tgt_ && Rnd::coinToss();
}

//------------------------------------------------------------ SUMMON MONSTER
SpellEffectNoticed SpellSummonMon::cast_(Actor* const caster) const
{
  //Try to summon a creature inside the player's FOV (inside the standard range), in a
  //free visible cell. If no such cell is available, instead summon near the caster.

  bool blocked[MAP_W][MAP_H];
  MapParse::run(CellCheck::BlocksMoveCmn(true), blocked);

  vector<Pos> freeCellsSeenByPlayer;
  const int RADI = FOV_STD_RADI_INT;
  const Pos playerPos(Map::player->pos);
  const int X0 = max(0, playerPos.x - RADI);
  const int Y0 = max(0, playerPos.y - RADI);
  const int X1 = min(MAP_W, playerPos.x + RADI) - 1;
  const int Y1 = min(MAP_H, playerPos.y + RADI) - 1;
  for (int x = X0; x <= X1; ++x)
  {
    for (int y = Y0; y <= Y1; ++y)
    {
      if (!blocked[x][y] && Map::cells[x][y].isSeenByPlayer)
      {
        freeCellsSeenByPlayer.push_back(Pos(x, y));
      }
    }
  }

  Pos summonPos(-1, -1);

  if (freeCellsSeenByPlayer.empty())
  {
    //No free cells seen by player, instead summon near the caster.
    vector<Pos> freeCellsVector;
    Utils::mkVectorFromBoolMap(false, blocked, freeCellsVector);
    if (!freeCellsVector.empty())
    {
      sort(freeCellsVector.begin(), freeCellsVector.end(), IsCloserToPos(caster->pos));
      summonPos = freeCellsVector[0];
    }
  }
  else //There are free cells seen by the player available
  {
    const size_t IDX  = Rnd::range(0, freeCellsSeenByPlayer.size() - 1);
    summonPos         = freeCellsSeenByPlayer[IDX];
  }

  vector<ActorId> summonBucket;
  for (int i = 0; i < int(ActorId::END); ++i)
  {
    const ActorDataT& data = ActorData::data[i];
    if (data.canBeSummoned)
    {
      //Method for finding eligible monsters depends on if player or monster is casting.
      int dlvlMax = -1;

      if (caster->isPlayer())
      {
        //Compare player CVL with monster's allowed spawning DLVL.
        const int PLAYER_CLVL     = DungeonMaster::getCLvl();
        const int PLAYER_CLVL_PCT = (PLAYER_CLVL * 100) / PLAYER_MAX_CLVL;

        dlvlMax                   = (PLAYER_CLVL_PCT * DLVL_LAST) / 100;
      }
      else //Caster is monster
      {
        //Compare caster and summoned monster's allowed spawning DLVL.
        dlvlMax = caster->getData().spawnMinDLVL;
      }

      if (data.spawnMinDLVL <= dlvlMax)
      {
        summonBucket.push_back(ActorId(i));
      }
    }
  }

  if (summonBucket.empty())
  {
    TRACE << "No elligible monsters found for spawning" << endl;
    assert(false);
    return SpellEffectNoticed::no;
  }

  const int       IDX                     = Rnd::range(0, summonBucket.size() - 1);
  const ActorId   monId                   = summonBucket[IDX];
  Actor*          leader                  = nullptr;
  bool            didPlayerSummonHostile  = false;

  if (caster->isPlayer())
  {
    const int N             = SUMMON_HOSTILE_ONE_IN_N *
                              (PlayerBon::traitsPicked[int(Trait::summoner)] ? 2 : 1);
    didPlayerSummonHostile  = Rnd::oneIn(N);
    leader                  = didPlayerSummonHostile ? nullptr : caster;
  }
  else //Caster is monster
  {
    Actor* const casterLeader = static_cast<Mon*>(caster)->leader_;
    leader                    = casterLeader ? casterLeader : caster;
  }

  vector<Mon*> monSummoned;

  ActorFactory::summonMon(summonPos, {monId}, true, leader, &monSummoned);

  Mon* const mon                = monSummoned[0];
  mon->nrTurnsUntilUnsummoned_  = Rnd::range(40, 70);;

  if (Map::player->isSeeingActor(*mon, nullptr))
  {
    Log::addMsg(mon->getNameA() + " appears.");

    if (didPlayerSummonHostile)
    {
      Log::addMsg("It is hostile!", clrMsgNote, true, true);
    }

    return SpellEffectNoticed::yes;
  }

  return SpellEffectNoticed::no;
}

bool SpellSummonMon::allowMonCastNow(Mon& mon) const
{
  return mon.awareCounter_ > 0  &&
         Rnd::coinToss()        &&
         (mon.tgt_ || Rnd::oneIn(20));
}

//------------------------------------------------------------ HEAL SELF
SpellEffectNoticed SpellHealSelf::cast_(Actor* const caster) const
{
  //The spell effect is noticed if any hit points were restored
  const bool IS_ANY_HP_HEALED = caster->restoreHp(999, true);

  return IS_ANY_HP_HEALED ? SpellEffectNoticed::yes : SpellEffectNoticed::no;
}

bool SpellHealSelf::allowMonCastNow(Mon& mon) const
{
  return mon.getHp() < mon.getHpMax(true);
}

//------------------------------------------------------------ MI-GO HYPNOSIS
SpellEffectNoticed SpellMiGoHypno::cast_(Actor* const caster) const
{
  Actor* const tgt = static_cast<Mon*>(caster)->tgt_;
  assert(tgt);

  if (tgt->isPlayer())
  {
    Log::addMsg("There is a sharp droning in my head!");
  }

  if (Rnd::coinToss())
  {
    Prop* const prop = new PropFainted(PropTurns::specific, Rnd::range(2, 10));
    tgt->getPropHandler().tryApplyProp(prop);
  }
  else
  {
    Log::addMsg("I feel dizzy.");
  }

  return SpellEffectNoticed::yes;
}

bool SpellMiGoHypno::allowMonCastNow(Mon& mon) const
{
  return mon.tgt_ && mon.tgt_->isPlayer() && Rnd::oneIn(4);
}

//------------------------------------------------------------ IMMOLATION
SpellEffectNoticed SpellBurn::cast_(Actor* const caster) const
{
  string        tgtStr  = "me";
  Actor* const  tgt     = static_cast<Mon*>(caster)->tgt_;
  assert(tgt);

  if (!tgt->isPlayer()) {tgtStr = tgt->getNameThe();}

  if (Map::player->isSeeingActor(*tgt, nullptr))
  {
    Log::addMsg("Flames are rising around " + tgtStr + "!");
  }

  Prop* const prop = new PropBurning(PropTurns::specific, Rnd::range(3, 4));
  tgt->getPropHandler().tryApplyProp(prop);

  return SpellEffectNoticed::yes;
}

bool SpellBurn::allowMonCastNow(Mon& mon) const
{
  return mon.tgt_ && Rnd::oneIn(4);
}
