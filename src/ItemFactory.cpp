#include "ItemFactory.h"

#include "Engine.h"

#include "ItemWeapon.h"
#include "ItemAmmoClip.h"
#include "ItemUsable.h"
#include "ItemExplosive.h"
#include "ItemArmor.h"
#include "ItemScroll.h"
#include "ItemPotion.h"
#include "ItemDrop.h"

//Private
Item* ItemFactory::spawnItem(ItemDevNames_t devName) {
	Item* item = NULL;

	ItemDefinition* const d = eng->itemData->itemDefinitions[devName];

	ItemDefinition* ammoD = NULL;

	if(d->rangedAmmoTypeUsed != item_empty)
		ammoD = eng->itemData->itemDefinitions[d->rangedAmmoTypeUsed];

	switch(devName) {
	case item_trapezohedron:
		item = new Item(d);
		break;
	case item_ironSpike:
		item = new Item(d);
		break;
	case item_rock:
		item = new Weapon(d, ammoD);
		break;
	case item_throwingKnife:
		item = new Weapon(d, ammoD);
		break;
	case item_dagger:
		item = new Weapon(d, ammoD);
		break;
	case item_hatchet:
		item = new Weapon(d, ammoD);
		break;
	case item_club:
		item = new Weapon(d, ammoD);
		break;
	case item_hammer:
		item = new Weapon(d, ammoD);
		break;
	case item_machete:
		item = new Weapon(d, ammoD);
		break;
	case item_axe:
		item = new Weapon(d, ammoD);
		break;
	case item_pitchFork:
		item = new Weapon(d, ammoD);
		break;
	case item_sledgeHammer:
		item = new Weapon(d, ammoD);
		break;

	case item_dynamite:
		item = new Dynamite(d);
		break;
	case item_flare:
		item = new Flare(d);
		break;
	case item_molotov:
		item = new Molotov(d);
		break;

	case item_sawedOff:
		item = new SawedOff(d, ammoD);
		break;
	case item_pumpShotgun:
		item = new PumpShotgun(d, ammoD);
		break;
	case item_machineGun:
		item = new MachineGun(d, ammoD);
		break;
	case item_drumOfBullets:
		item = new ItemAmmoClip(d);
		break;
	case item_pistol:
		item = new Pistol(d, ammoD);
		break;
	case item_flareGun:
		item = new FlareGun(d, ammoD);
		break;
	case item_pistolClip:
		item = new ItemAmmoClip(d);
		break;
	case item_incinerator:
		item = new Incinerator(d, ammoD);
		break;
	case item_teslaCanon:
		item = new TeslaCanon(d, ammoD);
		break;
	case item_spikeGun:
		item = new SpikeGun(d, ammoD);
		break;
	case item_teslaCanister:
		item = new ItemAmmoClip(d);
		break;

	case item_playerKick:
		item = new Weapon(d, ammoD);
		break;
	case item_playerStomp:
		item = new Weapon(d, ammoD);
		break;

	case item_armorFlackJacket:
		item = new Armor(d, eng);
		break;
	case item_armorLeatherJacket:
		item = new Armor(d, eng);
		break;
	case item_armorIronSuit:
		item = new Armor(d, eng);
		break;
//    case item_armorAsbestosSuit:
//        item = new Armor(d, eng);
//        break;

	case item_zombieAxe:
		item = new Weapon(d, ammoD);
		break;
	case item_zombieClaw:
		item = new Weapon(d, ammoD);
		break;
	case item_zombieClawDiseased:
		item = new Weapon(d, ammoD);
		break;
	case item_bloatedZombiePunch:
		item = new Weapon(d, ammoD);
		break;
	case item_bloatedZombieSpit:
		item = new Weapon(d, ammoD);
		break;
	case item_ratBite:
		item = new Weapon(d, ammoD);
		break;
	case item_ratBiteDiseased:
		item = new Weapon(d, ammoD);
		break;
	case item_ratThingBite:
		item = new Weapon(d, ammoD);
		break;
	case item_wormMassBite:
		item = new Weapon(d, ammoD);
		break;
	case item_greenSpiderBite:
		item = new Weapon(d, ammoD);
		break;
	case item_whiteSpiderBite:
		item = new Weapon(d, ammoD);
		break;
	case item_blackSpiderBite:
		item = new Weapon(d, ammoD);
		break;
	case item_lengSpiderBite:
		item = new Weapon(d, ammoD);
		break;
	case item_hellHoundFireBreath:
		item = new Weapon(d, ammoD);
		break;
	case item_hellHoundBite:
		item = new Weapon(d, ammoD);
		break;
	case item_giantBatBite:
		item = new Weapon(d, ammoD);
		break;
	case item_wolfBite:
		item = new Weapon(d, ammoD);
		break;
	case item_ghostClaw:
		item = new Weapon(d, ammoD);
		break;
	case item_phantasmSickle:
		item = new Weapon(d, ammoD);
		break;
	case item_wraithClaw:
		item = new Weapon(d, ammoD);
		break;
	case item_miGoElectricGun:
		item = new Weapon(d, ammoD);
		break;
	case item_ghoulClaw:
		item = new Weapon(d, ammoD);
		break;
	case item_shadowClaw:
		item = new Weapon(d, ammoD);
		break;
	case item_byakheeClaw:
		item = new Weapon(d, ammoD);
		break;
	case item_mummyMaul:
		item = new Weapon(d, ammoD);
		break;
	case item_deepOneJavelinAttack:
		item = new Weapon(d, ammoD);
		break;
	case item_deepOneSpearAttack:
		item = new Weapon(d, ammoD);
		break;
	case item_oozeGraySpewPus:
		item = new Weapon(d, ammoD);
		break;
	case item_oozeClearSpewPus:
		item = new Weapon(d, ammoD);
		break;
	case item_oozePutridSpewPus:
		item = new Weapon(d, ammoD);
		break;
	case item_huntingHorrorBite:
		item = new Weapon(d, ammoD);
		break;
	case item_fireVampireTouch:
		item = new Weapon(d, ammoD);
		break;

	case item_scrollOfMayhem:
		item = new ScrollOfMayhem(d);
		break;
	case item_scrollOfTeleportation:
		item = new ScrollOfTeleportation(d);
		break;
	case item_scrollOfDeepDescent:
		item = new ScrollOfDeepDescent(d);
		break;
	case item_scrollOfPestilence:
		item = new ScrollOfPestilence(d);
		break;
	case item_scrollOfConfuseEnemies:
		item = new ScrollOfConfuseEnemies(d);
		break;
	case item_scrollOfParalyzeEnemies:
		item = new ScrollOfParalyzeEnemies(d);
		break;
	case item_scrollOfSlowEnemies:
		item = new ScrollOfSlowEnemies(d);
		break;
	case item_scrollOfDetectItems:
		item = new ScrollOfDetectItems(d);
		break;
	case item_scrollOfDetectTraps:
		item = new ScrollOfDetectTraps(d);
		break;
	case item_scrollOfBlessing:
		item = new ScrollOfBlessing(d);
		break;

	case item_potionOfHealing:
		item = new PotionOfHealing(d);
		break;
	case item_potionOfClairvoyance:
		item = new PotionOfClairvoyance(d);
		break;
	case item_potionOfBlindness:
		item = new PotionOfBlindness(d);
		break;
	case item_potionOfCorruption:
		item = new PotionOfCorruption(d);
		break;
	case item_potionOfReflexes:
		item = new PotionOfReflexes(d);
		break;
	case item_potionOfAiming:
		item = new PotionOfAiming(d);
		break;
	case item_potionOfStealth:
		item = new PotionOfStealth(d);
		break;
	case item_potionOfFortitude:
		item = new PotionOfFortitude(d);
		break;
	case item_potionOfToughness:
		item = new PotionOfToughness(d);
		break;
	case item_potionOfParalyzation:
		item = new PotionOfParalyzation(d);
		break;
	case item_potionOfFear:
		item = new PotionOfFear(d);
		break;
	case item_potionOfConfusion:
		item = new PotionOfConfusion(d);
		break;

	default:
		item = new Item(d);
		break;
	}

	return item;
}

Item* ItemFactory::spawnItemOnMap(ItemDevNames_t devName, const coord pos) {
	Item* item = spawnItem(devName);

	//If it is a pure melee weapon, it may get a plus
	if(item->getInstanceDefinition().isMeleeWeapon == true && item->getInstanceDefinition().isRangedWeapon == false) {
		dynamic_cast<Weapon*>(item)->setRandomMeleePlus(eng);
	}

	if(item->getInstanceDefinition().isStackable == true) {
		item->numberOfItems = eng->dice(1, item->getInstanceDefinition().maxStackSizeAtSpawn);
	}

	eng->itemDrop->dropItemOnMap(pos, &item);

	return item;
}

Item* ItemFactory::copyItem(Item* oldItem) {
	Item* newItem = spawnItem(oldItem->getInstanceDefinition().devName);
	*newItem = *oldItem;
	return newItem;
}

Item* ItemFactory::spawnRandomItemRelatedToSpecialRoom(const SpecialRoom_t roomType) {
	vector<ItemDevNames_t> itemCandidates;

	for(unsigned int i = 1; i < endOfItemDevNames; i++) {
		const ItemDefinition* const d = eng->itemData->itemDefinitions[i];

		bool isNative = false;
		for(unsigned int ii = 0; ii < d->nativeRooms.size(); ii++) {
			if(d->nativeRooms.at(ii) == roomType) {
				isNative = true;
				ii = 999999;
			}
		}

		if(isNative) {
			itemCandidates.push_back(static_cast<ItemDevNames_t>(i));
		}
	}

	if(itemCandidates.size() > 0) {
		const unsigned int ELEMENT = eng->dice.getInRange(0, itemCandidates.size() - 1);
		return spawnItem(itemCandidates.at(ELEMENT));
	}

	return NULL;
}

Item* ItemFactory::spawnRandomScrollOrPotion(const bool ALLOW_SCROLLS, const bool ALLOW_POTIONS) {
	vector<ItemDevNames_t> itemCandidates;

	for(unsigned int i = 1; i < endOfItemDevNames; i++) {
		const ItemDefinition* const d = eng->itemData->itemDefinitions[i];
		if((d->isReadable == true && ALLOW_SCROLLS == true) || (d->isQuaffable == true && ALLOW_POTIONS == true)) {
			itemCandidates.push_back(static_cast<ItemDevNames_t>(i));
		}
	}

	if(itemCandidates.size() > 0) {
		const unsigned int ELEMENT = eng->dice(1, itemCandidates.size()) - 1;
		return spawnItem(itemCandidates.at(ELEMENT));
	}

	return NULL;
}


