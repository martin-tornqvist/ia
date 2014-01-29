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

Item* ItemFactory::spawnItem(const ItemId_t itemId, const int NR_ITEMS) {
  Item* item = NULL;

  ItemData* const d = eng.itemDataHandler->dataList[itemId];

  ItemData* ammoD = NULL;

  if(d->rangedAmmoTypeUsed != item_empty) {
    ammoD = eng.itemDataHandler->dataList[d->rangedAmmoTypeUsed];
  }

  switch(itemId) {
//    case item_crowbar:                  item = new Item(d, eng); break;
//    case item_lockpick:                 item = new Item(d, eng); break;
    case item_trapezohedron:            item = new Item(d, eng); break;
    case item_rock:                     item = new Weapon(d, ammoD, eng); break;
    case item_throwingKnife:            item = new Weapon(d, ammoD, eng); break;
    case item_ironSpike:                item = new Weapon(d, ammoD, eng); break;
    case item_dagger:                   item = new Weapon(d, ammoD, eng); break;
    case item_hatchet:                  item = new Weapon(d, ammoD, eng); break;
    case item_club:                     item = new Weapon(d, ammoD, eng); break;
    case item_hammer:                   item = new Weapon(d, ammoD, eng); break;
    case item_machete:                  item = new Weapon(d, ammoD, eng); break;
    case item_axe:                      item = new Weapon(d, ammoD, eng); break;
    case item_pitchFork:                item = new Weapon(d, ammoD, eng); break;
    case item_sledgeHammer:             item = new Weapon(d, ammoD, eng); break;
    case item_dynamite:                 item = new Dynamite(d, eng); break;
    case item_flare:                    item = new Flare(d, eng); break;
    case item_molotov:                  item = new Molotov(d, eng); break;

    case item_sawedOff:                 item = new SawedOff(d, ammoD, eng); break;
    case item_pumpShotgun:              item = new PumpShotgun(d, ammoD, eng); break;
    case item_shotgunShell:             item = new ItemAmmo(d, eng); break;
    case item_machineGun:               item = new MachineGun(d, ammoD, eng); break;
    case item_drumOfBullets:            item = new ItemAmmoClip(d, eng); break;
    case item_pistol:                   item = new Pistol(d, ammoD, eng); break;
    case item_flareGun:                 item = new FlareGun(d, ammoD, eng); break;
    case item_pistolClip:               item = new ItemAmmoClip(d, eng); break;
    case item_incinerator:              item = new Incinerator(d, ammoD, eng); break;
    case item_teslaCannon:              item = new TeslaCannon(d, ammoD, eng); break;
    case item_spikeGun:                 item = new SpikeGun(d, ammoD, eng); break;
    case item_teslaCanister:            item = new ItemAmmoClip(d, eng); break;
    case item_napalmCartridge:          item = new ItemAmmoClip(d, eng); break;

    case item_playerKick:               item = new Weapon(d, ammoD, eng); break;
    case item_playerStomp:              item = new Weapon(d, ammoD, eng); break;
    case item_playerPunch:              item = new Weapon(d, ammoD, eng); break;

    case item_armorFlackJacket:         item = new Armor(d, eng); break;
    case item_armorLeatherJacket:       item = new Armor(d, eng); break;
    case item_armorIronSuit:            item = new Armor(d, eng); break;
    case item_armorAsbestosSuit:        item = new ArmorAsbestosSuit(d, eng); break;
    case item_armorHeavyCoat:           item = new ArmorHeavyCoat(d, eng); break;

    case item_zombieAxe:                item = new Weapon(d, ammoD, eng); break;
    case item_zombieClaw:               item = new Weapon(d, ammoD, eng); break;
    case item_zombieClawDiseased:       item = new Weapon(d, ammoD, eng); break;
    case item_bloatedZombiePunch:       item = new Weapon(d, ammoD, eng); break;
    case item_bloatedZombieSpit:        item = new Weapon(d, ammoD, eng); break;
    case item_ratBite:                  item = new Weapon(d, ammoD, eng); break;
    case item_ratBiteDiseased:          item = new Weapon(d, ammoD, eng); break;
    case item_ratThingBite:             item = new Weapon(d, ammoD, eng); break;
    case item_wormMassBite:             item = new Weapon(d, ammoD, eng); break;
    case item_greenSpiderBite:          item = new Weapon(d, ammoD, eng); break;
    case item_whiteSpiderBite:          item = new Weapon(d, ammoD, eng); break;
    case item_redSpiderBite:            item = new Weapon(d, ammoD, eng); break;
    case item_shadowSpiderBite:         item = new Weapon(d, ammoD, eng); break;
    case item_lengSpiderBite:           item = new Weapon(d, ammoD, eng); break;
    case item_fireHoundBreath:          item = new Weapon(d, ammoD, eng); break;
    case item_frostHoundBreath:         item = new Weapon(d, ammoD, eng); break;
    case item_fireHoundBite:            item = new Weapon(d, ammoD, eng); break;
    case item_frostHoundBite:           item = new Weapon(d, ammoD, eng); break;
    case item_zuulBite:                 item = new Weapon(d, ammoD, eng); break;
    case item_giantBatBite:             item = new Weapon(d, ammoD, eng); break;
    case item_wolfBite:                 item = new Weapon(d, ammoD, eng); break;
    case item_ghostClaw:                item = new Weapon(d, ammoD, eng); break;
    case item_phantasmSickle:           item = new Weapon(d, ammoD, eng); break;
    case item_wraithClaw:               item = new Weapon(d, ammoD, eng); break;
    case item_polypTentacle:            item = new Weapon(d, ammoD, eng); break;
    case item_miGoElectricGun:          item = new Weapon(d, ammoD, eng); break;
    case item_ghoulClaw:                item = new Weapon(d, ammoD, eng); break;
    case item_shadowClaw:               item = new Weapon(d, ammoD, eng); break;
    case item_byakheeClaw:              item = new Weapon(d, ammoD, eng); break;
    case item_giantMantisClaw:          item = new Weapon(d, ammoD, eng); break;
    case item_giantLocustBite:          item = new Weapon(d, ammoD, eng); break;
    case item_mummyMaul:                item = new Weapon(d, ammoD, eng); break;
    case item_deepOneJavelinAttack:     item = new Weapon(d, ammoD, eng); break;
    case item_deepOneSpearAttack:       item = new Weapon(d, ammoD, eng); break;
    case item_oozeBlackSpewPus:         item = new Weapon(d, ammoD, eng); break;
    case item_oozeClearSpewPus:         item = new Weapon(d, ammoD, eng); break;
    case item_oozePutridSpewPus:        item = new Weapon(d, ammoD, eng); break;
    case item_oozePoisonSpewPus:        item = new Weapon(d, ammoD, eng); break;
    case item_colourOutOfSpaceTouch:    item = new Weapon(d, ammoD, eng); break;
    case item_huntingHorrorBite:        item = new Weapon(d, ammoD, eng); break;
    case item_dustVortexEngulf:         item = new Weapon(d, ammoD, eng); break;
    case item_fireVortexEngulf:         item = new Weapon(d, ammoD, eng); break;
    case item_frostVortexEngulf:        item = new Weapon(d, ammoD, eng); break;

    case item_scrollOfMayhem:           item = new Scroll(d, eng); break;
    case item_scrollOfTeleportation:    item = new Scroll(d, eng); break;
    case item_scrollOfPestilence:       item = new Scroll(d, eng); break;
    case item_scrollOfEnfeebleEnemies:  item = new Scroll(d, eng); break;
    case item_scrollOfDetectItems:      item = new Scroll(d, eng); break;
    case item_scrollOfDetectTraps:      item = new Scroll(d, eng); break;
    case item_scrollOfBlessing:         item = new Scroll(d, eng); break;
    case item_scrollOfClairvoyance:     item = new Scroll(d, eng); break;
    case item_scrollOfDarkbolt:         item = new Scroll(d, eng); break;
    case item_scrollOfAzathothsWrath:   item = new Scroll(d, eng); break;
    case item_scrollOfOpening:          item = new Scroll(d, eng); break;
    case item_scrollOfSacrificeLife:    item = new Scroll(d, eng); break;
    case item_scrollOfSacrificeSpirit:  item = new Scroll(d, eng); break;
    case item_thaumaturgicAlteration:   item = new Scroll(d, eng); break;

    case item_potionOfHealing:          item = new PotionOfHealing(d, eng); break;
    case item_potionOfSpirit:           item = new PotionOfSpirit(d, eng); break;
    case item_potionOfBlindness:        item = new PotionOfBlindness(d, eng); break;
    case item_potionOfFrenzy:           item = new PotionOfFrenzy(d, eng); break;
    case item_potionOfFortitude:        item = new PotionOfFortitude(d, eng); break;
    case item_potionOfParalyzation:     item = new PotionOfParalyzation(d, eng); break;
    case item_potionOfRElec:            item = new PotionOfRElec(d, eng); break;
    case item_potionOfConfusion:        item = new PotionOfConfusion(d, eng); break;
    case item_potionOfPoison:           item = new PotionOfPoison(d, eng); break;
    case item_potionOfInsight:          item = new PotionOfInsight(d, eng); break;
    case item_potionOfRFire:            item = new PotionOfRFire(d, eng); break;
    case item_potionOfAntidote:         item = new PotionOfAntidote(d, eng); break;
    case item_potionOfDescent:          item = new PotionOfDescent(d, eng); break;

    case item_deviceSentry:             item = new DeviceSentry(d, eng); break;
    case item_deviceRepeller:           item = new DeviceRepeller(d, eng); break;
    case item_deviceRejuvenator:        item = new DeviceRejuvenator(d, eng); break;
    case item_deviceTranslocator:       item = new DeviceTranslocator(d, eng); break;
    case item_deviceElectricLantern:    item = new DeviceElectricLantern(d, eng); break;

    case item_medicalBag:               item = new MedicalBag(d, eng); break;

    case item_empty:
    case endOfItemIds: {} break;
  }

  if(item->getData().isStackable == false && NR_ITEMS != 1) {
    trace << "[WARNING] Specified " + toString(NR_ITEMS) + " nr items";
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
      weapon->nrAmmoLoaded = eng.dice.coinToss() ? 1 : 0;
    } else {
      if(d.isMachineGun) {
        const int CAP = weapon->ammoCapacity;
        const int MIN = CAP / 2;
        const int CAP_SCALED = CAP / NR_MG_PROJECTILES;
        const int MIN_SCALED = MIN / NR_MG_PROJECTILES;
        weapon->nrAmmoLoaded =
          eng.dice.range(MIN_SCALED, CAP_SCALED) *
          NR_MG_PROJECTILES;
      } else {
        weapon->nrAmmoLoaded =
          eng.dice.range(weapon->ammoCapacity / 4, weapon->ammoCapacity);
      }
    }
  }

  if(d.isStackable) {
    item->nrItems = eng.dice(1, d.maxStackSizeAtSpawn);
  }
}

Item* ItemFactory::spawnItemOnMap(const ItemId_t itemId, const Pos& pos) {
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
  vector<ItemId_t> itemCandidates;

  for(unsigned int i = 1; i < endOfItemIds; i++) {
    const ItemData* const d = eng.itemDataHandler->dataList[i];
    if(
      d->isIntrinsic == false &&
      ((d->isScroll && ALLOW_SCROLLS) ||
       (d->isPotion && ALLOW_POTIONS))) {
      itemCandidates.push_back(static_cast<ItemId_t>(i));
    }
  }

  if(itemCandidates.size() > 0) {
    const unsigned int ELEMENT = eng.dice(1, itemCandidates.size()) - 1;
    return spawnItem(itemCandidates.at(ELEMENT));
  }

  return NULL;
}


