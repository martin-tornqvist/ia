#include "ItemFactory.h"

#include "Engine.h"

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

Item* ItemFactory::spawnItem(const ItemId itemId, const int NR_ITEMS) {
  Item* item = NULL;

  ItemData* const d = eng.itemDataHandler->dataList[int(itemId)];

  ItemData* ammoD = NULL;

  if(d->rangedAmmoTypeUsed != ItemId::empty) {
    ammoD = eng.itemDataHandler->dataList[int(d->rangedAmmoTypeUsed)];
  }

  switch(itemId) {
//    case ItemId::crowbar:                  item = new Item(d, eng); break;
//    case ItemId::lockpick:                 item = new Item(d, eng); break;
    case ItemId::trapezohedron:            item = new Item(d, eng); break;
    case ItemId::rock:                     item = new Weapon(d, ammoD, eng); break;
    case ItemId::throwingKnife:            item = new Weapon(d, ammoD, eng); break;
    case ItemId::ironSpike:                item = new Weapon(d, ammoD, eng); break;
    case ItemId::dagger:                   item = new Weapon(d, ammoD, eng); break;
    case ItemId::hatchet:                  item = new Weapon(d, ammoD, eng); break;
    case ItemId::club:                     item = new Weapon(d, ammoD, eng); break;
    case ItemId::hammer:                   item = new Weapon(d, ammoD, eng); break;
    case ItemId::machete:                  item = new Weapon(d, ammoD, eng); break;
    case ItemId::axe:                      item = new Weapon(d, ammoD, eng); break;
    case ItemId::pitchFork:                item = new Weapon(d, ammoD, eng); break;
    case ItemId::sledgeHammer:             item = new Weapon(d, ammoD, eng); break;
    case ItemId::dynamite:                 item = new Dynamite(d, eng); break;
    case ItemId::flare:                    item = new Flare(d, eng); break;
    case ItemId::molotov:                  item = new Molotov(d, eng); break;

    case ItemId::sawedOff:                 item = new SawedOff(d, ammoD, eng); break;
    case ItemId::pumpShotgun:              item = new PumpShotgun(d, ammoD, eng); break;
    case ItemId::shotgunShell:             item = new ItemAmmo(d, eng); break;
    case ItemId::machineGun:               item = new MachineGun(d, ammoD, eng); break;
    case ItemId::drumOfBullets:            item = new ItemAmmoClip(d, eng); break;
    case ItemId::pistol:                   item = new Pistol(d, ammoD, eng); break;
    case ItemId::flareGun:                 item = new FlareGun(d, ammoD, eng); break;
    case ItemId::pistolClip:               item = new ItemAmmoClip(d, eng); break;
    case ItemId::incinerator:              item = new Incinerator(d, ammoD, eng); break;
    case ItemId::teslaCannon:              item = new TeslaCannon(d, ammoD, eng); break;
    case ItemId::spikeGun:                 item = new SpikeGun(d, ammoD, eng); break;
    case ItemId::teslaCanister:            item = new ItemAmmoClip(d, eng); break;
    case ItemId::napalmCartridge:          item = new ItemAmmoClip(d, eng); break;

    case ItemId::playerKick:               item = new Weapon(d, ammoD, eng); break;
    case ItemId::playerStomp:              item = new Weapon(d, ammoD, eng); break;
    case ItemId::playerPunch:              item = new Weapon(d, ammoD, eng); break;

    case ItemId::armorFlackJacket:         item = new Armor(d, eng); break;
    case ItemId::armorLeatherJacket:       item = new Armor(d, eng); break;
    case ItemId::armorIronSuit:            item = new Armor(d, eng); break;
    case ItemId::armorAsbestosSuit:        item = new ArmorAsbestosSuit(d, eng); break;
    case ItemId::armorHeavyCoat:           item = new ArmorHeavyCoat(d, eng); break;

    case ItemId::zombieAxe:                item = new Weapon(d, ammoD, eng); break;
    case ItemId::zombieClaw:               item = new Weapon(d, ammoD, eng); break;
    case ItemId::zombieClawDiseased:       item = new Weapon(d, ammoD, eng); break;
    case ItemId::bloatedZombiePunch:       item = new Weapon(d, ammoD, eng); break;
    case ItemId::bloatedZombieSpit:        item = new Weapon(d, ammoD, eng); break;
    case ItemId::ratBite:                  item = new Weapon(d, ammoD, eng); break;
    case ItemId::ratBiteDiseased:          item = new Weapon(d, ammoD, eng); break;
    case ItemId::ratThingBite:             item = new Weapon(d, ammoD, eng); break;
    case ItemId::wormMassBite:             item = new Weapon(d, ammoD, eng); break;
    case ItemId::greenSpiderBite:          item = new Weapon(d, ammoD, eng); break;
    case ItemId::whiteSpiderBite:          item = new Weapon(d, ammoD, eng); break;
    case ItemId::redSpiderBite:            item = new Weapon(d, ammoD, eng); break;
    case ItemId::shadowSpiderBite:         item = new Weapon(d, ammoD, eng); break;
    case ItemId::lengSpiderBite:           item = new Weapon(d, ammoD, eng); break;
    case ItemId::fireHoundBreath:          item = new Weapon(d, ammoD, eng); break;
    case ItemId::frostHoundBreath:         item = new Weapon(d, ammoD, eng); break;
    case ItemId::fireHoundBite:            item = new Weapon(d, ammoD, eng); break;
    case ItemId::frostHoundBite:           item = new Weapon(d, ammoD, eng); break;
    case ItemId::zuulBite:                 item = new Weapon(d, ammoD, eng); break;
    case ItemId::giantBatBite:             item = new Weapon(d, ammoD, eng); break;
    case ItemId::wolfBite:                 item = new Weapon(d, ammoD, eng); break;
    case ItemId::ghostClaw:                item = new Weapon(d, ammoD, eng); break;
    case ItemId::phantasmSickle:           item = new Weapon(d, ammoD, eng); break;
    case ItemId::wraithClaw:               item = new Weapon(d, ammoD, eng); break;
    case ItemId::polypTentacle:            item = new Weapon(d, ammoD, eng); break;
    case ItemId::miGoElectricGun:          item = new Weapon(d, ammoD, eng); break;
    case ItemId::ghoulClaw:                item = new Weapon(d, ammoD, eng); break;
    case ItemId::shadowClaw:               item = new Weapon(d, ammoD, eng); break;
    case ItemId::byakheeClaw:              item = new Weapon(d, ammoD, eng); break;
    case ItemId::giantMantisClaw:          item = new Weapon(d, ammoD, eng); break;
    case ItemId::giantLocustBite:          item = new Weapon(d, ammoD, eng); break;
    case ItemId::mummyMaul:                item = new Weapon(d, ammoD, eng); break;
    case ItemId::deepOneJavelinAttack:     item = new Weapon(d, ammoD, eng); break;
    case ItemId::deepOneSpearAttack:       item = new Weapon(d, ammoD, eng); break;
    case ItemId::oozeBlackSpewPus:         item = new Weapon(d, ammoD, eng); break;
    case ItemId::oozeClearSpewPus:         item = new Weapon(d, ammoD, eng); break;
    case ItemId::oozePutridSpewPus:        item = new Weapon(d, ammoD, eng); break;
    case ItemId::oozePoisonSpewPus:        item = new Weapon(d, ammoD, eng); break;
    case ItemId::colourOutOfSpaceTouch:    item = new Weapon(d, ammoD, eng); break;
    case ItemId::chthonianBite:            item = new Weapon(d, ammoD, eng); break;
    case ItemId::huntingHorrorBite:        item = new Weapon(d, ammoD, eng); break;
    case ItemId::dustVortexEngulf:         item = new Weapon(d, ammoD, eng); break;
    case ItemId::fireVortexEngulf:         item = new Weapon(d, ammoD, eng); break;
    case ItemId::frostVortexEngulf:        item = new Weapon(d, ammoD, eng); break;

    case ItemId::scrollOfMayhem:          item = new Scroll(d, eng); break;
    case ItemId::scrollOfTelep:           item = new Scroll(d, eng); break;
    case ItemId::scrollOfPestilence:      item = new Scroll(d, eng); break;
    case ItemId::scrollOfSlowEnemies:     item = new Scroll(d, eng); break;
    case ItemId::scrollOfTerrifyEnemies:  item = new Scroll(d, eng); break;
    case ItemId::scrollOfParalyzeEnemies: item = new Scroll(d, eng); break;
    case ItemId::scrollOfDetItems:        item = new Scroll(d, eng); break;
    case ItemId::scrollOfDetTraps:        item = new Scroll(d, eng); break;
    case ItemId::scrollOfDetMon:          item = new Scroll(d, eng); break;
    case ItemId::scrollOfBless:           item = new Scroll(d, eng); break;
    case ItemId::scrollOfDarkbolt:        item = new Scroll(d, eng); break;
    case ItemId::scrollOfAzathothsWrath:  item = new Scroll(d, eng); break;
    case ItemId::scrollOfOpening:         item = new Scroll(d, eng); break;
    case ItemId::scrollOfSacrLife:        item = new Scroll(d, eng); break;
    case ItemId::scrollOfSacrSpi:         item = new Scroll(d, eng); break;
    case ItemId::scrollOfElemRes:         item = new Scroll(d, eng); break;

    case ItemId::potionOfVitality:        item = new PotionOfVitality(d, eng); break;
    case ItemId::potionOfSpirit:          item = new PotionOfSpirit(d, eng); break;
    case ItemId::potionOfBlindness:       item = new PotionOfBlindness(d, eng); break;
    case ItemId::potionOfFrenzy:          item = new PotionOfFrenzy(d, eng); break;
    case ItemId::potionOfFortitude:       item = new PotionOfFortitude(d, eng); break;
    case ItemId::potionOfParalyze:        item = new PotionOfParalyzation(d, eng); break;
    case ItemId::potionOfRElec:           item = new PotionOfRElec(d, eng); break;
    case ItemId::potionOfConfusion:       item = new PotionOfConfusion(d, eng); break;
    case ItemId::potionOfPoison:          item = new PotionOfPoison(d, eng); break;
    case ItemId::potionOfInsight:         item = new PotionOfInsight(d, eng); break;
    case ItemId::potionOfClairv:          item = new PotionOfClairvoyance(d, eng); break;
    case ItemId::potionOfRFire:           item = new PotionOfRFire(d, eng); break;
    case ItemId::potionOfAntidote:        item = new PotionOfAntidote(d, eng); break;
    case ItemId::potionOfDescent:         item = new PotionOfDescent(d, eng); break;

    case ItemId::deviceSentry:            item = new DeviceSentry(d, eng); break;
    case ItemId::deviceRepeller:          item = new DeviceRepeller(d, eng); break;
    case ItemId::deviceRejuvenator:       item = new DeviceRejuvenator(d, eng); break;
    case ItemId::deviceTranslocator:      item = new DeviceTranslocator(d, eng); break;
    case ItemId::electricLantern:         item = new DeviceLantern(d, eng); break;

    case ItemId::medicalBag:               item = new MedicalBag(d, eng); break;

    case ItemId::empty:
    case ItemId::endOfItemIds: return NULL;
  }

  if(item->getData().isStackable == false && NR_ITEMS != 1) {
    trace << "[WARNING] Specified " + toStr(NR_ITEMS) + " nr items";
    trace << " for non-stackable item, in ItemFactory::spawnItem()" << endl;
  } else {
    item->nrItems = NR_ITEMS;
  }

  return item;
}

void ItemFactory::setItemRandomizedProperties(Item* item) {
  const ItemData& d = item->getData();

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

Item* ItemFactory::spawnItemOnMap(const ItemId itemId, const Pos& pos) {
  Item* item = spawnItem(itemId);
  setItemRandomizedProperties(item);
  eng.itemDrop->dropItemOnMap(pos, *item);
  return item;
}

Item* ItemFactory::copyItem(Item* oldItem) {
  Item* newItem = spawnItem(oldItem->getData().id);
  *newItem = *oldItem;
  return newItem;
}

Item* ItemFactory::spawnRandomScrollOrPotion(const bool ALLOW_SCROLLS,
    const bool ALLOW_POTIONS) {
  vector<ItemId> itemCandidates;

  for(int i = 1; i < int(ItemId::endOfItemIds); i++) {
    const ItemData* const d = eng.itemDataHandler->dataList[i];
    if(
      d->isIntrinsic == false &&
      ((d->isScroll && ALLOW_SCROLLS) ||
       (d->isPotion && ALLOW_POTIONS))) {
      itemCandidates.push_back(static_cast<ItemId>(i));
    }
  }

  if(itemCandidates.size() > 0) {
    const int ELEMENT = Rnd::range(0, itemCandidates.size() - 1);
    return spawnItem(itemCandidates.at(ELEMENT));
  }

  return NULL;
}


