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

Item* ItemFactory::spawnItem(const ItemId_t itemId, const int NR_ITEMS) {
  Item* item = NULL;

  ItemDefinition* const d = eng->itemData->itemDefinitions[itemId];

  ItemDefinition* ammoD = NULL;

  if(d->rangedAmmoTypeUsed != item_empty) {
    ammoD = eng->itemData->itemDefinitions[d->rangedAmmoTypeUsed];
  }

  switch(itemId) {
//    case item_crowbar:                  item = new Item(d);                     break;
//    case item_lockpick:                 item = new Item(d);                     break;
    case item_trapezohedron:            item = new Item(d);                     break;
    case item_rock:                     item = new Weapon(d, ammoD);            break;
    case item_throwingKnife:            item = new Weapon(d, ammoD);            break;
    case item_ironSpike:                item = new Weapon(d, ammoD);            break;
    case item_dagger:                   item = new Weapon(d, ammoD);            break;
    case item_hatchet:                  item = new Weapon(d, ammoD);            break;
    case item_club:                     item = new Weapon(d, ammoD);            break;
    case item_hammer:                   item = new Weapon(d, ammoD);            break;
    case item_machete:                  item = new Weapon(d, ammoD);            break;
    case item_axe:                      item = new Weapon(d, ammoD);            break;
    case item_pitchFork:                item = new Weapon(d, ammoD);            break;
    case item_sledgeHammer:             item = new Weapon(d, ammoD);            break;
    case item_dynamite:                 item = new Dynamite(d);                 break;
    case item_flare:                    item = new Flare(d);                    break;
    case item_molotov:                  item = new Molotov(d);                  break;

    case item_sawedOff:                 item = new SawedOff(d, ammoD);          break;
    case item_pumpShotgun:              item = new PumpShotgun(d, ammoD);       break;
    case item_shotgunShell:             item = new ItemAmmo(d);                 break;
    case item_machineGun:               item = new MachineGun(d, ammoD);        break;
    case item_drumOfBullets:            item = new ItemAmmoClip(d);             break;
    case item_pistol:                   item = new Pistol(d, ammoD);            break;
    case item_flareGun:                 item = new FlareGun(d, ammoD);          break;
    case item_pistolClip:               item = new ItemAmmoClip(d);             break;
    case item_incinerator:              item = new Incinerator(d, ammoD);       break;
    case item_teslaCanon:               item = new TeslaCanon(d, ammoD);        break;
    case item_spikeGun:                 item = new SpikeGun(d, ammoD);          break;
    case item_teslaCanister:            item = new ItemAmmoClip(d);             break;
    case item_napalmCartridge:          item = new ItemAmmoClip(d);             break;

    case item_playerKick:               item = new Weapon(d, ammoD);            break;
    case item_playerStomp:              item = new Weapon(d, ammoD);            break;
    case item_playerPunch:              item = new Weapon(d, ammoD);            break;

    case item_armorFlackJacket:         item = new Armor(d, eng);               break;
    case item_armorLeatherJacket:       item = new Armor(d, eng);               break;
    case item_armorIronSuit:            item = new Armor(d, eng);               break;

    case item_zombieAxe:                item = new Weapon(d, ammoD);            break;
    case item_zombieClaw:               item = new Weapon(d, ammoD);            break;
    case item_zombieClawDiseased:       item = new Weapon(d, ammoD);            break;
    case item_bloatedZombiePunch:       item = new Weapon(d, ammoD);            break;
    case item_bloatedZombieSpit:        item = new Weapon(d, ammoD);            break;
    case item_ratBite:                  item = new Weapon(d, ammoD);            break;
    case item_ratBiteDiseased:          item = new Weapon(d, ammoD);            break;
    case item_ratThingBite:             item = new Weapon(d, ammoD);            break;
    case item_wormMassBite:             item = new Weapon(d, ammoD);            break;
    case item_greenSpiderBite:          item = new Weapon(d, ammoD);            break;
    case item_whiteSpiderBite:          item = new Weapon(d, ammoD);            break;
    case item_redSpiderBite:            item = new Weapon(d, ammoD);            break;
    case item_shadowSpiderBite:         item = new Weapon(d, ammoD);            break;
    case item_lengSpiderBite:           item = new Weapon(d, ammoD);            break;
    case item_hellHoundFireBreath:      item = new Weapon(d, ammoD);            break;
    case item_hellHoundBite:            item = new Weapon(d, ammoD);            break;
    case item_giantBatBite:             item = new Weapon(d, ammoD);            break;
    case item_wolfBite:                 item = new Weapon(d, ammoD);            break;
    case item_ghostClaw:                item = new Weapon(d, ammoD);            break;
    case item_phantasmSickle:           item = new Weapon(d, ammoD);            break;
    case item_wraithClaw:               item = new Weapon(d, ammoD);            break;
    case item_miGoElectricGun:          item = new Weapon(d, ammoD);            break;
    case item_ghoulClaw:                item = new Weapon(d, ammoD);            break;
    case item_shadowClaw:               item = new Weapon(d, ammoD);            break;
    case item_byakheeClaw:              item = new Weapon(d, ammoD);            break;
    case item_giantMantisClaw:          item = new Weapon(d, ammoD);            break;
    case item_giantLocustBite:          item = new Weapon(d, ammoD);            break;
    case item_mummyMaul:                item = new Weapon(d, ammoD);            break;
    case item_deepOneJavelinAttack:     item = new Weapon(d, ammoD);            break;
    case item_deepOneSpearAttack:       item = new Weapon(d, ammoD);            break;
    case item_oozeBlackSpewPus:         item = new Weapon(d, ammoD);            break;
    case item_oozeClearSpewPus:         item = new Weapon(d, ammoD);            break;
    case item_oozePutridSpewPus:        item = new Weapon(d, ammoD);            break;
    case item_oozePoisonSpewPus:        item = new Weapon(d, ammoD);            break;
    case item_colourOutOfSpaceTouch:    item = new Weapon(d, ammoD);            break;
    case item_huntingHorrorBite:        item = new Weapon(d, ammoD);            break;
    case item_dustVortexEngulf:         item = new Weapon(d, ammoD);            break;
    case item_fireVortexEngulf:         item = new Weapon(d, ammoD);            break;

    case item_scrollOfMayhem:           item = new ScrollOfMayhem(d);           break;
    case item_scrollOfTeleportation:    item = new ScrollOfTeleportation(d);    break;
    case item_scrollOfDescent:          item = new ScrollOfDescent(d);          break;
    case item_scrollOfPestilence:       item = new ScrollOfPestilence(d);       break;
    case item_scrollOfEnfeebleEnemies:  item = new ScrollOfEnfeebleEnemies(d);  break;
    case item_scrollOfDetectItems:      item = new ScrollOfDetectItems(d);      break;
    case item_scrollOfDetectTraps:      item = new ScrollOfDetectTraps(d);      break;
    case item_scrollOfIdentify:         item = new ScrollOfIdentify(d);         break;
    case item_scrollOfBlessing:         item = new ScrollOfBlessing(d);         break;
    case item_scrollOfClairvoyance:     item = new ScrollOfClairvoyance(d);     break;
    case item_scrollOfAzathothsBlast:   item = new ScrollOfAzathothsBlast(d);   break;
    case item_scrollOfOpening:          item = new ScrollOfOpening(d);          break;
    case item_thaumaturgicAlteration:   item = new ThaumaturgicAlteration(d);   break;

    case item_potionOfHealing:          item = new PotionOfHealing(d);          break;
    case item_potionOfBlindness:        item = new PotionOfBlindness(d);        break;
//    case item_potionOfCorruption:       item = new PotionOfCorruption(d);       break;
    case item_potionOfTheCobra:         item = new PotionOfTheCobra(d);         break;
    case item_potionOfFortitude:        item = new PotionOfFortitude(d);        break;
    case item_potionOfToughness:        item = new PotionOfToughness(d);        break;
    case item_potionOfParalyzation:     item = new PotionOfParalyzation(d);     break;
    case item_potionOfConfusion:        item = new PotionOfConfusion(d);        break;
    case item_potionOfSorcery:          item = new PotionOfSorcery(d);          break;
    case item_potionOfPoison:           item = new PotionOfPoison(d);           break;
    case item_potionOfKnowledge:        item = new PotionOfKnowledge(d);        break;

    case item_deviceSentry:             item = new DeviceSentry(d);             break;
    case item_deviceRepeller:           item = new DeviceRepeller(d);           break;
    case item_deviceRejuvenator:        item = new DeviceRejuvenator(d);        break;
    case item_deviceTranslocator:       item = new DeviceTranslocator(d);       break;
    case item_deviceElectricLantern:    item = new DeviceElectricLantern(d);    break;

    default: item = new Item(d); break;
  }

  if(item->getDef().isStackable == false && NR_ITEMS != 1) {
    tracer << "[WARNING] Specified " + intToString(NR_ITEMS) + " nr items";
    tracer << " for non-stackable item, in ItemFactory::spawnItem()" << endl;
  } else {
    item->numberOfItems = NR_ITEMS;
  }

  return item;
}

void ItemFactory::setItemRandomizedProperties(Item* item) {
  const ItemDefinition& d = item->getDef();

  //If it is a pure melee weapon, it may get a plus
  if(d.isMeleeWeapon && d.isRangedWeapon == false) {
    dynamic_cast<Weapon*>(item)->setRandomMeleePlus(eng);
  }

  //If firearm, spawn with random amount of ammo
  if(d.isRangedWeapon && d.rangedHasInfiniteAmmo == false) {
    Weapon* const weapon = dynamic_cast<Weapon*>(item);
    if(weapon->ammoCapacity == 1) {
      weapon->ammoLoaded = eng->dice.coinToss() ? 1 : 0;
    } else {
      if(d.isMachineGun) {
        const int CAP = weapon->ammoCapacity;
        const int MIN = CAP / 2;
        const int CAP_SCALED = CAP / NUMBER_OF_MACHINEGUN_PROJECTILES_PER_BURST;
        const int MIN_SCALED = MIN / NUMBER_OF_MACHINEGUN_PROJECTILES_PER_BURST;
        weapon->ammoLoaded = eng->dice.getInRange(MIN_SCALED, CAP_SCALED) * NUMBER_OF_MACHINEGUN_PROJECTILES_PER_BURST;
      } else {
        weapon->ammoLoaded = eng->dice.getInRange(weapon->ammoCapacity / 4, weapon->ammoCapacity);
      }
    }
  }

  if(d.isStackable) {
    item->numberOfItems = eng->dice(1, d.maxStackSizeAtSpawn);
  }
}

Item* ItemFactory::spawnItemOnMap(const ItemId_t itemId, const coord& pos) {
  Item* item = spawnItem(itemId);
  setItemRandomizedProperties(item);
  eng->itemDrop->dropItemOnMap(pos, *item);
  return item;
}

Item* ItemFactory::copyItem(Item* oldItem) {
  Item* newItem = spawnItem(oldItem->getDef().id);
  *newItem = *oldItem;
  return newItem;
}

Item* ItemFactory::spawnRandomScrollOrPotion(const bool ALLOW_SCROLLS, const bool ALLOW_POTIONS) {
  vector<ItemId_t> itemCandidates;

  for(unsigned int i = 1; i < endOfItemIds; i++) {
    const ItemDefinition* const d = eng->itemData->itemDefinitions[i];
    if(d->isIntrinsic == false && ((d->isReadable && ALLOW_SCROLLS) || (d->isQuaffable && ALLOW_POTIONS))) {
      itemCandidates.push_back(static_cast<ItemId_t>(i));
    }
  }

  if(itemCandidates.size() > 0) {
    const unsigned int ELEMENT = eng->dice(1, itemCandidates.size()) - 1;
    return spawnItem(itemCandidates.at(ELEMENT));
  }

  return NULL;
}


