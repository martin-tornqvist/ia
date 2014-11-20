#include "Init.h"

#include "Item.h"

#include <assert.h>

#include "Map.h"
#include "GameTime.h"
#include "Utils.h"
#include "MapParsing.h"
#include "Properties.h"
#include "Log.h"
#include "Explosion.h"
#include "Render.h"
#include "Input.h"
#include "Query.h"
#include "ItemFactory.h"
#include "FeatureMob.h"
#include "FeatureRigid.h"
#include "ItemData.h"

using namespace std;

//--------------------------------------------------------- ITEM
Item::Item(ItemDataT* itemData) :
  nrItems_(1),
  carrierProps_(vector<Prop*>()),
  carrierSpells_(vector<Spell*>()),
  meleeDmgPlus_(0),
  data_(itemData)
{

}

Item::~Item()
{
  for (auto prop   : carrierProps_)   {delete prop;}
  for (auto spell  : carrierSpells_)  {delete spell;}
}

const ItemDataT&  Item::getData()   const {return *data_;}
Clr               Item::getClr()    const {return data_->clr;}
char              Item::getGlyph()  const {return data_->glyph;}
TileId            Item::getTile()   const {return data_->tile;}

vector<string> Item::getDescr() const
{
  return data_->baseDescr;
}

int Item::getWeight() const {return data_->itemWeight * nrItems_;}

string Item::getWeightStr() const
{
  const int WEIGHT = getWeight();
  if (WEIGHT <= (itemWeight_extraLight + itemWeight_light) / 2)
  {
    return "very light";
  }
  if (WEIGHT <= (itemWeight_light + itemWeight_medium) / 2)
  {
    return "light";
  }
  if (WEIGHT <= (itemWeight_medium + itemWeight_heavy) / 2)
  {
    return "a bit heavy";
  }
  return "heavy";
}

ConsumeItem Item::activateDefault(Actor* const actor)
{
  (void)actor;
  Log::addMsg("I cannot apply that.");
  return ConsumeItem::no;
}

string Item::getName(const ItemRefType refType, const ItemRefInf inf,
                     const ItemRefAttInf attInf) const
{
  ItemRefType refTypeUsed = refType;
  if (refType == ItemRefType::plural && (!data_->isStackable || nrItems_ == 1))
  {
    refTypeUsed = ItemRefType::a;
  }

  string nrStr = "";
  if (refTypeUsed == ItemRefType::plural)
  {
    nrStr = toStr(nrItems_) + " ";
  }

  string attStr = "";

  ItemRefAttInf attInfUsed = attInf;

  if (attInf == ItemRefAttInf::wpnContext)
  {
    switch (data_->mainAttMode)
    {
      case MainAttMode::melee:  attInfUsed = ItemRefAttInf::melee;  break;
      case MainAttMode::ranged: attInfUsed = ItemRefAttInf::ranged; break;
      case MainAttMode::thrown: attInfUsed = ItemRefAttInf::thrown; break;
      case MainAttMode::none:   attInfUsed = ItemRefAttInf::none;   break;
    }
  }

  if (attInfUsed == ItemRefAttInf::melee)
  {
    const string rollsStr = toStr(data_->melee.dmg.first);
    const string sidesStr = toStr(data_->melee.dmg.second);
    const int PLUS        = meleeDmgPlus_;
    const string plusStr  = PLUS ==  0 ? "" : ((PLUS > 0 ? "+" : "") + toStr(PLUS));
    const int ITEM_SKILL  = data_->melee.hitChanceMod;
    const int PLAYER_MELEE_SKILL = Map::player->getData().abilityVals.getVal(
                                     AbilityId::melee, true, *(Map::player));
    const int SKILL_TOT = max(0, min(100, ITEM_SKILL + PLAYER_MELEE_SKILL));
    const string skillStr = toStr(SKILL_TOT) + "%";
    attStr = " " + rollsStr + "d" + sidesStr + plusStr + " " + skillStr;
  }

  const int PLAYER_RANGED_SKILL =
    Map::player->getData().abilityVals.getVal(AbilityId::ranged, true, *(Map::player));

  if (attInfUsed == ItemRefAttInf::ranged)
  {
    string dmgStr = data_->ranged.dmgInfoOverride;

    if (dmgStr.empty())
    {
      const int MULTIPL     = data_->ranged.isMachineGun ? NR_MG_PROJECTILES : 1;
      const string rollsStr = toStr(data_->ranged.dmg.rolls * MULTIPL);
      const string sidesStr = toStr(data_->ranged.dmg.sides);
      const int PLUS        = data_->ranged.dmg.plus * MULTIPL;
      const string plusStr  = PLUS ==  0 ? "" : ((PLUS > 0 ? "+" : "") + toStr(PLUS));
      dmgStr                = rollsStr + "d" + sidesStr + plusStr;
    }
    const int ITEM_SKILL    = data_->ranged.hitChanceMod;
    const int SKILL_TOT     = max(0, min(100, ITEM_SKILL + PLAYER_RANGED_SKILL));
    const string skillStr   = toStr(SKILL_TOT) + "%";
    attStr = " " + dmgStr + " " + skillStr;
  }

  if (attInfUsed == ItemRefAttInf::thrown)
  {
    const string rollsStr = toStr(data_->ranged.throwDmg.rolls);
    const string sidesStr = toStr(data_->ranged.throwDmg.sides);
    const int PLUS        = data_->ranged.throwDmg.plus;
    const string plusStr  = PLUS ==  0 ? "" : ((PLUS > 0 ? "+" : "") + toStr(PLUS));
    const int ITEM_SKILL  = data_->ranged.throwHitChanceMod;
    const int SKILL_TOT   = max(0, min(100, ITEM_SKILL + PLAYER_RANGED_SKILL));
    const string skillStr = toStr(SKILL_TOT) + "%";
    attStr = " " + rollsStr + "d" + sidesStr + plusStr + " " + skillStr;
  }

  string infStr = "";

  if (inf == ItemRefInf::yes)
  {
    infStr = getNameInf();
    if (!infStr.empty()) {infStr.insert(0, " ");}
  }

  const auto& namesUsed = data_->isIdentified ? data_->baseName : data_->baseNameUnid;

  return nrStr + namesUsed.names[int(refTypeUsed)] + attStr + infStr;
}

void Item::clearCarrierProps()
{
  for (Prop* prop : carrierProps_) {delete prop;}
  carrierProps_.clear();
}

//--------------------------------------------------------- ARMOR
Armor::Armor(ItemDataT* const itemData) :
  Item(itemData), dur_(Rnd::range(80, 100)) {}

string Armor::getArmorDataLine(const bool WITH_BRACKETS) const
{

  const int AP = getAbsorptionPoints();

  if (AP <= 0)
  {
    assert(false && "Armor AP less than 1");
    return "";
  }

  const string absorptionPointsStr = toStr(AP);

  if (WITH_BRACKETS)
  {
    return "[" + absorptionPointsStr + "]";
  }
  else
  {
    return absorptionPointsStr;
  }
}

int Armor::takeDurHitAndGetReducedDmg(const int DMG_BEFORE)
{
  TRACE_FUNC_BEGIN;

  //Absorption points (AP) = damage soaked up instead of hitting the player
  //DDF = Damage (to) Durability Factor
  //(how much damage the durability takes per attack damage point)

  const int AP_BEFORE = getAbsorptionPoints();

  const double DDF_BASE         = data_->armor.dmgToDurabilityFactor;
  //TODO Add check for if wearer is player
  const double DDF_WAR_VET_MOD  = PlayerBon::getBg() == Bg::warVet ?
                                  0.5 : 1.0;
  const double DDF_K            = 1.5;

  const double DMG_BEFORE_DB = double(DMG_BEFORE);

  dur_ -= int(DMG_BEFORE_DB * DDF_BASE * DDF_WAR_VET_MOD * DDF_K);

  dur_ = max(0, dur_);

  const int AP_AFTER = getAbsorptionPoints();

  if (AP_AFTER < AP_BEFORE && AP_AFTER != 0)
  {
    const string armorName = getName(ItemRefType::plain);
    Log::addMsg("My " + armorName + " is damaged!", clrMsgWarning);
  }

  TRACE << "Damage before: " + toStr(DMG_BEFORE) << endl;

  const int DMG_AFTER = max(1, DMG_BEFORE - AP_BEFORE);

  TRACE << "Damage after: " + toStr(DMG_AFTER) << endl;

  TRACE_FUNC_END;
  return DMG_AFTER;
}

int Armor::getAbsorptionPoints() const
{
  const int AP_MAX = data_->armor.absorptionPoints;

  if (dur_ > 60) {return AP_MAX;}
  if (dur_ > 40) {return max(0, AP_MAX - 1);}
  if (dur_ > 25) {return max(0, AP_MAX - 2);}
  if (dur_ > 15) {return max(0, AP_MAX - 3);}

  return 0;
}

void ArmorAsbSuit::onWear()
{
  carrierProps_.push_back(new PropRFire(PropTurns::indefinite));
  carrierProps_.push_back(new PropRAcid(PropTurns::indefinite));
  carrierProps_.push_back(new PropRElec(PropTurns::indefinite));
  carrierProps_.push_back(new PropRBreath(PropTurns::indefinite));
}

void ArmorAsbSuit::onTakeOff()
{
  clearCarrierProps();
}

void ArmorHeavyCoat::onWear()
{
  carrierProps_.push_back(new PropRCold(PropTurns::indefinite));
}

void ArmorHeavyCoat::onTakeOff()
{
  clearCarrierProps();
}

//--------------------------------------------------------- WEAPON
Wpn::Wpn(ItemDataT* const itemData, ItemDataT* const ammoData) :
  Item(itemData), ammoData_(ammoData)
{
  nrAmmoLoaded = 0;
  ammoCapacity = 0;
  effectiveRangeLmt = 3;
  clip = false;
}

Clr Wpn::getClr() const
{
  if (data_->ranged.isRangedWpn && !data_->ranged.hasInfiniteAmmo)
  {
    if (nrAmmoLoaded == 0)
    {
      Clr ret = data_->clr;
      ret.r /= 2; ret.g /= 2; ret.b /= 2;
      return ret;
    }
  }
  return data_->clr;
}

void Wpn::setRandomMeleePlus()
{
  meleeDmgPlus_ = 0;

  int chance = 45;
  while (Rnd::percentile() < chance && meleeDmgPlus_ < 3)
  {
    meleeDmgPlus_++;
    chance -= 5;
  }
}

string Wpn::getNameInf() const
{
  if (data_->ranged.isRangedWpn && !data_->ranged.hasInfiniteAmmo)
  {
    return toStr(nrAmmoLoaded) + "/" + toStr(ammoCapacity);
  }
  return "";
}

//--------------------------------------------------------- STAFF OF THE PHARAOHS
PharaohStaff::PharaohStaff(ItemDataT* const itemData) : Wpn(itemData, nullptr)
{
  carrierSpells_.push_back(new SpellPharaohStaff);
}

//--------------------------------------------------------- MACHINE GUN
MachineGun::MachineGun(ItemDataT* const itemData, ItemDataT* const ammoData) :
  Wpn(itemData, ammoData)
{
  ammoCapacity = ammoData->ranged.ammoContainedInClip;
  nrAmmoLoaded = ammoCapacity;
  effectiveRangeLmt = 8;
  clip = true;
}

//--------------------------------------------------------- TESLA CANNON
TeslaCannon::TeslaCannon(ItemDataT* const itemData, ItemDataT* const ammoData) :
  Wpn(itemData, ammoData)
{
  ammoCapacity = ammoData->ranged.ammoContainedInClip;
  nrAmmoLoaded = ammoCapacity;
  effectiveRangeLmt = 8;
  clip = true;
}

//--------------------------------------------------------- SPIKE GUN
SpikeGun::SpikeGun(ItemDataT* const itemData, ItemDataT* const ammoData) :
  Wpn(itemData, ammoData)
{
  ammoCapacity = 12;
  nrAmmoLoaded = ammoCapacity;
  effectiveRangeLmt = 3;
  clip = true;
}

//--------------------------------------------------------- INCINERATOR
Incinerator::Incinerator(ItemDataT* const itemData, ItemDataT* const ammoData) :
  Wpn(itemData, ammoData)
{
  ammoCapacity = ammoData->ranged.ammoContainedInClip;
  nrAmmoLoaded = ammoCapacity;
  effectiveRangeLmt = 8;
  clip = false;
}

void Incinerator::projectileObstructed(
  const Pos& pos, Actor* actorHit)
{
  (void)actorHit;
  Explosion::runExplosionAt(pos, ExplType::expl);
}

//--------------------------------------------------------- MEDICAL BAG
AmmoClip::AmmoClip(ItemDataT* const itemData) : Ammo(itemData)
{
  setFullAmmo();
}

void AmmoClip::setFullAmmo()
{
  ammo_ = data_->ranged.ammoContainedInClip;
}

//--------------------------------------------------------- MEDICAL BAG
const int NR_TRN_BEFORE_HEAL  = 10;
const int NR_TRN_PER_HP       = 2;

ConsumeItem MedicalBag::activateDefault(Actor* const actor)
{
  (void)actor;

  vector<Actor*> seenFoes;
  Map::player->getSeenFoes(seenFoes);
  if (!seenFoes.empty())
  {
    Log::addMsg("Not while an enemy is near.");
    curAction_ = MedBagAction::END;
    return ConsumeItem::no;
  }

  curAction_ = playerChooseAction();

  Log::clearLog();

  if (curAction_ == MedBagAction::END) {return ConsumeItem::no;}

  //Check if chosen action can be done
  bool props[endOfPropIds];
  Map::player->getPropHandler().getAllActivePropIds(props);
  switch (curAction_)
  {
    case MedBagAction::treatWounds:
    {
      if (Map::player->getHp() >= Map::player->getHpMax(true))
      {
        Log::addMsg("I have no wounds to treat.");
        curAction_ = MedBagAction::END;
        return ConsumeItem::no;
      }
    } break;

    case MedBagAction::sanitizeInfection:
    {
      if (!props[propInfected])
      {
        Log::addMsg("I have no infection to sanitize.");
        curAction_ = MedBagAction::END;
        return ConsumeItem::no;
      }
    } break;

//      case MedBagAction::takeMorphine: {
//        if(Map::player->getHp() >= Map::player->getHpMax(true)) {
//          Log::addMsg("I am not in pain.");
//          curAction_ = MedBagAction::END;
//        }
//      } break;

    case MedBagAction::END: {} break;
  }

  bool isEnoughSuppl = true;

  switch (curAction_)
  {
    case MedBagAction::sanitizeInfection:
    {
      isEnoughSuppl = getTotSupplForSanitize() <= nrSupplies_;
    } break;

    case MedBagAction::treatWounds: //Costs one supply per turn
    case MedBagAction::END: {} break;
  }

  if (!isEnoughSuppl)
  {
    Log::addMsg("I do not have enough supplies for that.");
    curAction_ = MedBagAction::END;
    return ConsumeItem::no;
  }

  //Action can be done
  Map::player->activeMedicalBag = this;

  switch (curAction_)
  {
    case MedBagAction::treatWounds:
    {
      Log::addMsg("I start to treat my wounds...");
      nrTurnsUntilHealWounds_ = NR_TRN_BEFORE_HEAL;
    } break;

    case MedBagAction::sanitizeInfection:
    {
      Log::addMsg("I start to sanitize an infection...");
      nrTurnsLeftSanitize_ = getTotTurnsForSanitize();
    } break;

//        case MedBagAction::takeMorphine: {
//          Log::addMsg("I start to take Morphine...");
//        } break;

    case MedBagAction::END: {} break;
  }

  GameTime::actorDidAct();

  return ConsumeItem::no;
}

MedBagAction MedicalBag::playerChooseAction() const
{

  Log::clearLog();

  Log::addMsg("Use Medical Bag how? [h/enter] Treat wounds [s] Sanitize infection",
              clrWhiteHigh);

//  int suppl = getTotSuppliesFor(MedBagAction::treatWounds);
//  int turns = getTotTurnsFor(MedBagAction::treatWounds);
//  Log::addMsg("[h/enter] Treat wounds (" + toStr(suppl) + "," + toStr(turns) + ")",
//              clrWhiteHigh);

//  suppl     = getTotSuppliesFor(MedBagAction::sanitizeInfection);
//  turns     = getTotTurnsFor(MedBagAction::sanitizeInfection);
//  Log::addMsg("[s] Sanitize infection (" + toStr(suppl) + "," + toStr(turns) + ")",
//              clrWhiteHigh);

  Render::drawMapAndInterface(true);

  while (true)
  {
    const KeyData d = Query::letter(true);
    if (d.sdlKey == SDLK_ESCAPE || d.sdlKey == SDLK_SPACE)
    {
      return MedBagAction::END;
    }
    else if (d.sdlKey == SDLK_RETURN || d.key == 'h')
    {
      return MedBagAction::treatWounds;
    }
    else if (d.key == 's')
    {
      return MedBagAction::sanitizeInfection;
    }
  }

  return MedBagAction(MedBagAction::END);
}

void MedicalBag::continueAction()
{
  switch (curAction_)
  {
    case MedBagAction::treatWounds:
    {

      auto& player = *Map::player;

      const bool IS_HEALER = PlayerBon::hasTrait(Trait::healer);

      if (nrTurnsUntilHealWounds_ > 0)
      {
        nrTurnsUntilHealWounds_ -= IS_HEALER ? 2 : 1;
      }
      else
      {
        //If player is healer, double the rate of HP healing.
        const int NR_TRN_PER_HP_W_BON = IS_HEALER ? (NR_TRN_PER_HP / 2) : NR_TRN_PER_HP;

        if (GameTime::getTurn() % NR_TRN_PER_HP_W_BON == 0)
        {
          player.restoreHp(1, false);
        }

        //The rate of supply use is consistent (effectively, this means that with the
        //healer trait, you spend half the time and supplies, as the description says).
        if (GameTime::getTurn() % NR_TRN_PER_HP == 0)
        {
          --nrSupplies_;
        }
      }

      if (nrSupplies_ <= 0)
      {
        Log::addMsg("No more medical supplies.");
        finishCurAction();
        return;
      }

      if (player.getHp() >= player.getHpMax(true))
      {
        finishCurAction();
        return;
      }

      GameTime::actorDidAct();

    } break;

    case MedBagAction::sanitizeInfection:
    {
      --nrTurnsLeftSanitize_;
      if (nrTurnsLeftSanitize_ <= 0)
      {
        finishCurAction();
      }
      else
      {
        GameTime::actorDidAct();
      }
    } break;

    case MedBagAction::END:
    {
      assert(false && "Illegal action");
    } break;
  }
}

void MedicalBag::finishCurAction()
{
  Map::player->activeMedicalBag = nullptr;

  switch (curAction_)
  {
    case MedBagAction::sanitizeInfection:
    {
      bool blockedLos[MAP_W][MAP_H];
      MapParse::parse(CellCheck::BlocksLos(), blockedLos);
      Map::player->getPropHandler().endAppliedProp(propInfected, blockedLos);
      nrSupplies_ -= getTotSupplForSanitize();
    } break;

    case MedBagAction::treatWounds:
    {
      Log::addMsg("I finish treating my wounds.");
    } break;

//    case MedBagAction::takeMorphine: {
//      Map::player->restoreHp(999);
//      Log::addMsg("The morphine takes a toll on my mind.");
//      Map::player->incrShock(ShockValue::heavy, ShockSrc::misc);
//    } break;

    case MedBagAction::END: {} break;
  }

  curAction_ = MedBagAction::END;

  if (nrSupplies_ <= 0)
  {
    Map::player->getInv().removeItemInGeneralWithPtr(this, true);
  }
}

void MedicalBag::interrupted()
{
  Log::addMsg("My healing is disrupted.", clrWhite, false);

  nrTurnsUntilHealWounds_ = -1;
  nrTurnsLeftSanitize_    = -1;

  Map::player->activeMedicalBag = nullptr;
}

int MedicalBag::getTotTurnsForSanitize() const
{
  return PlayerBon::hasTrait(Trait::healer) ? 10 : 20;
}

int MedicalBag::getTotSupplForSanitize() const
{
  return PlayerBon::hasTrait(Trait::healer) ? 5 : 10;
}

//--------------------------------------------------------- HIDEOUS MASK
void HideousMask::newTurnInInventory()
{
  vector<Actor*> adjActors;
  const Pos p(Map::player->pos);
  for (auto* const actor : GameTime::actors_)
  {
    if (actor->isAlive() && Utils::isPosAdj(p, actor->pos, false))
    {
      adjActors.push_back(actor);
    }
  }
  if (!adjActors.empty())
  {
    bool blockedLos[MAP_W][MAP_H];
    MapParse::parse(CellCheck::BlocksLos(), blockedLos);
    for (auto* const actor : adjActors)
    {
      if (Rnd::oneIn(4) && actor->isSeeingActor(*Map::player, blockedLos))
      {
        actor->getPropHandler().tryApplyProp(new PropTerrified(PropTurns::std));
      }
    }
  }
}

//--------------------------------------------------------- GAS MASK
void GasMask::onTakeOff()
{
  clearCarrierProps();
}

void GasMask::onWear()
{
  carrierProps_.push_back(new PropRBreath(PropTurns::indefinite));
}

//--------------------------------------------------------- EXPLOSIVE
ConsumeItem Explosive::activateDefault(Actor* const actor)
{
  (void)actor;
  //Make a copy to use as the held ignited explosive.
  auto* cpy = static_cast<Explosive*>(ItemFactory::mk(getData().id, 1));

  cpy->fuseTurns_               = getStdFuseTurns();
  Map::player->activeExplosive  = cpy;
  Map::player->updateClr();
  cpy->onPlayerIgnite();
  return ConsumeItem::yes;
}

//--------------------------------------------------------- DYNAMITE
void Dynamite::onPlayerIgnite() const
{
  const bool IS_SWIFT   = PlayerBon::hasTrait(Trait::demExpert) && Rnd::coinToss();
  const string swiftStr = IS_SWIFT ? "swiftly " : "";

  Log::addMsg("I " + swiftStr + "light a dynamite stick.");
  Render::drawMapAndInterface();
  GameTime::actorDidAct(IS_SWIFT);
}

void Dynamite::onStdTurnPlayerHoldIgnited()
{
  fuseTurns_--;
  if (fuseTurns_ > 0)
  {
    string fuseMsg = "***F";
    for (int i = 0; i < fuseTurns_; ++i) {fuseMsg += "Z";}
    fuseMsg += "***";
    Log::addMsg(fuseMsg, clrYellow);
  }
  else
  {
    Log::addMsg("The dynamite explodes in my hands!");
    Map::player->activeExplosive = nullptr;
    Explosion::runExplosionAt(Map::player->pos, ExplType::expl);
    Map::player->updateClr();
    fuseTurns_ = -1;
    delete this;
  }
}

void Dynamite::onThrownIgnitedLanding(const Pos& p)
{
  GameTime::addMob(new LitDynamite(p, fuseTurns_));
}

void Dynamite::onPlayerParalyzed()
{
  Log::addMsg("The lit Dynamite stick falls from my hands!");
  Map::player->activeExplosive = nullptr;
  Map::player->updateClr();
  const Pos& p = Map::player->pos;
  auto* const f = Map::cells[p.x][p.y].rigid;
  if (!f->isBottomless()) {GameTime::addMob(new LitDynamite(p, fuseTurns_));}
  delete this;
}

//--------------------------------------------------------- MOLOTOV
void Molotov::onPlayerIgnite() const
{
  const bool IS_SWIFT   = PlayerBon::hasTrait(Trait::demExpert) && Rnd::coinToss();
  const string swiftStr = IS_SWIFT ? "swiftly " : "";

  Log::addMsg("I " + swiftStr + "light a Molotov Cocktail.");
  Render::drawMapAndInterface();
  GameTime::actorDidAct(IS_SWIFT);
}

void Molotov::onStdTurnPlayerHoldIgnited()
{
  fuseTurns_--;

  if (fuseTurns_ <= 0)
  {
    Log::addMsg("The Molotov Cocktail explodes in my hands!");
    Map::player->activeExplosive = nullptr;
    Map::player->updateClr();
    Explosion::runExplosionAt(Map::player->pos, ExplType::applyProp, ExplSrc::misc, 0,
                              SfxId::explosionMolotov, new PropBurning(PropTurns::std));
    delete this;
  }
}

void Molotov::onThrownIgnitedLanding(const Pos& p)
{
  const int D = PlayerBon::hasTrait(Trait::demExpert) ? 1 : 0;
  Explosion::runExplosionAt(p, ExplType::applyProp, ExplSrc::playerUseMoltvIntended, D,
                            SfxId::explosionMolotov, new PropBurning(PropTurns::std));
}


void Molotov::onPlayerParalyzed()
{
  Log::addMsg("The lit Molotov Cocktail falls from my hands!");
  Map::player->activeExplosive = nullptr;
  Map::player->updateClr();
  Explosion::runExplosionAt(Map::player->pos, ExplType::applyProp, ExplSrc::misc, 0,
                            SfxId::explosionMolotov, new PropBurning(PropTurns::std));
  delete this;
}

//--------------------------------------------------------- FLARE
void Flare::onPlayerIgnite() const
{
  const bool IS_SWIFT   = PlayerBon::hasTrait(Trait::demExpert) && Rnd::coinToss();
  const string swiftStr = IS_SWIFT ? "swiftly " : "";

  Log::addMsg("I " + swiftStr + "light a Flare.");
  GameTime::updateLightMap();
  Map::player->updateFov();
  Render::drawMapAndInterface();
  GameTime::actorDidAct(IS_SWIFT);
}

void Flare::onStdTurnPlayerHoldIgnited()
{
  fuseTurns_--;
  if (fuseTurns_ <= 0)
  {
    Log::addMsg("The flare is extinguished.");
    Map::player->activeExplosive = nullptr;
    Map::player->updateClr();
    delete this;
  }
}

void Flare::onThrownIgnitedLanding(const Pos& p)
{
  GameTime::addMob(new LitFlare(p, fuseTurns_));
  GameTime::updateLightMap();
  Map::player->updateFov();
  Render::drawMapAndInterface();
}

void Flare::onPlayerParalyzed()
{
  Log::addMsg("The lit Flare falls from my hands.");
  Map::player->activeExplosive = nullptr;
  Map::player->updateClr();
  const Pos&  p = Map::player->pos;
  auto* const f = Map::cells[p.x][p.y].rigid;
  if (!f->isBottomless()) {GameTime::addMob(new LitFlare(p, fuseTurns_));}
  GameTime::updateLightMap();
  Map::player->updateFov();
  Render::drawMapAndInterface();
  delete this;
}

//--------------------------------------------------------- SMOKE GRENADE
void SmokeGrenade::onPlayerIgnite() const
{
  const bool IS_SWIFT   = PlayerBon::hasTrait(Trait::demExpert) && Rnd::coinToss();
  const string swiftStr = IS_SWIFT ? "swiftly " : "";

  Log::addMsg("I " + swiftStr + "ignite a smoke grenade.");
  Render::drawMapAndInterface();
  GameTime::actorDidAct(IS_SWIFT);
}

void SmokeGrenade::onStdTurnPlayerHoldIgnited()
{
  if (fuseTurns_ < getStdFuseTurns() && Rnd::coinToss())
  {
    Explosion::runSmokeExplosionAt(Map::player->pos);
  }
  fuseTurns_--;
  if (fuseTurns_ <= 0)
  {
    Log::addMsg("The smoke grenade is extinguished.");
    Map::player->activeExplosive = nullptr;
    Map::player->updateClr();
    delete this;
  }
}

void SmokeGrenade::onThrownIgnitedLanding(const Pos& p)
{
  Explosion::runSmokeExplosionAt(p);
  Map::player->updateFov();
  Render::drawMapAndInterface();
}

void SmokeGrenade::onPlayerParalyzed()
{
  Log::addMsg("The ignited smoke grenade falls from my hands.");
  Map::player->activeExplosive = nullptr;
  Map::player->updateClr();
  const Pos&  p = Map::player->pos;
  auto* const f = Map::cells[p.x][p.y].rigid;
  if (!f->isBottomless()) {Explosion::runSmokeExplosionAt(Map::player->pos);}
  Map::player->updateFov();
  Render::drawMapAndInterface();
  delete this;
}

Clr SmokeGrenade::getIgnitedProjectileClr() const
{
  return getData().clr;
}

