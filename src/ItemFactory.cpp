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

Item* spawnItem(const ItemId itemId, const int NR_ITEMS) {
  Item* r = NULL;

  ItemDataT* const d = ItemData::data[int(itemId)];

  ItemDataT* ammoD = NULL;

  if(d->rangedAmmoTypeUsed != ItemId::empty) {
    ammoD = ItemData::data[int(d->rangedAmmoTypeUsed)];
  }

  switch(itemId) {
    case ItemId::trapezohedron:       r = new Item(d);                  break;

    case ItemId::rock:                r = new Weapon(d, ammoD);         break;
    case ItemId::throwingKnife:       r = new Weapon(d, ammoD);         break;
    case ItemId::ironSpike:           r = new Weapon(d, ammoD);         break;
    case ItemId::dagger:              r = new Weapon(d, ammoD);         break;
    case ItemId::hatchet:             r = new Weapon(d, ammoD);         break;
    case ItemId::club:                r = new Weapon(d, ammoD);         break;
    case ItemId::hammer:              r = new Weapon(d, ammoD);         break;
    case ItemId::machete:             r = new Weapon(d, ammoD);         break;
    case ItemId::axe:                 r = new Weapon(d, ammoD);         break;
    case ItemId::pitchFork:           r = new Weapon(d, ammoD);         break;
    case ItemId::sledgeHammer:        r = new Weapon(d, ammoD);         break;

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

    case ItemId::playerKick:          r = new Weapon(d, ammoD);         break;
    case ItemId::playerStomp:         r = new Weapon(d, ammoD);         break;
    case ItemId::playerPunch:         r = new Weapon(d, ammoD);         break;

    case ItemId::armorFlackJacket:    r = new Armor(d);                 break;
    case ItemId::armorLeatherJacket:  r = new Armor(d);                 break;
    case ItemId::armorIronSuit:       r = new Armor(d);                 break;
    case ItemId::armorAsbSuit:        r = new ArmorAsbSuit(d);          break;
    case ItemId::armorHeavyCoat:      r = new ArmorHeavyCoat(d);        break;

    case ItemId::zombieAxe:           r = new Weapon(d, ammoD);         break;
    case ItemId::zombieClaw:          r = new Weapon(d, ammoD);         break;
    case ItemId::zombieClawDiseased:  r = new Weapon(d, ammoD);         break;
    case ItemId::bloatedZombiePunch:  r = new Weapon(d, ammoD);         break;
    case ItemId::bloatedZombieSpit:   r = new Weapon(d, ammoD);         break;
    case ItemId::ratBite:             r = new Weapon(d, ammoD);         break;
    case ItemId::ratBiteDiseased:     r = new Weapon(d, ammoD);         break;
    case ItemId::ratThingBite:        r = new Weapon(d, ammoD);         break;
    case ItemId::wormMassBite:        r = new Weapon(d, ammoD);         break;
    case ItemId::greenSpiderBite:     r = new Weapon(d, ammoD);         break;
    case ItemId::whiteSpiderBite:     r = new Weapon(d, ammoD);         break;
    case ItemId::redSpiderBite:       r = new Weapon(d, ammoD);         break;
    case ItemId::shadowSpiderBite:    r = new Weapon(d, ammoD);         break;
    case ItemId::lengSpiderBite:      r = new Weapon(d, ammoD);         break;
    case ItemId::fireHoundBreath:     r = new Weapon(d, ammoD);         break;
    case ItemId::frostHoundBreath:    r = new Weapon(d, ammoD);         break;
    case ItemId::fireHoundBite:       r = new Weapon(d, ammoD);         break;
    case ItemId::frostHoundBite:      r = new Weapon(d, ammoD);         break;
    case ItemId::zuulBite:            r = new Weapon(d, ammoD);         break;
    case ItemId::giantBatBite:        r = new Weapon(d, ammoD);         break;
    case ItemId::wolfBite:            r = new Weapon(d, ammoD);         break;
    case ItemId::ghostClaw:           r = new Weapon(d, ammoD);         break;
    case ItemId::phantasmSickle:      r = new Weapon(d, ammoD);         break;
    case ItemId::wraithClaw:          r = new Weapon(d, ammoD);         break;
    case ItemId::polypTentacle:       r = new Weapon(d, ammoD);         break;
    case ItemId::miGoElectricGun:     r = new Weapon(d, ammoD);         break;
    case ItemId::ghoulClaw:           r = new Weapon(d, ammoD);         break;
    case ItemId::shadowClaw:          r = new Weapon(d, ammoD);         break;
    case ItemId::byakheeClaw:         r = new Weapon(d, ammoD);         break;
    case ItemId::giantMantisClaw:     r = new Weapon(d, ammoD);         break;
    case ItemId::giantLocustBite:     r = new Weapon(d, ammoD);         break;
    case ItemId::mummyMaul:           r = new Weapon(d, ammoD);         break;
    case ItemId::deepOneJavelinAtt:   r = new Weapon(d, ammoD);         break;
    case ItemId::deepOneSpearAtt:     r = new Weapon(d, ammoD);         break;
    case ItemId::oozeBlackSpewPus:    r = new Weapon(d, ammoD);         break;
    case ItemId::oozeClearSpewPus:    r = new Weapon(d, ammoD);         break;
    case ItemId::oozePutridSpewPus:   r = new Weapon(d, ammoD);         break;
    case ItemId::oozePoisonSpewPus:   r = new Weapon(d, ammoD);         break;
    case ItemId::colourOOSpaceTouch:  r = new Weapon(d, ammoD);         break;
    case ItemId::chthonianBite:       r = new Weapon(d, ammoD);         break;
    case ItemId::huntingHorrorBite:   r = new Weapon(d, ammoD);         break;
    case ItemId::dustVortexEngulf:    r = new Weapon(d, ammoD);         break;
    case ItemId::fireVortexEngulf:    r = new Weapon(d, ammoD);         break;
    case ItemId::frostVortexEngulf:   r = new Weapon(d, ammoD);         break;

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
    case ItemId::endOfItemIds: return NULL;
  }

  if(r->getData().isStackable == false && NR_ITEMS != 1) {
    trace << "[WARNING] Specified " + toStr(NR_ITEMS) + " nr items";
    trace << " for non-stackable item, in ItemFactory::spawnItem()" << endl;
  } else {
    r->nrItems = NR_ITEMS;
  }

  return r;
}

void setItemRandomizedProperties(Item* item) {
  const ItemDataT& d = item->getData();

  //If it is a pure melee weapon, it may get a plus
  if(d.isMeleeWeapon && d.isRangedWeapon == false) {
    dynamic_cast<Weapon*>(item)->setRandomMeleePlus();
  }

  //If firearm, spawn with random amount of ammo
  if(d.isRangedWeapon && d.rangedHasInfiniteAmmo == false) {
    Weapon* const weapon = dynamic_cast<Weapon*>(item);
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

Item* spawnItemOnMap(const ItemId itemId, const Pos& pos) {
  Item* item = spawnItem(itemId);
  setItemRandomizedProperties(item);
  ItemDrop::dropItemOnMap(pos, *item);
  return item;
}

Item* copyItem(Item* oldItem) {
  Item* newItem = spawnItem(oldItem->getData().id);
  *newItem = *oldItem;
  return newItem;
}

Item* spawnRandomScrollOrPotion(const bool ALLOW_SCROLLS,
                                const bool ALLOW_POTIONS) {
  vector<ItemId> itemBucket;

  for(int i = 1; i < int(ItemId::endOfItemIds); i++) {
    const ItemDataT* const d = ItemData::data[i];
    if(
      d->isIntrinsic == false &&
      ((d->isScroll && ALLOW_SCROLLS) ||
       (d->isPotion && ALLOW_POTIONS))) {
      itemBucket.push_back(static_cast<ItemId>(i));
    }
  }

  if(itemBucket.empty() == false) {
    const int ELEMENT = Rnd::range(0, itemBucket.size() - 1);
    return spawnItem(itemBucket.at(ELEMENT));
  }

  return NULL;
}

} //ItemFactory
