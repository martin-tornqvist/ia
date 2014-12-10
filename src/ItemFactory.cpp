#include "ItemFactory.h"

#include "Init.h"
#include "ItemScroll.h"
#include "ItemPotion.h"
#include "ItemDrop.h"
#include "ItemDevice.h"
#include "Utils.h"
#include "ItemData.h"

using namespace std;

namespace ItemFactory
{

Item* mk(const ItemId itemId, const int NR_ITEMS)
{
  Item*             r     = nullptr;
  ItemDataT* const  d     = ItemData::data[int(itemId)];
  ItemDataT*        ammoD = nullptr;

  if (d->ranged.ammoItemId != ItemId::END)
  {
    ammoD = ItemData::data[int(d->ranged.ammoItemId)];
  }

  switch (itemId)
  {
    case ItemId::trapezohedron:
      r = new Item(d);
      break;

    case ItemId::rock:
    case ItemId::throwingKnife:
    case ItemId::ironSpike:
    case ItemId::dagger:
    case ItemId::hatchet:
    case ItemId::club:
    case ItemId::hammer:
    case ItemId::machete:
    case ItemId::axe:
    case ItemId::pitchFork:
    case ItemId::sledgeHammer:
    case ItemId::playerKick:
    case ItemId::playerStomp:
    case ItemId::playerPunch:
    case ItemId::zombieAxe:
    case ItemId::zombieClaw:
    case ItemId::zombieClawDiseased:
    case ItemId::bloatedZombiePunch:
    case ItemId::bloatedZombieSpit:
    case ItemId::ratBite:
    case ItemId::ratBiteDiseased:
    case ItemId::ratThingBite:
    case ItemId::brownJenkinBite:
    case ItemId::wormMassBite:
    case ItemId::greenSpiderBite:
    case ItemId::whiteSpiderBite:
    case ItemId::redSpiderBite:
    case ItemId::shadowSpiderBite:
    case ItemId::lengSpiderBite:
    case ItemId::fireHoundBreath:
    case ItemId::frostHoundBreath:
    case ItemId::fireHoundBite:
    case ItemId::frostHoundBite:
    case ItemId::zuulBite:
    case ItemId::giantBatBite:
    case ItemId::wolfBite:
    case ItemId::ghostClaw:
    case ItemId::phantasmSickle:
    case ItemId::wraithClaw:
    case ItemId::polypTentacle:
    case ItemId::ghoulClaw:
    case ItemId::shadowClaw:
    case ItemId::byakheeClaw:
    case ItemId::giantMantisClaw:
    case ItemId::giantLocustBite:
    case ItemId::mummyMaul:
    case ItemId::deepOneJavelinAtt:
    case ItemId::deepOneSpearAtt:
    case ItemId::oozeBlackSpewPus:
    case ItemId::oozeClearSpewPus:
    case ItemId::oozePutridSpewPus:
    case ItemId::oozePoisonSpewPus:
    case ItemId::colourOOSpaceTouch:
    case ItemId::chthonianBite:
    case ItemId::huntingHorrorBite:
    case ItemId::dustVortexEngulf:
    case ItemId::fireVortexEngulf:
    case ItemId::frostVortexEngulf:
    case ItemId::moldSpores:
      r = new Wpn(d, ammoD);
      break;

    case ItemId::pharaohStaff:
      r = new PharaohStaff(d);
      break;

    case ItemId::dynamite:
      r = new Dynamite(d);
      break;

    case ItemId::flare:
      r = new Flare(d);
      break;

    case ItemId::molotov:
      r = new Molotov(d);
      break;

    case ItemId::smokeGrenade:
      r = new SmokeGrenade(d);
      break;

    case ItemId::sawedOff:
      r = new SawedOff(d, ammoD);
      break;

    case ItemId::pumpShotgun:
      r = new PumpShotgun(d, ammoD);
      break;

    case ItemId::shotgunShell:
      r = new Ammo(d);
      break;

    case ItemId::machineGun:
      r = new MachineGun(d, ammoD);
      break;

    case ItemId::drumOfBullets:
    case ItemId::pistolClip:
    case ItemId::incineratorAmmo:
    case ItemId::migoGunAmmo:
      r = new AmmoClip(d);
      break;

    case ItemId::pistol:
      r = new Pistol(d, ammoD);
      break;

    case ItemId::flareGun:
      r = new FlareGun(d, ammoD);
      break;

    case ItemId::incinerator:
      r = new Incinerator(d, ammoD);
      break;

    case ItemId::spikeGun:
      r = new SpikeGun(d, ammoD);
      break;

    case ItemId::migoGun:
      r = new MigoGun(d, ammoD);
      break;

    case ItemId::armorFlackJacket:
    case ItemId::armorLeatherJacket:
    case ItemId::armorIronSuit:
      r = new Armor(d);
      break;

    case ItemId::armorAsbSuit:
      r = new ArmorAsbSuit(d);
      break;

    case ItemId::armorHeavyCoat:
      r = new ArmorHeavyCoat(d);
      break;

    case ItemId::armorMigo:
      r = new ArmorMigo(d);
      break;

    case ItemId::gasMask:
      r = new GasMask(d);
      break;

    case ItemId::hideousMask:
      r = new HideousMask(d);
      break;

    case ItemId::scrollMayhem:
    case ItemId::scrollTelep:
    case ItemId::scrollPest:
    case ItemId::scrollSlowMon:
    case ItemId::scrollTerrifyMon:
    case ItemId::scrollParalMon:
    case ItemId::scrollDetItems:
    case ItemId::scrollDetTraps:
    case ItemId::scrollDetMon:
    case ItemId::scrollBless:
    case ItemId::scrollDarkbolt:
    case ItemId::scrollAzaWrath:
    case ItemId::scrollOpening:
    case ItemId::scrollSacrLife:
    case ItemId::scrollSacrSpi:
    case ItemId::scrollElemRes:
    case ItemId::scrollSummonMon:
    case ItemId::scrollLight:
      r = new Scroll(d);
      break;

    case ItemId::potionVitality:
      r = new PotionVitality(d);
      break;

    case ItemId::potionSpirit:
      r = new PotionSpirit(d);
      break;

    case ItemId::potionBlindness:
      r = new PotionBlindness(d);
      break;

    case ItemId::potionFrenzy:
      r = new PotionFrenzy(d);
      break;

    case ItemId::potionFortitude:
      r = new PotionFortitude(d);
      break;

    case ItemId::potionParalyze:
      r = new PotionParal(d);
      break;

    case ItemId::potionRElec:
      r = new PotionRElec(d);
      break;

    case ItemId::potionConf:
      r = new PotionConf(d);
      break;

    case ItemId::potionPoison:
      r = new PotionPoison(d);
      break;

    case ItemId::potionInsight:
      r = new PotionInsight(d);
      break;

    case ItemId::potionClairv:
      r = new PotionClairv(d);
      break;

    case ItemId::potionRFire:
      r = new PotionRFire(d);
      break;

    case ItemId::potionAntidote:
      r = new PotionAntidote(d);
      break;

    case ItemId::potionDescent:
      r = new PotionDescent(d);
      break;

    case ItemId::deviceBlaster:
      r = new DeviceBlaster(d);
      break;

    case ItemId::deviceShockwave:
      r = new DeviceShockwave(d);
      break;

    case ItemId::deviceRejuvenator:
      r = new DeviceRejuvenator(d);
      break;

    case ItemId::deviceTranslocator:
      r = new DeviceTranslocator(d);
      break;

    case ItemId::deviceSentryDrone:
      r = new DeviceSentryDrone(d);
      break;

    case ItemId::electricLantern:
      r = new DeviceLantern(d);
      break;

    case ItemId::medicalBag:
      r = new MedicalBag(d);
      break;

    case ItemId::END:
      return nullptr;
  }

  if (!r->getData().isStackable && NR_ITEMS != 1)
  {
    TRACE << "Specified number of items (" + toStr(NR_ITEMS) + ") != 1 for "
          << "non-stackable item" << endl;
    assert(false);
  }
  else
  {
    r->nrItems_ = NR_ITEMS;
  }

  return r;
}

void setItemRandomizedProperties(Item* item)
{
  const ItemDataT& d = item->getData();

  //If it is a pure melee weapon, it may get a plus
  if (d.melee.isMeleeWpn && !d.ranged.isRangedWpn)
  {
    static_cast<Wpn*>(item)->setRandomMeleePlus();
  }

  //If firearm, spawn with random amount of ammo
  if (d.ranged.isRangedWpn && !d.ranged.hasInfiniteAmmo)
  {
    Wpn* const wpn = static_cast<Wpn*>(item);
    if (wpn->AMMO_CAP == 1)
    {
      wpn->nrAmmoLoaded = Rnd::coinToss() ? 1 : 0;
    }
    else
    {
      if (d.ranged.isMachineGun)
      {
        //Number of machine gun bullets loaded needs to be a multiple of the number of
        //projectiles fired in each burst
        const int CAP         = wpn->AMMO_CAP;
        const int CAP_SCALED  = CAP / NR_MG_PROJECTILES;
        const int MIN_SCALED  = CAP_SCALED / 4;
        wpn->nrAmmoLoaded     = Rnd::range(MIN_SCALED, CAP_SCALED) * NR_MG_PROJECTILES;
      }
      else
      {
        wpn->nrAmmoLoaded = Rnd::range(wpn->AMMO_CAP / 4, wpn->AMMO_CAP);
      }
    }
  }

  if (d.isStackable) {item->nrItems_ = Rnd::range(1, d.maxStackAtSpawn);}
}

Item* mkItemOnMap(const ItemId itemId, const Pos& pos)
{
  Item* item = mk(itemId);
  setItemRandomizedProperties(item);
  ItemDrop::dropItemOnMap(pos, *item);
  return item;
}

Item* copyItem(Item* oldItem)
{
  Item* newItem     = mk(oldItem->getData().id);
  newItem->nrItems_ = oldItem->nrItems_;
  return newItem;
}

Item* mkRandomScrollOrPotion(const bool ALLOW_SCROLLS, const bool ALLOW_POTIONS)
{
  vector<ItemId> itemBucket;

  for (int i = 0; i < int(ItemId::END); ++i)
  {
    const ItemDataT* const d = ItemData::data[i];

    if (!d->isIntrinsic &&
        ((d->isScroll && ALLOW_SCROLLS) || (d->isPotion && ALLOW_POTIONS)))
    {
      itemBucket.push_back(static_cast<ItemId>(i));
    }
  }

  if (!itemBucket.empty())
  {
    const int ELEMENT = Rnd::range(0, itemBucket.size() - 1);
    return mk(itemBucket[ELEMENT]);
  }

  return nullptr;
}

} //ItemFactory
