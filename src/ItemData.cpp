#include "ItemData.h"

#include <iostream>

#include "Engine.h"

#include "Colors.h"
#include "Item.h"
#include "ItemWeapon.h"
#include "ItemAmmoClip.h"
#include "ConstTypes.h"
#include "ActorData.h"
#include "ItemArmor.h"
#include "ItemScroll.h"
#include "ItemPotion.h"

using namespace std;

//------------------------------- ITEM ARCHETYPES (DEFAULTS)
void ItemData::resetDef(ItemDefinition* const d, ItemDefArchetypes_t const archetype) const {
	switch(archetype) {
	case itemDef_general: {
		d->itemWeight = itemWeight_medium;
		d->spawnStandardMinDLVL = 1;
		d->spawnStandardMaxDLVL = 999999;
		d->maxStackSizeAtSpawn = 1;
		d->chanceToIncludeInSpawnList = 100;
		d->isStackable = true;
		d->isIdentified = true;
		d->glyph = 'X';
		d->color = clrWhite;
		d->tile = tile_empty;
		d->isUsable = false;
		d->isReadable = false;
		d->isScroll = false;
		d->isScrollLearned = false;
		d->isScrollLearnable = false;
		d->isQuaffable = false;
		d->isEatable = false;
		d->nutritionValue = NutritionValue_none;
		d->isArmor = false;
		d->isCloak = false;
		d->isRing = false;
		d->isAmulet = false;
		d->isIntrinsicWeapon = false;
		d->isMeleeWeapon = false;
		d->isRangedWeapon = false;
		d->isMissileWeapon = false;
		d->isShotgun = false;
		d->isMachineGun = false;
		d->isAmmo = false;
		d->isAmmoClip = false;
		d->ammoContainedInClip = 0;
		d->meleeBaseAttackSkill = 0;
		d->meleeAbilityUsed = ability_accuracyMelee;
		d->meleeStatusEffect = NULL;
		d->meleeCausesKnockBack = false;
		d->rangedCausesKnockBack = false;
		d->meleeDamageType = damageType_physical;
		d->rangedBaseAttackSkill = 0;
		d->rangedDmgLabelOverRide = "";
		d->rangedAbilityUsed = ability_accuracyRanged;
		d->rangedAmmoTypeUsed = item_empty;
		d->rangedDamageType = damageType_physical;
		d->rangedHasInfiniteAmmo = false;
		d->rangedMissileGlyph = 'X';
		d->rangedMissileTile = tile_projectileStandardFrontSlash;
		d->rangedMissileColor = clrWhite;
		d->rangedMissileLeavesTrail = false;
		d->rangedMissileLeavesSmoke = false;
		d->rangedAudio = audio_none;
		d->meleeAudio = audio_none;
		d->reloadAudio = audio_none;
		d->rangedSoundMessage = "";
		d->rangedSoundStrength = 0;
		d->rangedStatusEffect = NULL;
		d->isExplosive = false;
		d->abilityToIdentify = ability_empty;
		d->identifySkillFactor = 100;
		d->armorData = ArmorData();
		d->nativeRooms.resize(0);
	}
	break;

	case itemDef_meleeWpn: {
		resetDef(d, itemDef_general);
		d->isStackable = false;
		d->itemWeight = itemWeight_medium;
		d->glyph = '(';
		d->color = clrWhite;
		d->isMeleeWeapon = true;
	}
	break;

	case itemDef_meleeWpnIntr: {
		resetDef(d, itemDef_meleeWpn);
		d->isIntrinsicWeapon = true;
		d->spawnStandardMinDLVL = -1;
		d->spawnStandardMaxDLVL = -1;
	}
	break;

	case itemDef_rangedWpn: {
		resetDef(d, itemDef_general);
		d->isStackable = false;
		d->glyph = '}';
		d->color = clrWhite;
		d->isMeleeWeapon = true;
		d->meleeDmg = DiceParam(1, 6);
		d->isRangedWeapon = true;
		d->rangedMissileGlyph = '/';
		d->rangedMissileColor = clrWhite;
		d->spawnStandardMaxDLVL = FIRST_CAVERN_LEVEL - 1;
	}
	break;

	case itemDef_rangedWpnIntr: {
		resetDef(d, itemDef_rangedWpn);
		d->isIntrinsicWeapon = true;
		d->rangedHasInfiniteAmmo = true;
		d->spawnStandardMinDLVL = -1;
		d->spawnStandardMaxDLVL = -1;
		d->isMeleeWeapon = false; //(Extrinsic ranged weapons tend to double as melee weapons, while intrinsics do not)
		d->rangedMissileGlyph = '*';
	}
	break;

	case itemDef_missileWeapon: {
		resetDef(d, itemDef_general);
		d->isStackable = true;
		d->isMissileWeapon = true;
		d->spawnStandardMaxDLVL = FIRST_CAVERN_LEVEL - 1;
	}
	break;

	case itemDef_ammo: {
		resetDef(d, itemDef_general);
		d->itemWeight = itemWeight_light;
		d->glyph = '{';
		d->color = clrWhite;
		d->tile = tile_ammo;
		d->isAmmo = true;
		d->spawnStandardMaxDLVL = FIRST_CAVERN_LEVEL - 1;
	}
	break;

	case itemDef_ammoClip: {
		resetDef(d, itemDef_ammo);
		d->isStackable = false;
		d->isAmmoClip = true;
		d->spawnStandardMaxDLVL = FIRST_CAVERN_LEVEL - 1;
	}
	break;

	case itemDef_scroll: {
		resetDef(d, itemDef_general);
		d->chanceToIncludeInSpawnList = 75;
		d->itemWeight = itemWeight_feather;
		d->isIdentified = false;
		d->glyph = '?';
		d->color = clrWhite;
		d->tile = tile_scroll;
		d->isReadable = true;
		d->isScroll = true;
		d->isScrollLearnable = true;
		d->isScrollLearned = 0;
		d->abilityToIdentify = ability_loreLanguage;
		d->identifySkillFactor = eng->dice.getInRange(40, 100);
		d->maxStackSizeAtSpawn = 2;
		d->nativeRooms.push_back(specialRoom_tombs);
		d->nativeRooms.push_back(specialRoom_ritualChamber);
		eng->scrollNameHandler->setFalseScrollName(d);
	}
	break;

	case itemDef_potion: {
		resetDef(d, itemDef_general);
		d->chanceToIncludeInSpawnList = 75;
		d->itemWeight = itemWeight_light;
		d->isIdentified = false;
		d->glyph = '!';
		d->tile = tile_potion;
		d->isQuaffable = true;
		d->abilityToIdentify = ability_loreArcana;
		d->identifySkillFactor = eng->dice.getInRange(10, 100);
		d->isMissileWeapon = true;
		d->missileBaseAttackSkill = -5;
		d->missileDmg = DiceParam(1, 3, 0);
		d->maxStackSizeAtSpawn = 1;
		d->nativeRooms.push_back(specialRoom_tombs);
		d->nativeRooms.push_back(specialRoom_ritualChamber);
		eng->potionNameHandler->setColorAndFalseName(d);
	}
	break;

	case itemDef_armor: {
		resetDef(d, itemDef_general);
		d->itemWeight = itemWeight_heavy;
		d->glyph = '[';
		d->tile = tile_armor;
		d->isArmor = true;
		d->isStackable = false;
	}
	break;

	case itemDef_explosive: {
		resetDef(d, itemDef_general);
		d->isExplosive = true;
		d->glyph = '-';
		d->maxStackSizeAtSpawn = 3;
	}
	break;

	default: {
	}
	break;
	}
}

void ItemData::setDmgFromFormula(ItemDefinition& d, const ActorDefinition& owningActor, const EntityStrength_t dmgStrength) const {
	const int ACTOR_LEVEL = owningActor.monsterLvl;

	//Set 1dY dmg from formula
	const int DMG_Y_CAP = 999;
	const int DMG_Y_BASE = 3;
	const double DMG_Y_INCR = 0.25;
	const double DMG_STRENGTH_FACTOR = EntityStrength::getFactor(dmgStrength);
	const double DMG_Y_BEFORE_STR = DMG_Y_BASE + static_cast<double>(ACTOR_LEVEL - 1) * DMG_Y_INCR;
	const int DMG_Y_AFTER_STR = static_cast<int>(ceil(DMG_Y_BEFORE_STR * DMG_STRENGTH_FACTOR));
	const int DMG_Y_AFTER_CAP = min(DMG_Y_CAP, DMG_Y_AFTER_STR);

	//Set + dmg from formula
	const int DMG_PLUS_CAP = 6;
	const double DMG_PLUS_DIV = 2;
	const double DMG_PLUS_BUMP = 0.8;
	const int DMG_PLUS = static_cast<int>(floor(static_cast<double>(ACTOR_LEVEL - 1) / DMG_PLUS_DIV + DMG_PLUS_BUMP));
	const int DMG_PLUS_AFTER_CAP = min(DMG_PLUS_CAP, DMG_PLUS);

	d.meleeDmg = DiceParam(1, DMG_Y_AFTER_CAP, DMG_PLUS_AFTER_CAP);
	d.rangedDmg = DiceParam(1, DMG_Y_AFTER_CAP, DMG_PLUS_AFTER_CAP);
}

//------------------------------- LIST OF ITEMS
void ItemData::makeList() {
	ItemDefinition* d = NULL;

	d = new ItemDefinition(item_trapezohedron);
	resetDef(d, itemDef_general);
	d->name = ItemName("Shining Trapezohedron", "Shining Trapezohedrons", "The Shining Trapezohedron");
	d->spawnStandardMinDLVL = -1;
	d->spawnStandardMaxDLVL = -1;
	d->isStackable = false;
	d->glyph = '*';
	d->color = clrRedLight;
	d->tile = tile_trapezohedron;
	itemDefinitions[d->devName] = d;

	d = new ItemDefinition(item_sawedOff);
	resetDef(d, itemDef_rangedWpn);
	d->name = ItemName("Sawed-off Shotgun", "Sawed-off shotguns", "a Sawed-off Shotgun");
	d->tile = tile_shotgun;
	d->isShotgun = true;
	d->meleeAttackMessages = ItemAttackMessages("strike", "strikes you with a shotgun");
	d->rangedDmg = DiceParam(8, 3);
	d->rangedAmmoTypeUsed = item_shotgunShell;
	d->rangedAttackMessages = ItemAttackMessages("fire", "fires a shotgun");
	d->rangedSoundMessage = "You hear a shotgun blast.";
	d->rangedSoundStrength = SOUND_STANDARD_STRENGTH_GUNFIRE;
	d->rangedAbilityUsed = ability_accuracyRanged;
	d->meleeAbilityUsed = ability_accuracyMelee;
	d->itemWeight = itemWeight_medium;
	d->causeOfDeathMessage = "Shotgun";
	d->reloadAudio = audio_shotgun_load_shell;
	itemDefinitions[d->devName] = d;

	d = new ItemDefinition(item_pumpShotgun);
	resetDef(d, itemDef_rangedWpn);
	d->name = ItemName("Pump Shotgun", "Pump shotguns", "a Pump Shotgun");
	d->tile = tile_shotgun;
	d->isShotgun = true;
	d->meleeAttackMessages = ItemAttackMessages("strike", "strikes you with a shotgun");
	d->rangedDmg = DiceParam(7, 3);
	d->rangedAmmoTypeUsed = item_shotgunShell;
	d->rangedAttackMessages = ItemAttackMessages("fire", "fires a shotgun");
	d->rangedSoundMessage = "You hear a shotgun blast.";
	d->rangedSoundStrength = SOUND_STANDARD_STRENGTH_GUNFIRE;
	d->rangedAbilityUsed = ability_accuracyRanged;
	d->meleeAbilityUsed = ability_accuracyMelee;
	d->itemWeight = itemWeight_medium;
	d->causeOfDeathMessage = "Shotgun";
	d->rangedAudio = audio_shotgunPump_fire;
	d->reloadAudio = audio_shotgun_load_shell;
	itemDefinitions[d->devName] = d;

	d = new ItemDefinition(item_shotgunShell);
	resetDef(d, itemDef_ammo);
	d->name = ItemName("Shotgun shell", "Shotgun shells", "a shotgun shell");
	d->maxStackSizeAtSpawn = 10;
	itemDefinitions[d->devName] = d;

	d = new ItemDefinition(item_incinerator);
	resetDef(d, itemDef_rangedWpn);
	d->name = ItemName("Incinerator", "Incinerators", "an Incinerator");
	d->tile = tile_incinerator;
	d->meleeAttackMessages = ItemAttackMessages("strike", "strikes you with an Incinerator");
	d->rangedDmg = DiceParam(1, 3);
	d->rangedAbilityUsed = ability_accuracyRanged;
	d->meleeAbilityUsed = ability_accuracyMelee;
	d->rangedAmmoTypeUsed = item_incineratorShell;
	d->rangedAttackMessages = ItemAttackMessages("fire", "fires an incinerator");
	d->rangedSoundMessage = "You hear the blast of a launched missile.";
	d->rangedSoundStrength = SOUND_STANDARD_STRENGTH_GUNFIRE;
	d->rangedMissileGlyph = '*';
	d->rangedMissileColor = clrRedLight;
	d->spawnStandardMinDLVL = 4;
	d->rangedDmgLabelOverRide = "?";
	d->itemWeight = itemWeight_heavy;
	itemDefinitions[d->devName] = d;

	d = new ItemDefinition(item_incineratorShell);
	resetDef(d, itemDef_ammo);
	d->name = ItemName("Napalm shell", "Napalm shells", "a Napalm shell");
	d->spawnStandardMinDLVL = 3;
	d->maxStackSizeAtSpawn = 2;
	itemDefinitions[d->devName] = d;

	d = new ItemDefinition(item_machineGun);
	resetDef(d, itemDef_rangedWpn);
	d->name = ItemName("Tommy Gun", "Tommy Guns", "a Tommy Gun");
	d->tile = tile_tommyGun;
	d->meleeAttackMessages = ItemAttackMessages("strike", "strikes you with a Tommy Gun");
	d->isMachineGun = true;
	d->rangedDmg = DiceParam(2, 2, 2);
	d->rangedBaseAttackSkill = -10;
	d->rangedAbilityUsed = ability_accuracyRanged;
	d->meleeAbilityUsed = ability_accuracyMelee;
	d->rangedAmmoTypeUsed = item_drumOfBullets;
	d->rangedAttackMessages = ItemAttackMessages("fire", "fires a Tommy Gun");
	d->rangedSoundMessage = "You hear the burst of a machine gun.";
	d->rangedSoundStrength = SOUND_STANDARD_STRENGTH_GUNFIRE;
	d->itemWeight = itemWeight_medium;
	d->causeOfDeathMessage = "Machine gun fire";
	d->rangedAudio = audio_tommygun_fire;
	itemDefinitions[d->devName] = d;

	d = new ItemDefinition(item_drumOfBullets);
	resetDef(d, itemDef_ammoClip);
	d->name = ItemName("Drum of .45 ACP", "Drums of .45 ACP", "a Drum of .45 ACP");
	d->ammoContainedInClip = 50;
	itemDefinitions[d->devName] = d;

	d = new ItemDefinition(item_pistol);
	resetDef(d, itemDef_rangedWpn);
	d->name = ItemName("M1911 Colt", "M1911 Colt", "an M1911 Colt");
	d->tile = tile_pistol;
	d->rangedDmg = DiceParam(1, 8, 4);
	d->rangedAmmoTypeUsed = item_pistolClip;
	d->rangedAbilityUsed = ability_accuracyRanged;
	d->meleeAbilityUsed = ability_accuracyMelee;
	d->meleeAttackMessages = ItemAttackMessages("strike", "strikes you with a pistol");
	d->rangedAttackMessages = ItemAttackMessages("fire", "fires a pistol");
	d->rangedSoundMessage = "You hear a pistol being fired.";
	d->rangedSoundStrength = SOUND_STANDARD_STRENGTH_GUNFIRE;
	d->itemWeight = itemWeight_light;
	d->causeOfDeathMessage = "Shot with a pistol";
	d->rangedAudio = audio_pistol_fire;
	d->reloadAudio = audio_pistol_reload;
	itemDefinitions[d->devName] = d;

	d = new ItemDefinition(item_flareGun);
	resetDef(d, itemDef_rangedWpn);
	d->name = ItemName("Flare Gun", "Flare Gun", "a Flare Gun");
	d->tile = tile_flareGun;
	d->rangedDmg = DiceParam(1, 3, 0);
	d->rangedDmgLabelOverRide = "?";
	d->rangedAmmoTypeUsed = item_flare;
	d->rangedAbilityUsed = ability_accuracyRanged;
	d->meleeAbilityUsed = ability_accuracyMelee;
	d->meleeAttackMessages = ItemAttackMessages("strike", "strikes you with a flare gun");
	d->rangedAttackMessages = ItemAttackMessages("fire", "fires a flare gun");
	d->rangedSoundMessage = "You hear a flare gun being fired.";
	d->rangedSoundStrength = SOUND_STANDARD_STRENGTH_GUNFIRE;
	d->itemWeight = itemWeight_light;
	d->causeOfDeathMessage = "Shot with a flare gun";
	d->rangedStatusEffect = new StatusFlared(eng);
	//d->rangedAudio = audio_pistol_fire;
	//d->reloadAudio = audio_pistol_reload;
	itemDefinitions[d->devName] = d;

	d = new ItemDefinition(item_pistolClip);
	resetDef(d, itemDef_ammoClip);
	d->name = ItemName(".45ACP Colt cartridge", ".45ACP Colt cartridges", "a .45ACP Colt cartridge");
	d->ammoContainedInClip = 7;
	itemDefinitions[d->devName] = d;

	d = new ItemDefinition(item_teslaCanon);
	resetDef(d, itemDef_rangedWpn);
	d->name = ItemName("Tesla Canon", "Tesla Canons", "a Tesla Canon");
	d->tile = tile_teslaCannon;
	d->meleeAttackMessages = ItemAttackMessages("strike", "strikes you with a Tesla Cannon");
	d->isMachineGun = true;
	d->rangedBaseAttackSkill = -15;
	d->rangedAbilityUsed = ability_accuracyRanged;
	d->meleeAbilityUsed = ability_accuracyMelee;
	d->rangedDmg = DiceParam(2, 3, 2);
	d->rangedDamageType = damageType_electricity;
	d->rangedAmmoTypeUsed = item_teslaCanister;
	d->rangedAttackMessages = ItemAttackMessages("fire", "fires a Tesla Cannon");
	d->rangedSoundMessage = "You hear loud electric crackle.";
	d->rangedSoundStrength = SOUND_STANDARD_STRENGTH_GUNFIRE;
	d->rangedMissileGlyph = '*';
	d->rangedMissileColor = clrYellow;
	d->spawnStandardMinDLVL = 7;
	d->itemWeight = itemWeight_heavy;
	d->causeOfDeathMessage = "Fried by a Tesla Cannon";
	itemDefinitions[d->devName] = d;

	d = new ItemDefinition(item_spikeGun);
	resetDef(d, itemDef_rangedWpn);
	d->name = ItemName("Spike Gun", "Spike Guns", "a Spike Gun");
	d->tile = tile_tommyGun;
	d->color = clrBlueLight;
	d->meleeAttackMessages = ItemAttackMessages("strike", "strikes you with a Spike Gun");
	d->isMachineGun = false;
	d->rangedBaseAttackSkill = 0;
	d->rangedAbilityUsed = ability_accuracyRanged;
	d->meleeAbilityUsed = ability_accuracyMelee;
	d->rangedDmg = DiceParam(1, 7, 0);
	d->rangedDamageType = damageType_physical;
	d->rangedCausesKnockBack = true;
	d->rangedAmmoTypeUsed = item_ironSpike;
	d->rangedAttackMessages = ItemAttackMessages("fire", "fires a Spike Gun");
	d->rangedSoundMessage = "You hear a very crude gun being fired.";
	d->rangedSoundStrength = SOUND_STANDARD_STRENGTH_GUNFIRE + 1;
	d->rangedMissileGlyph = '/';
	d->rangedMissileColor = clrGray;
	d->spawnStandardMinDLVL = 4;
	d->itemWeight = itemWeight_heavy;
	d->causeOfDeathMessage = "Perforated by a Spike Gun";
	itemDefinitions[d->devName] = d;

	d = new ItemDefinition(item_teslaCanister);
	resetDef(d, itemDef_ammoClip);
	d->name = ItemName("Nuclear battery", "Nuclear batteries", "a Nuclear battery");
	d->ammoContainedInClip = 50;
	d->spawnStandardMinDLVL = 6;
	itemDefinitions[d->devName] = d;

	d = new ItemDefinition(item_dynamite);
	resetDef(d, itemDef_explosive);
	d->name = ItemName("Dynamite", "Sticks of Dynamite", "a Stick of Dynamite");
	d->tile = tile_dynamite;
	d->color = clrRedLight;
	d->itemWeight = itemWeight_light;
	itemDefinitions[d->devName] = d;

	d = new ItemDefinition(item_flare);
	resetDef(d, itemDef_explosive);
	d->name = ItemName("Flare", "Flares", "a Flare");
	d->tile = tile_flare;
	d->color = clrGray;
	d->glyph = '{';
	d->itemWeight = itemWeight_light;
	d->isAmmo = true;
	itemDefinitions[d->devName] = d;

	d = new ItemDefinition(item_molotov);
	resetDef(d, itemDef_explosive);
	d->name = ItemName("Molotov Cocktail", "Molotov Cocktails", "a Molotov Cocktail");
	d->tile = tile_molotov;
	d->color = clrWhite;
	d->itemWeight = itemWeight_light;
	itemDefinitions[d->devName] = d;

	d = new ItemDefinition(item_throwingKnife);
	resetDef(d, itemDef_missileWeapon);
	d->name = ItemName("Throwing Knife", "Throwing Knives", "a Throwing Knife");
	d->tile = tile_dagger;
	d->itemWeight = itemWeight_light;
	d->glyph = '/';
	d->color = clrWhite;
	d->missileBaseAttackSkill = 0;
	d->missileDmg = DiceParam(2, 4);
	d->maxStackSizeAtSpawn = 12;
	itemDefinitions[d->devName] = d;

	d = new ItemDefinition(item_rock);
	resetDef(d, itemDef_missileWeapon);
	d->name = ItemName("Rock", "Rocks", "a Rock");
	d->tile = tile_rock;
	d->itemWeight = itemWeight_light;
	d->glyph = '*';
	d->color = clrGray;
	d->missileBaseAttackSkill = 10;
	d->missileDmg = DiceParam(1, 3);
	d->maxStackSizeAtSpawn = 12;
	itemDefinitions[d->devName] = d;

	d = new ItemDefinition(item_dagger);
	resetDef(d, itemDef_meleeWpn);
	d->name = ItemName("Dagger", "Daggers", "a Dagger");
	d->tile = tile_dagger;
	d->meleeAttackMessages = ItemAttackMessages("strike", "strikes you with a Dagger");
	d->meleeDmg = DiceParam(1, 4);
	d->meleeBaseAttackSkill = 20;
	d->meleeAbilityUsed = ability_accuracyMelee;
	d->itemWeight = itemWeight_light;
	d->causeOfDeathMessage = "Stabbed with a dagger";
	itemDefinitions[d->devName] = d;

	d = new ItemDefinition(item_hatchet);
	resetDef(d, itemDef_meleeWpn);
	d->name = ItemName("Hatchet", "Hatchets", "a Hatchet");
	d->tile = tile_axe;
	d->meleeAttackMessages = ItemAttackMessages("strike", "strikes you with a Hatchet");
	d->meleeDmg = DiceParam(1, 5);
	d->meleeBaseAttackSkill = 15;
	d->meleeAbilityUsed = ability_accuracyMelee;
	d->itemWeight = itemWeight_light;
	itemDefinitions[d->devName] = d;

	d = new ItemDefinition(item_club);
	resetDef(d, itemDef_meleeWpn);
	d->name = ItemName("Club", "Clubs", "a Club");
	d->tile = tile_club;
	d->color = clrBrown;
	d->meleeAttackMessages = ItemAttackMessages("strike", "strikes you with a Club");
	d->meleeDmg = DiceParam(2, 3);
	d->meleeBaseAttackSkill = 10;
	d->meleeAbilityUsed = ability_accuracyMelee;
	d->itemWeight = itemWeight_medium;
	itemDefinitions[d->devName] = d;

	d = new ItemDefinition(item_hammer);
	resetDef(d, itemDef_meleeWpn);
	d->name = ItemName("Hammer", "Hammers", "a Hammer");
	d->tile = tile_hammer;
	d->meleeAttackMessages = ItemAttackMessages("strike", "strikes you with a Hammer");
	d->meleeDmg = DiceParam(2, 4);
	d->meleeBaseAttackSkill = 5;
	d->meleeAbilityUsed = ability_accuracyMelee;
	d->itemWeight = itemWeight_medium;
	itemDefinitions[d->devName] = d;

	d = new ItemDefinition(item_machete);
	resetDef(d, itemDef_meleeWpn);
	d->name = ItemName("Machete", "Machetes", "a Machete");
	d->tile = tile_machete;
	d->meleeAttackMessages = ItemAttackMessages("strike", "strikes you with a Machete");
	d->meleeDmg = DiceParam(2, 5);
	d->meleeBaseAttackSkill = 0;
	d->meleeAbilityUsed = ability_accuracyMelee;
	d->itemWeight = itemWeight_medium;
	itemDefinitions[d->devName] = d;

	d = new ItemDefinition(item_axe);
	resetDef(d, itemDef_meleeWpn);
	d->name = ItemName("Axe", "Axes", "an Axe");
	d->tile = tile_axe;
	d->meleeAttackMessages = ItemAttackMessages("strike", "strikes you with an axe");
	d->meleeDmg = DiceParam(2, 6);
	d->meleeBaseAttackSkill = -5;
	d->meleeAbilityUsed = ability_accuracyMelee;
	d->itemWeight = itemWeight_medium;
	itemDefinitions[d->devName] = d;

	d = new ItemDefinition(item_pitchFork);
	resetDef(d, itemDef_meleeWpn);
	d->name = ItemName("Pitchfork", "Pitchforks", "a Pitchfork");
	d->tile = tile_pitchfork;
	d->meleeAttackMessages = ItemAttackMessages("strike", "strikes you with a Pitchfork");
	d->meleeDmg = DiceParam(3, 4);
	d->meleeBaseAttackSkill = -10;
	d->meleeAbilityUsed = ability_accuracyMelee;
	d->itemWeight = itemWeight_heavy;
	d->meleeCausesKnockBack = true;
	itemDefinitions[d->devName] = d;

	d = new ItemDefinition(item_sledgeHammer);
	resetDef(d, itemDef_meleeWpn);
	d->name = ItemName("Sledge Hammer", "Sledge Hammers", "a Sledge Hammer");
	d->tile = tile_sledgeHammer;
	d->meleeAttackMessages = ItemAttackMessages("strike", "strikes you with a Sledge Hammer");
	d->meleeDmg = DiceParam(3, 5);
	d->meleeBaseAttackSkill = -15;
	d->meleeAbilityUsed = ability_accuracyMelee;
	d->itemWeight = itemWeight_heavy;
	d->meleeCausesKnockBack = true;
	itemDefinitions[d->devName] = d;

	d = new ItemDefinition(item_ironSpike);
	resetDef(d, itemDef_ammo);
	d->name = ItemName("Iron Spike", "Iron Spikes", "an Iron Spike");
	d->tile = tile_ironSpike;
	d->isStackable = true;
	d->itemWeight = itemWeight_light;
	d->color = clrGray;
	d->glyph = '|';
	d->maxStackSizeAtSpawn = 12;
	itemDefinitions[d->devName] = d;

	d = new ItemDefinition(item_playerKick);
	resetDef(d, itemDef_meleeWpnIntr);
	d->meleeAttackMessages = ItemAttackMessages("kick", "");
	d->meleeAbilityUsed = ability_accuracyMelee;
	d->meleeBaseAttackSkill = 25;
	d->meleeDmg = DiceParam(1, 3);
	d->meleeCausesKnockBack = true;
	itemDefinitions[d->devName] = d;

	d = new ItemDefinition(item_playerStomp);
	resetDef(d, itemDef_meleeWpnIntr);
	d->meleeAttackMessages = ItemAttackMessages("stomp", "");
	d->meleeAbilityUsed = ability_accuracyMelee;
	d->meleeBaseAttackSkill = 25;
	d->meleeDmg = DiceParam(1, 3);
	d->meleeCausesKnockBack = false;
	itemDefinitions[d->devName] = d;

	d = new ItemDefinition(item_zombieClaw);
	resetDef(d, itemDef_meleeWpnIntr);
	d->meleeAttackMessages = ItemAttackMessages("", "claws you");
	setDmgFromFormula(*d, eng->actorData->actorDefinitions[actor_zombie], normal);
	d->causeOfDeathMessage = "Clawed by an undead creature";
	itemDefinitions[d->devName] = d;

	d = new ItemDefinition(item_zombieClawDiseased);
	resetDef(d, itemDef_meleeWpnIntr);
	d->meleeAttackMessages = ItemAttackMessages("", "claws you");
	setDmgFromFormula(*d, eng->actorData->actorDefinitions[actor_zombie], normal);
	d->meleeStatusEffect = new StatusDiseased(eng);
	d->causeOfDeathMessage = "Clawed by a diseased undead creature";
	itemDefinitions[d->devName] = d;

	d = new ItemDefinition(item_zombieAxe);
	resetDef(d, itemDef_meleeWpnIntr);
	d->meleeAttackMessages = ItemAttackMessages("", "chops you with a rusty axe");
	setDmgFromFormula(*d, eng->actorData->actorDefinitions[actor_zombieAxe], strong);
	d->causeOfDeathMessage = "Chopped with an axe by an undead creature";
	itemDefinitions[d->devName] = d;

	d = new ItemDefinition(item_bloatedZombiePunch);
	resetDef(d, itemDef_meleeWpnIntr);
	d->meleeAttackMessages = ItemAttackMessages("", "mauls you");
	setDmgFromFormula(*d, eng->actorData->actorDefinitions[actor_bloatedZombie], strong);
	d->meleeCausesKnockBack = true;
	d->causeOfDeathMessage = "Mauled by an undead creature";
	itemDefinitions[d->devName] = d;

	d = new ItemDefinition(item_bloatedZombieSpit);
	resetDef(d, itemDef_rangedWpnIntr);
	d->rangedAttackMessages = ItemAttackMessages("", "spits a gob of puke at you");
	setDmgFromFormula(*d, eng->actorData->actorDefinitions[actor_zombieAxe], strong);
	d->rangedSoundMessage = "You hear spitting.";
	d->rangedMissileColor = clrGreenLight;
	d->rangedDamageType = damageType_acid;
	d->rangedMissileGlyph = '*';
	d->causeOfDeathMessage = "Spat upon by an undead creature";
	d->rangedSoundStrength = 3;
	itemDefinitions[d->devName] = d;

	d = new ItemDefinition(item_ratBite);
	resetDef(d, itemDef_meleeWpnIntr);
	d->meleeAttackMessages = ItemAttackMessages("", "bites you");
	setDmgFromFormula(*d, eng->actorData->actorDefinitions[actor_rat], normal);
	d->causeOfDeathMessage = "Gnawed by a rat";
	itemDefinitions[d->devName] = d;

	d = new ItemDefinition(item_ratBiteDiseased);
	resetDef(d, itemDef_meleeWpnIntr);
	d->meleeAttackMessages = ItemAttackMessages("", "bites you");
	setDmgFromFormula(*d, eng->actorData->actorDefinitions[actor_rat], normal);
	d->meleeStatusEffect = new StatusDiseased(eng);
	d->causeOfDeathMessage = "Gnawed by a diseased rat";
	itemDefinitions[d->devName] = d;

	d = new ItemDefinition(item_ratThingBite);
	resetDef(d, itemDef_meleeWpnIntr);
	d->meleeAttackMessages = ItemAttackMessages("", "bites you");
	setDmgFromFormula(*d, eng->actorData->actorDefinitions[actor_ratThing], strong);
	d->causeOfDeathMessage = "Gnawed by a very strange rat";
	itemDefinitions[d->devName] = d;

	d = new ItemDefinition(item_wormMassBite);
	resetDef(d, itemDef_meleeWpnIntr);
	d->meleeAttackMessages = ItemAttackMessages("", "bites you");
	setDmgFromFormula(*d, eng->actorData->actorDefinitions[actor_wormMass], weak);
	d->causeOfDeathMessage = "Eaten by worms";
	itemDefinitions[d->devName] = d;

	d = new ItemDefinition(item_wolfBite);
	resetDef(d, itemDef_meleeWpnIntr);
	d->meleeAttackMessages = ItemAttackMessages("", "bites you");
	setDmgFromFormula(*d, eng->actorData->actorDefinitions[actor_wolf], normal);
	d->causeOfDeathMessage = "Bitten by a wolf";
	itemDefinitions[d->devName] = d;

	d = new ItemDefinition(item_greenSpiderBite);
	resetDef(d, itemDef_meleeWpnIntr);
	d->meleeAttackMessages = ItemAttackMessages("", "bites you");
	setDmgFromFormula(*d, eng->actorData->actorDefinitions[actor_greenSpider], normal);
	d->meleeStatusEffect = new StatusBlind(4);
	d->causeOfDeathMessage = "Bitten by a green spider";
	itemDefinitions[d->devName] = d;

	d = new ItemDefinition(item_whiteSpiderBite);
	resetDef(d, itemDef_meleeWpnIntr);
	d->meleeAttackMessages = ItemAttackMessages("", "bites you");
	setDmgFromFormula(*d, eng->actorData->actorDefinitions[actor_whiteSpider], normal);
	d->meleeStatusEffect = new StatusParalyzed(3);
	d->causeOfDeathMessage = "Bitten by a white spider";
	itemDefinitions[d->devName] = d;

	d = new ItemDefinition(item_blackSpiderBite);
	resetDef(d, itemDef_meleeWpnIntr);
	d->meleeAttackMessages = ItemAttackMessages("", "bites you");
	setDmgFromFormula(*d, eng->actorData->actorDefinitions[actor_blackSpider], normal);
	d->causeOfDeathMessage = "Bitten by a black spider";
	itemDefinitions[d->devName] = d;

	d = new ItemDefinition(item_lengSpiderBite);
	resetDef(d, itemDef_meleeWpnIntr);
	d->meleeAttackMessages = ItemAttackMessages("", "bites you");
	setDmgFromFormula(*d, eng->actorData->actorDefinitions[actor_lengSpider], strong);
	d->causeOfDeathMessage = "Bitten by a Leng-Spider";
	itemDefinitions[d->devName] = d;

	d = new ItemDefinition(item_hellHoundFireBreath);
	resetDef(d, itemDef_rangedWpnIntr);
	d->rangedAttackMessages = ItemAttackMessages("", "breaths fire at you");
	d->rangedSoundMessage = "You hear a burst of flames.";
	d->rangedSoundStrength = 3;
	setDmgFromFormula(*d, eng->actorData->actorDefinitions[actor_fireHound], normal);
	d->rangedStatusEffect = new StatusBurning(eng);
	d->rangedMissileColor = clrRedLight;
	d->rangedMissileGlyph = '*';
	d->rangedMissileLeavesTrail = true;
	d->rangedMissileLeavesSmoke = true;
	d->rangedDamageType = damageType_fire;
	d->causeOfDeathMessage = "Struck by a blast of fire";
	itemDefinitions[d->devName] = d;

	d = new ItemDefinition(item_hellHoundBite);
	resetDef(d, itemDef_meleeWpnIntr);
	d->meleeAttackMessages = ItemAttackMessages("", "bites you");
	setDmgFromFormula(*d, eng->actorData->actorDefinitions[actor_fireHound], normal);
	d->causeOfDeathMessage = "Bitten by a fire hound";
	itemDefinitions[d->devName] = d;

	d = new ItemDefinition(item_fireVampireTouch);
	resetDef(d, itemDef_meleeWpnIntr);
	d->meleeAttackMessages = ItemAttackMessages("", "touches you you");
	setDmgFromFormula(*d, eng->actorData->actorDefinitions[actor_fireVampire], weak);
	d->meleeStatusEffect = new StatusBurning(eng);
	d->causeOfDeathMessage = "Touched by a Fire Vampire";
	itemDefinitions[d->devName] = d;

	d = new ItemDefinition(item_ghostClaw);
	resetDef(d, itemDef_meleeWpnIntr);
	d->meleeAttackMessages = ItemAttackMessages("", "claws you");
	setDmgFromFormula(*d, eng->actorData->actorDefinitions[actor_ghost], normal);
	d->meleeStatusEffect = new StatusTerrified(4);
	d->causeOfDeathMessage = "Clawed by a ghostly figure";
	itemDefinitions[d->devName] = d;

	d = new ItemDefinition(item_phantasmSickle);
	resetDef(d, itemDef_meleeWpnIntr);
	d->meleeAttackMessages = ItemAttackMessages("", "cuts you with a sickle");
	setDmgFromFormula(*d, eng->actorData->actorDefinitions[actor_phantasm], normal);
	d->meleeStatusEffect = new StatusTerrified(4);
	d->causeOfDeathMessage = "Sliced by a sickle";
	itemDefinitions[d->devName] = d;

	d = new ItemDefinition(item_wraithClaw);
	resetDef(d, itemDef_meleeWpnIntr);
	d->meleeAttackMessages = ItemAttackMessages("", "claws you");
	setDmgFromFormula(*d, eng->actorData->actorDefinitions[actor_wraith], normal);
	d->meleeStatusEffect = new StatusTerrified(4);
	d->causeOfDeathMessage = "Clawed by a great ghostly figure";
	itemDefinitions[d->devName] = d;

	d = new ItemDefinition(item_giantBatBite);
	resetDef(d, itemDef_meleeWpnIntr);
	d->meleeAttackMessages = ItemAttackMessages("", "bites you");
	setDmgFromFormula(*d, eng->actorData->actorDefinitions[actor_giantBat], weak);
	d->causeOfDeathMessage = "Bitten by a giant bat";
	itemDefinitions[d->devName] = d;

	d = new ItemDefinition(item_miGoElectricGun);
	resetDef(d, itemDef_rangedWpnIntr);
	d->rangedMissileLeavesTrail = true;
	d->rangedMissileColor = clrYellow;
	d->rangedMissileGlyph = '/';
	d->rangedAttackMessages = ItemAttackMessages("", "fires an electric gun");
	d->rangedDamageType = damageType_electricity;
	d->rangedStatusEffect = new StatusParalyzed(2);
	d->rangedSoundMessage = "You hear a bolt of electricity.";
	d->rangedSoundStrength = SOUND_STANDARD_STRENGTH_GUNFIRE;
	setDmgFromFormula(*d, eng->actorData->actorDefinitions[actor_miGo], strong);
	d->causeOfDeathMessage = "Electrocuted by an alien weapon";
	itemDefinitions[d->devName] = d;

	d = new ItemDefinition(item_ghoulClaw);
	resetDef(d, itemDef_meleeWpnIntr);
	d->meleeAttackMessages = ItemAttackMessages("", "claws you");
	setDmgFromFormula(*d, eng->actorData->actorDefinitions[actor_ghoul], normal);
	d->meleeStatusEffect = new StatusDiseased(eng);
	d->causeOfDeathMessage = "Clawed by a ghoul";
	itemDefinitions[d->devName] = d;

	d = new ItemDefinition(item_shadowClaw);
	resetDef(d, itemDef_meleeWpnIntr);
	d->meleeAttackMessages = ItemAttackMessages("", "claws you");
	setDmgFromFormula(*d, eng->actorData->actorDefinitions[actor_shadow], normal);
	d->causeOfDeathMessage = "Clawed by a Shadow";
	itemDefinitions[d->devName] = d;

	d = new ItemDefinition(item_byakheeClaw);
	resetDef(d, itemDef_meleeWpnIntr);
	d->meleeAttackMessages = ItemAttackMessages("", "claws you");
	setDmgFromFormula(*d, eng->actorData->actorDefinitions[actor_byakhee], normal);
	d->causeOfDeathMessage = "Clawed by a Byakhee";
	itemDefinitions[d->devName] = d;

	d = new ItemDefinition(item_mummyMaul);
	resetDef(d, itemDef_meleeWpnIntr);
	d->meleeAttackMessages = ItemAttackMessages("", "mauls you");
	setDmgFromFormula(*d, eng->actorData->actorDefinitions[actor_mummy], strong);
	d->meleeCausesKnockBack = true;
	d->causeOfDeathMessage = "Mauled by a mummy";
	itemDefinitions[d->devName] = d;

	d = new ItemDefinition(item_deepOneJavelinAttack);
	resetDef(d, itemDef_rangedWpnIntr);
	d->rangedAttackMessages = ItemAttackMessages("", "throws a Javelin at you");
	setDmgFromFormula(*d, eng->actorData->actorDefinitions[actor_deepOne], normal);
	d->rangedSoundMessage = "You hear something being thrown your way.";
	d->rangedMissileColor = clrBrown;
	d->rangedMissileGlyph = '/';
	d->causeOfDeathMessage = "Hit by a Javelin from a Deep One";
	d->rangedSoundStrength = 3;
	itemDefinitions[d->devName] = d;

	d = new ItemDefinition(item_deepOneSpearAttack);
	resetDef(d, itemDef_meleeWpnIntr);
	d->meleeAttackMessages = ItemAttackMessages("", "thrusts a spear at you");
	setDmgFromFormula(*d, eng->actorData->actorDefinitions[actor_deepOne], normal);
	d->causeOfDeathMessage = "Impaled on a spear by a Deep One";
	itemDefinitions[d->devName] = d;

	d = new ItemDefinition(item_oozeGraySpewPus);
	resetDef(d, itemDef_meleeWpnIntr);
	d->meleeAttackMessages = ItemAttackMessages("", "spews pus on you");
	setDmgFromFormula(*d, eng->actorData->actorDefinitions[actor_oozeGray], strong);
	d->causeOfDeathMessage = "Drenched in slime";
	itemDefinitions[d->devName] = d;

	d = new ItemDefinition(item_oozeClearSpewPus);
	resetDef(d, itemDef_meleeWpnIntr);
	d->meleeAttackMessages = ItemAttackMessages("", "spews pus on you");
	setDmgFromFormula(*d, eng->actorData->actorDefinitions[actor_oozeClear], strong);
	d->causeOfDeathMessage = "Drenched in slime";
	itemDefinitions[d->devName] = d;

	d = new ItemDefinition(item_oozePutridSpewPus);
	resetDef(d, itemDef_meleeWpnIntr);
	d->meleeAttackMessages = ItemAttackMessages("", "spews infected pus on you");
	setDmgFromFormula(*d, eng->actorData->actorDefinitions[actor_oozePutrid], strong);
	d->meleeStatusEffect = new StatusDiseased(eng);
	d->causeOfDeathMessage = "Drenched in infected slime";
	itemDefinitions[d->devName] = d;

	d = new ItemDefinition(item_huntingHorrorBite);
	resetDef(d, itemDef_meleeWpnIntr);
	d->meleeAttackMessages = ItemAttackMessages("", "bites you");
	setDmgFromFormula(*d, eng->actorData->actorDefinitions[actor_huntingHorror], strong);
	d->meleeStatusEffect = new StatusParalyzed(eng);
	d->causeOfDeathMessage = "Eaten by a Hunting Horror";
	itemDefinitions[d->devName] = d;

	d = new ItemDefinition(item_armorLeatherJacket);
	resetDef(d, itemDef_armor);
	d->name = ItemName("Leather Jacket", "", "a Leather Jacket");
	d->color = clrGray;
	d->spawnStandardMinDLVL = 1;
	d->armorData.absorptionPoints[damageType_acid] = 1;
	d->armorData.damageToDurabilityFactors[damageType_acid] = 2.0;
	d->armorData.absorptionPoints[damageType_electricity] = 2;
	d->armorData.damageToDurabilityFactors[damageType_electricity] = 0.0;
	d->armorData.absorptionPoints[damageType_fire] = 1;
	d->armorData.damageToDurabilityFactors[damageType_fire] = 2.0;
	d->armorData.absorptionPoints[damageType_physical] = 1;
	d->armorData.damageToDurabilityFactors[damageType_physical] = 1.0;
	d->armorData.chanceToDeflectTouchAttacks = 20;
	itemDefinitions[d->devName] = d;

	d = new ItemDefinition(item_armorIronSuit);
	resetDef(d, itemDef_armor);
	d->name = ItemName("Iron Suit", "", "an Iron Suit");
	d->color = clrWhite;
	d->spawnStandardMinDLVL = 2;
	d->armorData.absorptionPoints[damageType_acid] = 1;
	d->armorData.damageToDurabilityFactors[damageType_acid] = 2.0;
	d->armorData.absorptionPoints[damageType_electricity] = 0;
	d->armorData.damageToDurabilityFactors[damageType_electricity] = 0.0;
	d->armorData.absorptionPoints[damageType_fire] = 1;
	d->armorData.damageToDurabilityFactors[damageType_fire] = 2.0;
	d->armorData.absorptionPoints[damageType_physical] = 4;
	d->armorData.damageToDurabilityFactors[damageType_physical] = 0.5;
	d->armorData.chanceToDeflectTouchAttacks = 80;
	itemDefinitions[d->devName] = d;

	d = new ItemDefinition(item_armorFlackJacket);
	resetDef(d, itemDef_armor);
	d->name = ItemName("Flak Jacket", "", "a Flak Jacket");
	d->color = clrGreen;
	d->spawnStandardMinDLVL = 3;
	d->armorData.absorptionPoints[damageType_acid] = 1;
	d->armorData.damageToDurabilityFactors[damageType_acid] = 2.0;
	d->armorData.absorptionPoints[damageType_electricity] = 0;
	d->armorData.damageToDurabilityFactors[damageType_electricity] = 0.0;
	d->armorData.absorptionPoints[damageType_fire] = 1;
	d->armorData.damageToDurabilityFactors[damageType_fire] = 2.0;
	d->armorData.absorptionPoints[damageType_physical] = 3;
	d->armorData.damageToDurabilityFactors[damageType_physical] = 0.5;
	d->armorData.chanceToDeflectTouchAttacks = 20;
	itemDefinitions[d->devName] = d;

//	d = new ItemDefinition(item_armorAsbestosSuit);
//	resetDef(d, itemDef_armor);
//	d->name = ItemName("Asbestos Suit", "", "an Asbestos Suit");
//	d->color = clrRedLight;
//	d->spawnStandardMinDLVL = 3;
//	d->armorData.absorptionPoints[damageType_acid] = 999;
//	d->armorData.damageToDurabilityFactors[damageType_acid] = 0.1;
//	d->armorData.absorptionPoints[damageType_electricity] = 999;
//	d->armorData.damageToDurabilityFactors[damageType_electricity] = 0.0;
//	d->armorData.absorptionPoints[damageType_fire] = 999;
//	d->armorData.damageToDurabilityFactors[damageType_fire] = 0.1;
//	d->armorData.absorptionPoints[damageType_physical] = 0;
//	d->armorData.damageToDurabilityFactors[damageType_physical] = 2.0;
//	d->armorData.chanceToDeflectTouchAttacks = 95;
//	d->armorData.protectsAgainstStatusBurning = true;
//	d->armorData.overRideAbsorptionPointLabel = "?";
//	itemDefinitions[d->devName] = d;

   d = new ItemDefinition(item_scrollOfMayhem);
	resetDef(d, itemDef_scroll);
	itemDefinitions[d->devName] = d;

	d = new ItemDefinition(item_scrollOfTeleportation);
	resetDef(d, itemDef_scroll);
	itemDefinitions[d->devName] = d;

	d = new ItemDefinition(item_scrollOfDeepDescent);
	resetDef(d, itemDef_scroll);
	d->spawnStandardMinDLVL = 6;
	d->identifySkillFactor = 40;
	d->isScrollLearnable = false;
	itemDefinitions[d->devName] = d;

	d = new ItemDefinition(item_scrollOfPestilence);
	resetDef(d, itemDef_scroll);
	d->isScrollLearnable = false;
	itemDefinitions[d->devName] = d;

	d = new ItemDefinition(item_scrollOfConfuseEnemies);
	resetDef(d, itemDef_scroll);
	itemDefinitions[d->devName] = d;

	d = new ItemDefinition(item_scrollOfParalyzeEnemies);
	resetDef(d, itemDef_scroll);
	itemDefinitions[d->devName] = d;

	d = new ItemDefinition(item_scrollOfSlowEnemies);
	resetDef(d, itemDef_scroll);
	itemDefinitions[d->devName] = d;

	d = new ItemDefinition(item_scrollOfDetectItems);
	resetDef(d, itemDef_scroll);
	itemDefinitions[d->devName] = d;

	d = new ItemDefinition(item_scrollOfDetectTraps);
	resetDef(d, itemDef_scroll);
	itemDefinitions[d->devName] = d;

    d = new ItemDefinition(item_scrollOfBlessing);
	resetDef(d, itemDef_scroll);
	itemDefinitions[d->devName] = d;

	d = new ItemDefinition(item_potionOfHealing);
	resetDef(d, itemDef_potion);
	itemDefinitions[d->devName] = d;

	d = new ItemDefinition(item_potionOfBlindness);
	resetDef(d, itemDef_potion);
	itemDefinitions[d->devName] = d;

	d = new ItemDefinition(item_potionOfClairvoyance);
	resetDef(d, itemDef_potion);
	itemDefinitions[d->devName] = d;

	d = new ItemDefinition(item_potionOfCorruption);
	resetDef(d, itemDef_potion);
	itemDefinitions[d->devName] = d;

	d = new ItemDefinition(item_potionOfReflexes);
	resetDef(d, itemDef_potion);
	itemDefinitions[d->devName] = d;

	d = new ItemDefinition(item_potionOfAiming);
	resetDef(d, itemDef_potion);
	itemDefinitions[d->devName] = d;

	d = new ItemDefinition(item_potionOfStealth);
	resetDef(d, itemDef_potion);
	itemDefinitions[d->devName] = d;

	d = new ItemDefinition(item_potionOfFortitude);
	resetDef(d, itemDef_potion);
	itemDefinitions[d->devName] = d;

	d = new ItemDefinition(item_potionOfToughness);
	resetDef(d, itemDef_potion);
	itemDefinitions[d->devName] = d;

	d = new ItemDefinition(item_potionOfParalyzation);
	resetDef(d, itemDef_potion);
	itemDefinitions[d->devName] = d;

   d = new ItemDefinition(item_potionOfFear);
	resetDef(d, itemDef_potion);
	itemDefinitions[d->devName] = d;

	d = new ItemDefinition(item_potionOfConfusion);
	resetDef(d, itemDef_potion);
	itemDefinitions[d->devName] = d;

	/*
	 d = new ItemDefinition(item_ration);
	 resetDef(d, itemDef_general);
	 d->itemWeight				= itemWeight_light;
	 d->name					= ItemName("Iron Ration", "Iron Rations", "an Iron Ration");
	 d->glyph					= '%';
	 d->isEatable				= true;
	 d->nutritionValue			= NutritionValue_dinner;
	 itemDefinitions[d->devName] = d;
	 */
}

void ItemData::addSaveLines(vector<string>& lines) const {
	for(unsigned int i = 1; i < endOfItemDevNames; i++) {
		lines.push_back(itemDefinitions[i]->isIdentified == false ? "0" : "1");
		if(itemDefinitions[i]->abilityToIdentify != ability_empty) {
			lines.push_back(intToString(itemDefinitions[i]->identifySkillFactor));
		}
		if(itemDefinitions[i]->isScroll == true) {
			lines.push_back(itemDefinitions[i]->isScrollLearned == false ? "0" : "1");
		}
	}
}

void ItemData::setParametersFromSaveLines(vector<string>& lines) {
	for(unsigned int i = 1; i < endOfItemDevNames; i++) {
		itemDefinitions[i]->isIdentified = lines.front() == "0" ? false : true;
		lines.erase(lines.begin());
		if(itemDefinitions[i]->abilityToIdentify != ability_empty) {
			itemDefinitions[i]->identifySkillFactor = stringToInt(lines.front());
			lines.erase(lines.begin());
		}
		if(itemDefinitions[i]->isScroll == true) {
			itemDefinitions[i]->isScrollLearned = lines.front() == "0" ? false : true;
			lines.erase(lines.begin());
		}
	}
}

bool ItemData::isWeaponStronger(const ItemDefinition& oldDef, const ItemDefinition& newDef, bool melee) {
	int rolls1 = 0;
	int sides1 = 0;
	int plus1 = 0;
	int rolls2 = 0;
	int sides2 = 0;
	int plus2 = 0;

	if(melee == true) {
		rolls1 = oldDef.meleeDmg.rolls;
		sides1 = oldDef.meleeDmg.sides;
		plus1 = oldDef.meleeDmg.plus;

		rolls2 = newDef.meleeDmg.rolls;
		sides2 = newDef.meleeDmg.sides;
		plus2 = newDef.meleeDmg.plus;
	} else {
		rolls1 = oldDef.rangedDmg.rolls;
		sides1 = oldDef.rangedDmg.sides;
		plus1 = oldDef.rangedDmg.plus;

		rolls2 = newDef.rangedDmg.rolls;
		sides2 = newDef.rangedDmg.sides;
		plus2 = newDef.rangedDmg.plus;
	}

	bool newWeaponIsStronger = rolls2 * sides2 + plus2 > rolls1 * sides1 + plus1;

	return newWeaponIsStronger;
}

string ItemData::itemInterfaceName(Item* const item, const bool PUT_A_OR_AN_IN_FRONT) const {
	const ItemDefinition& d = item->getInstanceDefinition();

	string str;
	if(d.isStackable == true && item->numberOfItems > 1) {
		str = intToString(item->numberOfItems) + " " + d.name.name_plural;
	} else {
		str = PUT_A_OR_AN_IN_FRONT == true ? d.name.name_a : d.name.name;
		if(d.isAmmoClip == true) {
			str += " {" + intToString((dynamic_cast<ItemAmmoClip*>(item))->ammo) + "}";
		}
		if(d.isRangedWeapon == true && d.rangedHasInfiniteAmmo == false) {
			str += " {" + intToString((dynamic_cast<Weapon*>(item))->ammoLoaded) + "}";
		}
		if(d.isMeleeWeapon == true && d.isRangedWeapon == false) {
			const int PLUS = d.meleeDmg.plus;
			str += PLUS > 0 ? " (+" + intToString(PLUS) + ")" : PLUS < 0 ? " (-" + intToString(PLUS) + ")" : "";
		}
		if(d.isArmor == true) {
			str += " " + dynamic_cast<Armor*>(item)->getArmorDataLine();
		}
	}
	return str;
}
