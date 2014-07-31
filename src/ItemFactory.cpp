#include "ItemFactory.h"

#include "Init.h"
#include "ItemWeapon.h"
#include "ItemAmmo.h"
#include "ItemExplosive.h"
#include "ItemArmor.h"
#include "ItemScroll.h"
#include "ItemPotion.h"
#include "ItemDrop.h"
#include "ItemDevice.h"
#include "ItemMedicalBag.h"
#include "Utils.h"

using namespace std;

namespace ItemFactory {

Item* mk(const ItemId itemId, const int NR_ITEMS) {
  Item* r = nullptr;

  ItemDataT* const d = ItemData::data[int(itemId)];

  ItemDataT* ammoD = nullptr;

  if(d->rangedAmmoTypeUsed != ItemId::empty) {
    ammoD = ItemData::data[int(d->rangedAmmoTypeUsed)];
  }

  switch(itemId) {
    case ItemId::trapezohedron:       r = new Item(d);                  break;

    case ItemId::rock:                r = new Wpn(d, ammoD);         break;
    case ItemId::throwingKnife:       r = new Wpn(d, ammoD);         break;
    case ItemId::ironSpike:           r = new Wpn(d, ammoD);         break;
    case ItemId::dagger:              r = new Wpn(d, ammoD);         break;
    case ItemId::hatchet:             r = new Wpn(d, ammoD);         break;
    case ItemId::club:                r = new Wpn(d, ammoD);         break;
    case ItemId::hammer:              r = new Wpn(d, ammoD);         break;
    case ItemId::machete:             r = new Wpn(d, ammoD);         break;
    case ItemId::axe:                 r = new Wpn(d, ammoD);         break;
    case ItemId::pitchFork:           r = new Wpn(d, ammoD);         break;
    case ItemId::sledgeHammer:        r = new Wpn(d, ammoD);         break;

    case ItemId::dynamite:            r = new Dynamite(d);              break;
    case ItemId::flare:               r = new Flare(d);                 break;
    case ItemId::molotov:             r = new Molotov(d);               break;

    case ItemId::sawedOff:            r = new SawedOff(d, ammoD);       break;
    case ItemId::pumpShotgun:         r = new PumpShotgun(d, ammoD);    break;
    case ItemId::shotgunShell:        r = new ItemAmmo(d);              break;
    case ItemId::machineGun:          r = new MachineGun(d, ammoD);     break;
    case ItemId::drumOfBullets:       r = new ItemAmmoClip(d);          break;
    case ItemId::pistol:              r = new Pistol(d, ammoD);         break;
    case ItemId::flareGun:            r = new FlareGun(d, ammoD);       break;
    case ItemId::pistolClip:          r = new ItemAmmoClip(d);          break;
    case ItemId::incinerator:         r = new Incinerator(d, ammoD);    break;
    case ItemId::teslaCannon:         r = new TeslaCannon(d, ammoD);    break;
    case ItemId::spikeGun:            r = new SpikeGun(d, ammoD);       break;
    case ItemId::teslaCanister:       r = new ItemAmmoClip(d);          break;
    case ItemId::napalmCartridge:     r = new ItemAmmoClip(d);          break;

    case ItemId::playerKick:          r = new Wpn(d, ammoD);         break;
    case ItemId::playerStomp:         r = new Wpn(d, ammoD);         break;
    case ItemId::playerPunch:         r = new Wpn(d, ammoD);         break;

    case ItemId::armorFlackJacket:    r = new Armor(d);                 break;
    case ItemId::armorLeatherJacket:  r = new Armor(d);                 break;
    case ItemId::armorIronSuit:       r = new Armor(d);                 break;
    case ItemId::armorAsbSuit:        r = new ArmorAsbSuit(d);          break;
    case ItemId::armorHeavyCoat:      r = new ArmorHeavyCoat(d);        break;

    case ItemId::zombieAxe:           r = new Wpn(d, ammoD);         break;
    case ItemId::zombieClaw:          r = new Wpn(d, ammoD);         break;
    case ItemId::zombieClawDiseased:  r = new Wpn(d, ammoD);         break;
    case ItemId::bloatedZombiePunch:  r = new Wpn(d, ammoD);         break;
    case ItemId::bloatedZombieSpit:   r = new Wpn(d, ammoD);         break;
    case ItemId::ratBite:             r = new Wpn(d, ammoD);         break;
    case ItemId::ratBiteDiseased:     r = new Wpn(d, ammoD);         break;
    case ItemId::ratThingBite:        r = new Wpn(d, ammoD);         break;
    case ItemId::wormMassBite:        r = new Wpn(d, ammoD);         break;
    case ItemId::greenSpiderBite:     r = new Wpn(d, ammoD);         break;
    case ItemId::whiteSpiderBite:     r = new Wpn(d, ammoD);         break;
    case ItemId::redSpiderBite:       r = new Wpn(d, ammoD);         break;
    case ItemId::shadowSpiderBite:    r = new Wpn(d, ammoD);         break;
    case ItemId::lengSpiderBite:      r = new Wpn(d, ammoD);         break;
    case ItemId::fireHoundBreath:     r = new Wpn(d, ammoD);         break;
    case ItemId::frostHoundBreath:    r = new Wpn(d, ammoD);         break;
    case ItemId::fireHoundBite:       r = new Wpn(d, ammoD);         break;
    case ItemId::frostHoundBite:      r = new Wpn(d, ammoD);         break;
    case ItemId::zuulBite:            r = new Wpn(d, ammoD);         break;
    case ItemId::giantBatBite:        r = new Wpn(d, ammoD);         break;
    case ItemId::wolfBite:            r = new Wpn(d, ammoD);         break;
    case ItemId::ghostClaw:           r = new Wpn(d, ammoD);         break;
    case ItemId::phantasmSickle:      r = new Wpn(d, ammoD);         break;
    case ItemId::wraithClaw:          r = new Wpn(d, ammoD);         break;
    case ItemId::polypTentacle:       r = new Wpn(d, ammoD);         break;
    case ItemId::miGoElectricGun:     r = new Wpn(d, ammoD);         break;
    case ItemId::ghoulClaw:           r = new Wpn(d, ammoD);         break;
    case ItemId::shadowClaw:          r = new Wpn(d, ammoD);         break;
    case ItemId::byakheeClaw:         r = new Wpn(d, ammoD);         break;
    case ItemId::giantMantisClaw:     r = new Wpn(d, ammoD);         break;
    case ItemId::giantLocustBite:     r = new Wpn(d, ammoD);         break;
    case ItemId::mummyMaul:           r = new Wpn(d, ammoD);         break;
    case ItemId::deepOneJavelinAtt:   r = new Wpn(d, ammoD);         break;
    case ItemId::deepOneSpearAtt:     r = new Wpn(d, ammoD);         break;
    case ItemId::oozeBlackSpewPus:    r = new Wpn(d, ammoD);         break;
    case ItemId::oozeClearSpewPus:    r = new Wpn(d, ammoD);         break;
    case ItemId::oozePutridSpewPus:   r = new Wpn(d, ammoD);         break;
    case ItemId::oozePoisonSpewPus:   r = new Wpn(d, ammoD);         break;
    case ItemId::colourOOSpaceTouch:  r = new Wpn(d, ammoD);         break;
    case ItemId::chthonianBite:       r = new Wpn(d, ammoD);         break;
    case ItemId::huntingHorrorBite:   r = new Wpn(d, ammoD);         break;
    case ItemId::dustVortexEngulf:    r = new Wpn(d, ammoD);         break;
    case ItemId::fireVortexEngulf:    r = new Wpn(d, ammoD);         break;
    case ItemId::frostVortexEngulf:   r = new Wpn(d, ammoD);         break;

    case ItemId::scrollMayhem:        r = new Scroll(d);                break;
    case ItemId::scrollTelep:         r = new Scroll(d);                break;
    case ItemId::scrollPestilence:    r = new Scroll(d);                break;
    case ItemId::scrollSlowMon:       r = new Scroll(d);                break;
    case ItemId::scrollTerrifyMon:    r = new Scroll(d);                break;
    case ItemId::scrollParalMon:      r = new Scroll(d);                break;
    case ItemId::scrollDetItems:      r = new Scroll(d);                break;
    case ItemId::scrollDetTraps:      r = new Scroll(d);                break;
    case ItemId::scrollDetMon:        r = new Scroll(d);                break;
    case ItemId::scrollBless:         r = new Scroll(d);                break;
    case ItemId::scrollDarkbolt:      r = new Scroll(d);                break;
    case ItemId::scrollAzaWrath:      r = new Scroll(d);                break;
    case ItemId::scrollOpening:       r = new Scroll(d);                break;
    case ItemId::scrollSacrLife:      r = new Scroll(d);                break;
    case ItemId::scrollSacrSpi:       r = new Scroll(d);                break;
    case ItemId::scrollElemRes:       r = new Scroll(d);                break;

    case ItemId::potionVitality:      r = new PotionVitality(d);        break;
    case ItemId::potionSpirit:        r = new PotionSpirit(d);          break;
    case ItemId::potionBlindness:     r = new PotionBlindness(d);       break;
    case ItemId::potionFrenzy:        r = new PotionFrenzy(d);          break;
    case ItemId::potionFortitude:     r = new PotionFortitude(d);       break;
    case ItemId::potionParalyze:      r = new PotionParal(d);           break;
    case ItemId::potionRElec:         r = new PotionRElec(d);           break;
    case ItemId::potionConf:          r = new PotionConf(d);            break;
    case ItemId::potionPoison:        r = new PotionPoison(d);          break;
    case ItemId::potionInsight:       r = new PotionInsight(d);         break;
    case ItemId::potionClairv:        r = new PotionClairv(d);          break;
    case ItemId::potionRFire:         r = new PotionRFire(d);           break;
    case ItemId::potionAntidote:      r = new PotionAntidote(d);        break;
    case ItemId::potionDescent:       r = new PotionDescent(d);         break;

    case ItemId::deviceSentry:        r = new DeviceSentry(d);          break;
    case ItemId::deviceRepeller:      r = new DeviceRepeller(d);        break;
    case ItemId::deviceRejuvenator:   r = new DeviceRejuvenator(d);     break;
    case ItemId::deviceTranslocator:  r = new DeviceTranslocator(d);    break;
    case ItemId::electricLantern:     r = new DeviceLantern(d);         break;

    case ItemId::medicalBag:          r = new MedicalBag(d);            break;

    case ItemId::empty:
    case ItemId::END: return nullptr;
  }

  if(!r->getData().isStackable && NR_ITEMS != 1) {
    TRACE << "Warning, Specified " + toStr(NR_ITEMS) + " nr items"
          << " for non-stackable item";
  } else {
    r->nrItems = NR_ITEMS;
  }

  return r;
}

void setItemRandomizedProperties(Item* item) {
  const ItemDataT& d = item->getData();

  //If it is a pure melee weapon, it may get a plus
  if(d.isMeleeWpn && !d.isRangedWpn) {
    static_cast<Wpn*>(item)->setRandomMeleePlus();
  }

  //If firearm, spawn with random amount of ammo
  if(d.isRangedWpn && !d.rangedHasInfiniteAmmo) {
    Wpn* const weapon = static_cast<Wpn*>(item);
    if(weapon->ammoCapacity == 1) {
      weapon->nrAmmoLoaded = Rnd::coinToss() ? 1 : 0;
    } else {
      if(d.isMachineGun) {
        const int CAP = weapon->ammoCapacity;
        const int MIN = CAP / 2;
        const int CAP_SCALED = CAP / NR_MG_PROJECTILES;
        const int MIN_SCALED = MIN / NR_MG_PROJECTILES;
        weapon->nrAmmoLoaded =
          Rnd::range(MIN_SCALED, CAP_SCALED) *
          NR_MG_PROJECTILES;
      } else {
        weapon->nrAmmoLoaded =
          Rnd::range(weapon->ammoCapacity / 4, weapon->ammoCapacity);
      }
    }
  }

  if(d.isStackable) {
    item->nrItems = Rnd::range(1, d.maxStackSizeAtSpawn);
  }
}

Item* mkItemOnMap(const ItemId itemId, const Pos& pos) {
  Item* item = mk(itemId);
  setItemRandomizedProperties(item);
  ItemDrop::dropItemOnMap(pos, *item);
  return item;
}

Item* copyItem(Item* oldItem) {
  Item* newItem = mk(oldItem->getData().id);
  *newItem = *oldItem;
  return newItem;
}

Item* mkRandomScrollOrPotion(const bool ALLOW_SCROLLS,
                             const bool ALLOW_POTIONS) {
  vector<ItemId> itemBucket;

  for(int i = 1; i < int(ItemId::END); ++i) {
    const ItemDataT* const d = ItemData::data[i];
    if(
      !d->isIntrinsic &&
      ((d->isScroll && ALLOW_SCROLLS) ||
       (d->isPotion && ALLOW_POTIONS))) {
      itemBucket.push_back(static_cast<ItemId>(i));
    }
  }

  if(!itemBucket.empty()) {
    const int ELEMENT = Rnd::range(0, itemBucket.size() - 1);
    return mk(itemBucket.at(ELEMENT));
  }

  return nullptr;
}

} //ItemFactory
