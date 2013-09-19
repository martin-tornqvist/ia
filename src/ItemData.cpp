#include "ItemData.h"

#include <iostream>

#include "Engine.h"

#include "Colors.h"
#include "Item.h"
#include "ItemWeapon.h"
#include "ItemAmmo.h"
#include "CommonData.h"
#include "ActorData.h"
#include "ItemArmor.h"
#include "ItemScroll.h"
#include "ItemPotion.h"
#include "ActorPlayer.h"
#include "ItemMedicalBag.h"

using namespace std;

//------------------------------- ITEM ARCHETYPES (DEFAULTS)
void ItemDataHandler::resetData(ItemData* const d,
                                ItemDataArchetypes_t const archetype) const {
  switch(archetype) {
    case itemData_general: {
      d->itemValue = itemValue_normal;
      d->itemWeight = itemWeight_none;
      d->spawnStandardMinDLVL = 1;
      d->spawnStandardMaxDLVL = 999999;
      d->maxStackSizeAtSpawn = 1;
      d->chanceToIncludeInSpawnList = 100;
      d->isStackable = true;
      d->isIdentified = true;
      d->isTried = false;
      d->glyph = 'X';
      d->color = clrWhite;
      d->tile = tile_empty;
      d->primaryAttackMode = primaryAttackMode_none;
      d->isReadable = d->isScroll = d->isQuaffable = d->isEatable = false;
      d->isArmor = d->isCloak = d->isRing = d->isAmulet = false;
      d->isIntrinsic = d->isMeleeWeapon = d->isRangedWeapon = false;
      d->isMissileWeapon = d->isShotgun = d->isMachineGun = false;
      d->isAmmo = d->isAmmoClip = d->isDevice = d->isMedicalBag = false;
      d->spellCastFromScroll = endOfSpells;
      d->ammoContainedInClip = 0;
      d->meleeHitChanceMod = 0;
      d->meleeAbilityUsed = ability_accuracyMelee;
      d->propAppliedOnMelee = NULL;
      d->meleeCausesKnockBack = false;
      d->rangedCausesKnockBack = false;
      d->meleeDmgType = dmgType_physical;
      d->rangedHitChanceMod = 0;
      d->rangedDmgLabelOverRide = "";
      d->rangedAbilityUsed = ability_accuracyRanged;
      d->rangedAmmoTypeUsed = item_empty;
      d->rangedDmgType = dmgType_physical;
      d->rangedHasInfiniteAmmo = false;
      d->rangedMissileGlyph = 'X';
      d->rangedMissileTile = tile_projectileStandardFrontSlash;
      d->rangedMissileColor = clrWhite;
      d->rangedMissileLeavesTrail = false;
      d->rangedMissileLeavesSmoke = false;
      d->rangedSoundMessage = "";
      d->rangedSoundIsLoud = false;
      d->landOnHardSurfaceSoundMessage = "I hear a thudding sound.";
      d->propAppliedOnRanged = NULL;
      d->isExplosive = false;
      d->armorData = ArmorData();
      d->nativeRooms.resize(0);
      d->featuresCanBeFoundIn.resize(0);
    } break;

    case itemData_meleeWpn: {
      resetData(d, itemData_general);
      d->isStackable = false;
      d->itemWeight = itemWeight_medium;
      d->glyph = '(';
      d->color = clrWhite;
      d->primaryAttackMode = primaryAttackMode_melee;
      d->isMeleeWeapon = true;
    } break;

    case itemData_meleeWpnIntr: {
      resetData(d, itemData_meleeWpn);
      d->isIntrinsic = true;
      d->spawnStandardMinDLVL = -1;
      d->spawnStandardMaxDLVL = -1;
    } break;

    case itemData_rangedWpn: {
      resetData(d, itemData_general);
      d->isStackable = false;
      d->itemWeight = itemWeight_medium;
      d->glyph = '}';
      d->color = clrWhite;
      d->isMeleeWeapon = true;
      d->meleeDmg = pair<int, int>(1, 6);
      d->primaryAttackMode = primaryAttackMode_ranged;
      d->isRangedWeapon = true;
      d->rangedMissileGlyph = '/';
      d->rangedMissileColor = clrWhite;
      d->spawnStandardMaxDLVL = FIRST_CAVERN_LEVEL - 1;
      d->rangedSoundIsLoud = true;
    } break;

    case itemData_rangedWpnIntr: {
      resetData(d, itemData_rangedWpn);
      d->isIntrinsic = true;
      d->rangedHasInfiniteAmmo = true;
      d->spawnStandardMinDLVL = -1;
      d->spawnStandardMaxDLVL = -1;
      d->isMeleeWeapon = false;
      d->rangedMissileGlyph = '*';
      d->rangedSoundIsLoud = false;
    } break;

    case itemData_missileWeapon: {
      resetData(d, itemData_general);
      d->itemWeight = itemWeight_extraLight;
      d->isStackable = true;
      d->isMissileWeapon = true;
      d->spawnStandardMaxDLVL = FIRST_CAVERN_LEVEL - 1;
      d->rangedSoundIsLoud = false;
    } break;

    case itemData_ammo: {
      resetData(d, itemData_general);
      d->itemWeight = itemWeight_extraLight;
      d->glyph = '{';
      d->color = clrWhite;
      d->tile = tile_ammo;
      d->isAmmo = true;
      d->spawnStandardMaxDLVL = FIRST_CAVERN_LEVEL - 1;
    } break;

    case itemData_ammoClip: {
      resetData(d, itemData_ammo);
      d->itemWeight = itemWeight_light;
      d->isStackable = false;
      d->isAmmoClip = true;
      d->spawnStandardMaxDLVL = FIRST_CAVERN_LEVEL - 1;
    } break;

    case itemData_scroll: {
      resetData(d, itemData_general);
      d->itemValue = itemValue_minorTreasure;
      d->chanceToIncludeInSpawnList = 40;
      d->itemWeight = itemWeight_none;
      d->isIdentified = false;
      d->glyph = '?';
      d->color = clrWhite;
      d->tile = tile_scroll;
      d->isReadable = true;
      d->isScroll = true;
      d->maxStackSizeAtSpawn = 1;
      d->landOnHardSurfaceSoundMessage = "";
      addFeatureFoundIn(d, feature_chest);
      addFeatureFoundIn(d, feature_tomb);
      addFeatureFoundIn(d, feature_cabinet, 25);
      addFeatureFoundIn(d, feature_cocoon, 25);
      eng->scrollNameHandler->setFalseScrollName(d);
    } break;

    case itemData_potion: {
      resetData(d, itemData_general);
      d->itemValue = itemValue_minorTreasure;
      d->chanceToIncludeInSpawnList = 55;
      d->itemWeight = itemWeight_light;
      d->isIdentified = false;
      d->glyph = '!';
      d->tile = tile_potion;
      d->isQuaffable = true;
      d->isMissileWeapon = true;
      d->missileHitChanceMod = -5;
      d->missileDmg = DiceParam(1, 3, 0);
      d->maxStackSizeAtSpawn = 1;
      d->landOnHardSurfaceSoundMessage = "";
      addFeatureFoundIn(d, feature_chest);
      addFeatureFoundIn(d, feature_tomb);
      addFeatureFoundIn(d, feature_cabinet, 25);
      addFeatureFoundIn(d, feature_cocoon, 25);
      eng->potionNameHandler->setColorAndFalseName(d);
    } break;

    case itemData_device: {
      resetData(d, itemData_general);
      d->isDevice = true;
      d->chanceToIncludeInSpawnList = 5;
      d->itemWeight = itemWeight_light;
      d->isIdentified = true;
      d->glyph = '~';
      d->tile = tile_device1;
      d->isStackable = false;
      d->spawnStandardMinDLVL = 4;
      d->spawnStandardMaxDLVL = 999;
      d->landOnHardSurfaceSoundMessage = "I hear a clanking sound.";
    } break;

    case itemData_armor: {
      resetData(d, itemData_general);
      d->itemWeight = itemWeight_heavy;
      d->glyph = '[';
      d->tile = tile_armor;
      d->isArmor = true;
      d->isStackable = false;
    } break;

    case itemData_explosive: {
      resetData(d, itemData_general);
      d->itemWeight = itemWeight_light;
      d->isExplosive = true;
      d->glyph = '-';
      d->maxStackSizeAtSpawn = 2;
      d->landOnHardSurfaceSoundMessage = "";
    } break;

    default: {} break;
  }
}

void ItemDataHandler::addFeatureFoundIn(ItemData* const itemData,
                                        const Feature_t featureId,
                                        const int CHANCE_TO_INCLUDE) const {
  itemData->featuresCanBeFoundIn.push_back(
    pair<Feature_t, int>(featureId, CHANCE_TO_INCLUDE));
}

void ItemDataHandler::setDmgFromMonsterData(
  ItemData& itemData, const ActorData& actorData) const {

  itemData.meleeDmg  = pair<int, int>(1, actorData.dmgMelee);
  itemData.rangedDmg = DiceParam(1, actorData.dmgRanged, 0);
}

//------------------------------- LIST OF ITEMS
void ItemDataHandler::initDataList() {
  ItemData* d = NULL;

  d = new ItemData(item_trapezohedron);
  resetData(d, itemData_general);
  d->name = ItemName("Shining Trapezohedron", "Shining Trapezohedrons", "The Shining Trapezohedron");
  d->spawnStandardMinDLVL = -1;
  d->spawnStandardMaxDLVL = -1;
  d->isStackable = false;
  d->glyph = '*';
  d->color = clrRedLgt;
  d->tile = tile_trapezohedron;
  dataList[d->id] = d;

  d = new ItemData(item_sawedOff);
  resetData(d, itemData_rangedWpn);
  d->name = ItemName("Sawed-off Shotgun", "Sawed-off shotguns", "a Sawed-off Shotgun");
  d->itemWeight = itemWeight_medium;
  d->tile = tile_shotgun;
  d->isShotgun = true;
  d->meleeAttackMessages = ItemAttackMessages("strike", "strikes me with a shotgun");
  d->rangedDmg = DiceParam(8, 3);
  d->rangedAmmoTypeUsed = item_shotgunShell;
  d->rangedAttackMessages = ItemAttackMessages("fire", "fires a shotgun");
  d->rangedSoundMessage = "I hear a shotgun blast.";
//  d->rangedAbilityUsed = ability_accuracyRanged;
//  d->meleeAbilityUsed = ability_accuracyMelee;
//  d->reloadAudio = audio_shotgun_load_shell;
  addFeatureFoundIn(d, feature_chest);
  addFeatureFoundIn(d, feature_cabinet);
  addFeatureFoundIn(d, feature_cocoon);
  dataList[d->id] = d;

  d = new ItemData(item_pumpShotgun);
  resetData(d, itemData_rangedWpn);
  d->name = ItemName("Pump Shotgun", "Pump shotguns", "a Pump Shotgun");
  d->itemWeight = itemWeight_medium;
  d->tile = tile_shotgun;
  d->isShotgun = true;
  d->meleeAttackMessages = ItemAttackMessages("strike", "strikes me with a shotgun");
  d->rangedDmg = DiceParam(6, 3);
  d->rangedAmmoTypeUsed = item_shotgunShell;
  d->rangedAttackMessages = ItemAttackMessages("fire", "fires a shotgun");
  d->rangedSoundMessage = "I hear a shotgun blast.";
  d->rangedAbilityUsed = ability_accuracyRanged;
  d->meleeAbilityUsed = ability_accuracyMelee;
//  d->rangedAudio = audio_shotgunPump_fire;
//  d->reloadAudio = audio_shotgun_load_shell;
  addFeatureFoundIn(d, feature_chest);
  addFeatureFoundIn(d, feature_cabinet);
  addFeatureFoundIn(d, feature_cocoon);
  dataList[d->id] = d;

  d = new ItemData(item_shotgunShell);
  resetData(d, itemData_ammo);
  d->name = ItemName("Shotgun shell", "Shotgun shells", "a shotgun shell");
  d->maxStackSizeAtSpawn = 10;
  addFeatureFoundIn(d, feature_chest);
  addFeatureFoundIn(d, feature_cabinet);
  addFeatureFoundIn(d, feature_cocoon);
  dataList[d->id] = d;

  d = new ItemData(item_incinerator);
  resetData(d, itemData_rangedWpn);
  d->name = ItemName("Incinerator", "Incinerators", "an Incinerator");
  d->itemWeight = itemWeight_heavy;
  d->tile = tile_incinerator;
  d->meleeAttackMessages = ItemAttackMessages("strike", "strikes me with an Incinerator");
  d->rangedDmg = DiceParam(1, 3);
  d->rangedAbilityUsed = ability_accuracyRanged;
  d->meleeAbilityUsed = ability_accuracyMelee;
  d->rangedAmmoTypeUsed = item_napalmCartridge;
  d->rangedAttackMessages = ItemAttackMessages("fire", "fires an incinerator");
  d->rangedSoundMessage = "I hear the blast of a launched missile.";
  d->rangedMissileGlyph = '*';
  d->rangedMissileColor = clrRedLgt;
  d->spawnStandardMinDLVL = 10;
  d->rangedDmgLabelOverRide = "?";
  addFeatureFoundIn(d, feature_chest, 25);
  addFeatureFoundIn(d, feature_cabinet, 25);
  addFeatureFoundIn(d, feature_cocoon, 25);
  dataList[d->id] = d;

  d = new ItemData(item_napalmCartridge);
  resetData(d, itemData_ammoClip);
  d->name = ItemName("Napalm Cartridge", "Napalm Cartridges", "a Napalm Cartridge");
  d->itemWeight = itemWeight_light;
  d->ammoContainedInClip = 3;
  d->spawnStandardMinDLVL = 8;
  d->maxStackSizeAtSpawn = 1;
  addFeatureFoundIn(d, feature_chest, 25);
  addFeatureFoundIn(d, feature_cabinet, 25);
  addFeatureFoundIn(d, feature_cocoon, 25);
  dataList[d->id] = d;

  d = new ItemData(item_machineGun);
  resetData(d, itemData_rangedWpn);
  d->name = ItemName("Tommy Gun", "Tommy Guns", "a Tommy Gun");
  d->itemWeight = itemWeight_medium;
  d->tile = tile_tommyGun;
  d->meleeAttackMessages = ItemAttackMessages("strike", "strikes me with a Tommy Gun");
  d->isMachineGun = true;
  d->rangedDmg = DiceParam(2, 2, 2);
  d->rangedHitChanceMod = -10;
  d->rangedAbilityUsed = ability_accuracyRanged;
  d->meleeAbilityUsed = ability_accuracyMelee;
  d->rangedAmmoTypeUsed = item_drumOfBullets;
  d->rangedAttackMessages = ItemAttackMessages("fire", "fires a Tommy Gun");
  d->rangedSoundMessage = "I hear the burst of a machine gun.";
//  d->rangedAudio = audio_tommygun_fire;
  addFeatureFoundIn(d, feature_chest);
  addFeatureFoundIn(d, feature_cabinet);
  addFeatureFoundIn(d, feature_cocoon);
  dataList[d->id] = d;

  d = new ItemData(item_drumOfBullets);
  resetData(d, itemData_ammoClip);
  d->name = ItemName("Drum of .45 ACP", "Drums of .45 ACP", "a Drum of .45 ACP");
  d->ammoContainedInClip = 50;
  addFeatureFoundIn(d, feature_chest);
  addFeatureFoundIn(d, feature_cabinet);
  addFeatureFoundIn(d, feature_cocoon);
  dataList[d->id] = d;

  d = new ItemData(item_pistol);
  resetData(d, itemData_rangedWpn);
  d->name = ItemName("M1911 Colt", "M1911 Colt", "an M1911 Colt");
  d->itemWeight = itemWeight_light;
  d->tile = tile_pistol;
  d->rangedDmg = DiceParam(1, 8, 4);
  d->rangedAmmoTypeUsed = item_pistolClip;
  d->rangedAbilityUsed = ability_accuracyRanged;
  d->meleeAbilityUsed = ability_accuracyMelee;
  d->meleeAttackMessages = ItemAttackMessages("strike", "strikes me with a pistol");
  d->rangedAttackMessages = ItemAttackMessages("fire", "fires a pistol");
  d->rangedSoundMessage = "I hear a pistol being fired.";
//  d->rangedAudio = audio_pistol_fire;
//  d->reloadAudio = audio_pistol_reload;
  addFeatureFoundIn(d, feature_chest);
  addFeatureFoundIn(d, feature_cabinet);
  addFeatureFoundIn(d, feature_cocoon);
  dataList[d->id] = d;

  d = new ItemData(item_flareGun);
  resetData(d, itemData_rangedWpn);
  d->name = ItemName("Flare Gun", "Flare Gun", "a Flare Gun");
  d->itemWeight = itemWeight_light;
  d->tile = tile_flareGun;
  d->rangedDmg = DiceParam(1, 3, 0);
  d->rangedDmgLabelOverRide = "?";
  d->rangedAmmoTypeUsed = item_flare;
  d->rangedAbilityUsed = ability_accuracyRanged;
  d->meleeAbilityUsed = ability_accuracyMelee;
  d->meleeAttackMessages = ItemAttackMessages("strike", "strikes me with a flare gun");
  d->rangedAttackMessages = ItemAttackMessages("fire", "fires a flare gun");
  d->rangedSoundMessage = "I hear a flare gun being fired.";
  d->propAppliedOnRanged = new PropFlared(eng, propTurnsStandard);
  addFeatureFoundIn(d, feature_chest);
  addFeatureFoundIn(d, feature_cabinet);
  addFeatureFoundIn(d, feature_cocoon);
  dataList[d->id] = d;

  d = new ItemData(item_pistolClip);
  resetData(d, itemData_ammoClip);
  d->name = ItemName(".45ACP Colt cartridge", ".45ACP Colt cartridges", "a .45ACP Colt cartridge");
  d->ammoContainedInClip = 7;
  addFeatureFoundIn(d, feature_chest);
  addFeatureFoundIn(d, feature_cabinet);
  addFeatureFoundIn(d, feature_cocoon);
  dataList[d->id] = d;

  d = new ItemData(item_teslaCanon);
  resetData(d, itemData_rangedWpn);
  d->name = ItemName("Tesla Canon", "Tesla Canons", "a Tesla Canon");
  d->itemWeight = itemWeight_medium;
  d->tile = tile_teslaCannon;
  d->meleeAttackMessages = ItemAttackMessages("strike", "strikes me with a Tesla Cannon");
  d->isMachineGun = true;
  d->rangedHitChanceMod = -15;
  d->rangedAbilityUsed = ability_accuracyRanged;
  d->meleeAbilityUsed = ability_accuracyMelee;
  d->rangedDmg = DiceParam(2, 3, 3);
  d->rangedDmgType = dmgType_electric;
  d->rangedAmmoTypeUsed = item_teslaCanister;
  d->rangedAttackMessages = ItemAttackMessages("fire", "fires a Tesla Cannon");
  d->rangedSoundMessage = "I hear loud electric crackle.";
  d->rangedMissileGlyph = '*';
  d->rangedMissileColor = clrYellow;
  d->spawnStandardMinDLVL = 7;
  addFeatureFoundIn(d, feature_chest, 50);
  addFeatureFoundIn(d, feature_cabinet, 50);
  addFeatureFoundIn(d, feature_cocoon, 50);
  dataList[d->id] = d;

  d = new ItemData(item_teslaCanister);
  resetData(d, itemData_ammoClip);
  d->name = ItemName("Nuclear battery", "Nuclear batteries", "a Nuclear battery");
  d->ammoContainedInClip = 30;
  d->spawnStandardMinDLVL = 6;
  addFeatureFoundIn(d, feature_chest, 50);
  addFeatureFoundIn(d, feature_cabinet, 50);
  addFeatureFoundIn(d, feature_cocoon, 50);
  dataList[d->id] = d;

  d = new ItemData(item_spikeGun);
  resetData(d, itemData_rangedWpn);
  d->name = ItemName("Spike Gun", "Spike Guns", "a Spike Gun");
  d->itemWeight = itemWeight_medium;
  d->tile = tile_tommyGun;
  d->color = clrBlueLgt;
  d->meleeAttackMessages = ItemAttackMessages("strike", "strikes me with a Spike Gun");
  d->isMachineGun = false;
  d->rangedHitChanceMod = 0;
  d->rangedAbilityUsed = ability_accuracyRanged;
  d->meleeAbilityUsed = ability_accuracyMelee;
  d->rangedDmg = DiceParam(1, 7, 0);
  d->rangedDmgType = dmgType_physical;
  d->rangedCausesKnockBack = true;
  d->rangedAmmoTypeUsed = item_ironSpike;
  d->rangedAttackMessages = ItemAttackMessages("fire", "fires a Spike Gun");
  d->rangedSoundMessage = "I hear a very crude gun being fired.";
  d->rangedMissileGlyph = '/';
  d->rangedMissileColor = clrGray;
  d->spawnStandardMinDLVL = 4;
  addFeatureFoundIn(d, feature_chest, 50);
  addFeatureFoundIn(d, feature_cabinet, 50);
  addFeatureFoundIn(d, feature_cocoon, 50);
  dataList[d->id] = d;

  d = new ItemData(item_dynamite);
  resetData(d, itemData_explosive);
  d->name = ItemName("Dynamite", "Sticks of Dynamite", "a Stick of Dynamite");
  d->itemWeight = itemWeight_light;
  d->tile = tile_dynamite;
  d->color = clrRedLgt;
  addFeatureFoundIn(d, feature_chest);
  addFeatureFoundIn(d, feature_cabinet);
  addFeatureFoundIn(d, feature_cocoon);
  dataList[d->id] = d;

  d = new ItemData(item_flare);
  resetData(d, itemData_explosive);
  d->name = ItemName("Flare", "Flares", "a Flare");
  d->itemWeight = itemWeight_light;
  d->tile = tile_flare;
  d->color = clrGray;
  d->isAmmo = true;
  addFeatureFoundIn(d, feature_chest);
  addFeatureFoundIn(d, feature_cabinet);
  addFeatureFoundIn(d, feature_cocoon);
  dataList[d->id] = d;

  d = new ItemData(item_molotov);
  resetData(d, itemData_explosive);
  d->name = ItemName("Molotov Cocktail", "Molotov Cocktails", "a Molotov Cocktail");
  d->itemWeight = itemWeight_light;
  d->tile = tile_molotov;
  d->color = clrWhite;
  addFeatureFoundIn(d, feature_chest);
  addFeatureFoundIn(d, feature_cabinet);
  addFeatureFoundIn(d, feature_cocoon);
  dataList[d->id] = d;

  d = new ItemData(item_throwingKnife);
  resetData(d, itemData_missileWeapon);
  d->name = ItemName("Throwing Knife", "Throwing Knives", "a Throwing Knife");
  d->itemWeight = itemWeight_extraLight;
  d->tile = tile_dagger;
  d->glyph = '/';
  d->color = clrWhite;
  d->missileHitChanceMod = 0;
  d->missileDmg = DiceParam(2, 4);
  d->maxStackSizeAtSpawn = 8;
  d->landOnHardSurfaceSoundMessage = "I hear a clanking sound.";
  d->primaryAttackMode = primaryAttackMode_missile;
  addFeatureFoundIn(d, feature_chest);
  addFeatureFoundIn(d, feature_cabinet);
  addFeatureFoundIn(d, feature_cocoon);
  dataList[d->id] = d;

  d = new ItemData(item_rock);
  resetData(d, itemData_missileWeapon);
  d->name = ItemName("Rock", "Rocks", "a Rock");
  d->itemWeight = itemWeight_extraLight;
  d->tile = tile_rock;
  d->glyph = '*';
  d->color = clrGray;
  d->missileHitChanceMod = 10;
  d->missileDmg = DiceParam(1, 3);
  d->maxStackSizeAtSpawn = 6;
  d->primaryAttackMode = primaryAttackMode_missile;
  addFeatureFoundIn(d, feature_cabinet);
  addFeatureFoundIn(d, feature_cocoon);
  dataList[d->id] = d;

  d = new ItemData(item_dagger);
  resetData(d, itemData_meleeWpn);
  d->name = ItemName("Dagger", "Daggers", "a Dagger");
  d->itemWeight = itemWeight_light;
  d->tile = tile_dagger;
  d->meleeAttackMessages = ItemAttackMessages("strike", "strikes me with a Dagger");
  d->meleeDmg = pair<int, int>(1, 4);
  d->meleeHitChanceMod = 20;
  d->meleeAbilityUsed = ability_accuracyMelee;
  addFeatureFoundIn(d, feature_chest);
  addFeatureFoundIn(d, feature_cabinet);
  addFeatureFoundIn(d, feature_tomb);
  addFeatureFoundIn(d, feature_cocoon);
  dataList[d->id] = d;

  d = new ItemData(item_hatchet);
  resetData(d, itemData_meleeWpn);
  d->name = ItemName("Hatchet", "Hatchets", "a Hatchet");
  d->itemWeight = itemWeight_light;
  d->tile = tile_axe;
  d->meleeAttackMessages = ItemAttackMessages("strike", "strikes me with a Hatchet");
  d->meleeDmg = pair<int, int>(1, 5);
  d->meleeHitChanceMod = 15;
  d->meleeAbilityUsed = ability_accuracyMelee;
  d->missileHitChanceMod = -5;
  d->missileDmg = DiceParam(1, 10);
  d->isMissileWeapon = true;
  addFeatureFoundIn(d, feature_chest);
  addFeatureFoundIn(d, feature_cabinet);
  addFeatureFoundIn(d, feature_cocoon);
  dataList[d->id] = d;

  d = new ItemData(item_club);
  resetData(d, itemData_meleeWpn);
  d->name = ItemName("Club", "Clubs", "a Club");
  d->spawnStandardMinDLVL = FIRST_CAVERN_LEVEL;
  d->spawnStandardMaxDLVL = 999;
  d->itemWeight = itemWeight_medium;
  d->tile = tile_club;
  d->color = clrBrown;
  d->meleeAttackMessages = ItemAttackMessages("strike", "strikes me with a Club");
  d->meleeDmg = pair<int, int>(2, 3);
  d->meleeHitChanceMod = 10;
  d->meleeAbilityUsed = ability_accuracyMelee;
  dataList[d->id] = d;

  d = new ItemData(item_hammer);
  resetData(d, itemData_meleeWpn);
  d->name = ItemName("Hammer", "Hammers", "a Hammer");
  d->itemWeight = itemWeight_medium;
  d->tile = tile_hammer;
  d->meleeAttackMessages = ItemAttackMessages("strike", "strikes me with a Hammer");
  d->meleeDmg = pair<int, int>(2, 4);
  d->meleeHitChanceMod = 5;
  d->meleeAbilityUsed = ability_accuracyMelee;
  addFeatureFoundIn(d, feature_cabinet);
  addFeatureFoundIn(d, feature_cocoon);
  dataList[d->id] = d;

  d = new ItemData(item_machete);
  resetData(d, itemData_meleeWpn);
  d->name = ItemName("Machete", "Machetes", "a Machete");
  d->itemWeight = itemWeight_medium;
  d->tile = tile_machete;
  d->meleeAttackMessages = ItemAttackMessages("strike", "strikes me with a Machete");
  d->meleeDmg = pair<int, int>(2, 5);
  d->meleeHitChanceMod = 0;
  d->meleeAbilityUsed = ability_accuracyMelee;
  addFeatureFoundIn(d, feature_cabinet);
  addFeatureFoundIn(d, feature_cocoon);
  dataList[d->id] = d;

  d = new ItemData(item_axe);
  resetData(d, itemData_meleeWpn);
  d->name = ItemName("Axe", "Axes", "an Axe");
  d->itemWeight = itemWeight_medium;
  d->tile = tile_axe;
  d->meleeAttackMessages = ItemAttackMessages("strike", "strikes me with an axe");
  d->meleeDmg = pair<int, int>(2, 6);
  d->meleeHitChanceMod = -5;
  d->meleeAbilityUsed = ability_accuracyMelee;
  addFeatureFoundIn(d, feature_cabinet);
  addFeatureFoundIn(d, feature_tomb);
  addFeatureFoundIn(d, feature_cocoon);
  dataList[d->id] = d;

  d = new ItemData(item_pitchFork);
  resetData(d, itemData_meleeWpn);
  d->name = ItemName("Pitchfork", "Pitchforks", "a Pitchfork");
  d->itemWeight = itemWeight_heavy;
  d->tile = tile_pitchfork;
  d->meleeAttackMessages = ItemAttackMessages("strike", "strikes me with a Pitchfork");
  d->meleeDmg = pair<int, int>(3, 4);
  d->meleeHitChanceMod = -5;
  d->meleeAbilityUsed = ability_accuracyMelee;
  d->meleeCausesKnockBack = true;
  addFeatureFoundIn(d, feature_cabinet);
  addFeatureFoundIn(d, feature_cocoon);
  dataList[d->id] = d;

  d = new ItemData(item_sledgeHammer);
  resetData(d, itemData_meleeWpn);
  d->name = ItemName("Sledgehammer", "Sledgehammers", "a Sledgehammer");
  d->itemWeight = itemWeight_heavy;
  d->tile = tile_sledgeHammer;
  d->meleeAttackMessages = ItemAttackMessages("strike", "strikes me with a Sledgehammer");
  d->meleeDmg = pair<int, int>(3, 5);
  d->meleeHitChanceMod = -10;
  d->meleeAbilityUsed = ability_accuracyMelee;
  d->meleeCausesKnockBack = true;
  addFeatureFoundIn(d, feature_cabinet);
  dataList[d->id] = d;

  d = new ItemData(item_ironSpike);
  resetData(d, itemData_missileWeapon);
  d->name = ItemName("Iron Spike", "Iron Spikes", "an Iron Spike");
  d->isAmmo = true;
  d->itemWeight = itemWeight_extraLight;
  d->tile = tile_ironSpike;
  d->isStackable = true;
  d->color = clrGray;
  d->glyph = '/';
  d->missileHitChanceMod = -5;
  d->missileDmg = DiceParam(1, 3);
  d->maxStackSizeAtSpawn = 12;
  d->landOnHardSurfaceSoundMessage = "I hear a clanking sound.";
  d->primaryAttackMode = primaryAttackMode_missile;
  addFeatureFoundIn(d, feature_cabinet);
  addFeatureFoundIn(d, feature_cocoon);
  dataList[d->id] = d;

//  d = new ItemData(item_crowbar);
//  resetData(d, itemData_general);
//  d->name = ItemName("Crowbar", "Crowbars", "a Crowbar");
//  d->chanceToIncludeInSpawnList = 70;
//  d->itemWeight = itemWeight_light;
//  d->tile = tile_crowbar;
//  d->isStackable = false;
//  d->color = clrGray;
//  d->glyph = '~';
//  d->landOnHardSurfaceSoundMessage = "I hear a clanking sound.";
//  addFeatureFoundIn(d, feature_cabinet);
//  dataList[d->id] = d;
//
//  d = new ItemData(item_lockpick);
//  resetData(d, itemData_general);
//  d->name = ItemName("Lockpick", "Lockpicks", "a Lockpick");
//  d->itemWeight = itemWeight_extraLight;
//  d->tile = tile_lockpick;
//  d->isStackable = true;
//  d->maxStackSizeAtSpawn = 1;
//  d->color = clrWhite;
//  d->glyph = '~';
//  d->landOnHardSurfaceSoundMessage = "I clinking sound.";
//  addFeatureFoundIn(d, feature_cabinet);
//  dataList[d->id] = d;

  d = new ItemData(item_playerKick);
  resetData(d, itemData_meleeWpnIntr);
  d->meleeAttackMessages = ItemAttackMessages("kick", "");
  d->meleeAbilityUsed = ability_accuracyMelee;
  d->meleeHitChanceMod = 20;
  d->meleeDmg = pair<int, int>(1, 3);
  d->meleeCausesKnockBack = true;
  dataList[d->id] = d;

  d = new ItemData(item_playerStomp);
  resetData(d, itemData_meleeWpnIntr);
  d->meleeAttackMessages = ItemAttackMessages("stomp", "");
  d->meleeAbilityUsed = ability_accuracyMelee;
  d->meleeHitChanceMod = 20;
  d->meleeDmg = pair<int, int>(1, 3);
  d->meleeCausesKnockBack = false;
  dataList[d->id] = d;

  d = new ItemData(item_playerPunch);
  resetData(d, itemData_meleeWpnIntr);
  d->meleeAttackMessages = ItemAttackMessages("punch", "");
  d->meleeAbilityUsed = ability_accuracyMelee;
  d->meleeHitChanceMod = 25;
  d->meleeDmg = pair<int, int>(1, 2);
  dataList[d->id] = d;

  d = new ItemData(item_zombieClaw);
  resetData(d, itemData_meleeWpnIntr);
  d->meleeAttackMessages = ItemAttackMessages("", "claws me");
  setDmgFromMonsterData(*d, eng->actorDataHandler->dataList[actor_zombie]);
  dataList[d->id] = d;

  d = new ItemData(item_zombieClawDiseased);
  resetData(d, itemData_meleeWpnIntr);
  d->meleeAttackMessages = ItemAttackMessages("", "claws me");
  setDmgFromMonsterData(*d, eng->actorDataHandler->dataList[actor_zombie]);
  d->propAppliedOnMelee = new PropDiseased(eng, propTurnsStandard);
  dataList[d->id] = d;

  d = new ItemData(item_zombieAxe);
  resetData(d, itemData_meleeWpnIntr);
  d->meleeAttackMessages = ItemAttackMessages("", "chops me with a rusty axe");
  setDmgFromMonsterData(*d, eng->actorDataHandler->dataList[actor_zombieAxe]);
  dataList[d->id] = d;

  d = new ItemData(item_bloatedZombiePunch);
  resetData(d, itemData_meleeWpnIntr);
  d->meleeAttackMessages = ItemAttackMessages("", "mauls me");
  setDmgFromMonsterData(
    *d, eng->actorDataHandler->dataList[actor_bloatedZombie]);
  d->meleeCausesKnockBack = true;
  dataList[d->id] = d;

  d = new ItemData(item_bloatedZombieSpit);
  resetData(d, itemData_rangedWpnIntr);
  d->rangedAttackMessages = ItemAttackMessages("", "spits pus at me");
  setDmgFromMonsterData(*d, eng->actorDataHandler->dataList[actor_zombieAxe]);
  d->rangedSoundMessage = "I hear spitting.";
  d->rangedMissileColor = clrGreenLgt;
  d->rangedDmgType = dmgType_acid;
  d->rangedMissileGlyph = '*';
  dataList[d->id] = d;

  d = new ItemData(item_ratBite);
  resetData(d, itemData_meleeWpnIntr);
  d->meleeAttackMessages = ItemAttackMessages("", "bites me");
  setDmgFromMonsterData(*d, eng->actorDataHandler->dataList[actor_rat]);
  dataList[d->id] = d;

  d = new ItemData(item_ratBiteDiseased);
  resetData(d, itemData_meleeWpnIntr);
  d->meleeAttackMessages = ItemAttackMessages("", "bites me");
  setDmgFromMonsterData(*d, eng->actorDataHandler->dataList[actor_rat]);
  d->propAppliedOnMelee = new PropDiseased(eng, propTurnsStandard);
  dataList[d->id] = d;

  d = new ItemData(item_ratThingBite);
  resetData(d, itemData_meleeWpnIntr);
  d->meleeAttackMessages = ItemAttackMessages("", "bites me");
  setDmgFromMonsterData(*d, eng->actorDataHandler->dataList[actor_ratThing]);
  dataList[d->id] = d;

  d = new ItemData(item_wormMassBite);
  resetData(d, itemData_meleeWpnIntr);
  d->meleeAttackMessages = ItemAttackMessages("", "bites me");
  setDmgFromMonsterData(*d, eng->actorDataHandler->dataList[actor_wormMass]);
  dataList[d->id] = d;

  d = new ItemData(item_wolfBite);
  resetData(d, itemData_meleeWpnIntr);
  d->meleeAttackMessages = ItemAttackMessages("", "bites me");
  setDmgFromMonsterData(*d, eng->actorDataHandler->dataList[actor_wolf]);
  dataList[d->id] = d;

  d = new ItemData(item_greenSpiderBite);
  resetData(d, itemData_meleeWpnIntr);
  d->meleeAttackMessages = ItemAttackMessages("", "bites me");
  setDmgFromMonsterData(
    *d, eng->actorDataHandler->dataList[actor_greenSpider]);
  d->propAppliedOnMelee = new PropBlind(eng, propTurnsStandard, 4);
  dataList[d->id] = d;

  d = new ItemData(item_whiteSpiderBite);
  resetData(d, itemData_meleeWpnIntr);
  d->meleeAttackMessages = ItemAttackMessages("", "bites me");
  setDmgFromMonsterData(
    *d, eng->actorDataHandler->dataList[actor_whiteSpider]);
  d->propAppliedOnMelee = new PropParalyzed(eng, propTurnsSpecified, 3);
  dataList[d->id] = d;

  d = new ItemData(item_redSpiderBite);
  resetData(d, itemData_meleeWpnIntr);
  d->meleeAttackMessages = ItemAttackMessages("", "bites me");
  setDmgFromMonsterData(*d, eng->actorDataHandler->dataList[actor_redSpider]);
  d->propAppliedOnMelee = new PropWeakened(eng, propTurnsStandard);
  dataList[d->id] = d;

  d = new ItemData(item_shadowSpiderBite);
  resetData(d, itemData_meleeWpnIntr);
  d->meleeAttackMessages = ItemAttackMessages("", "bites me");
  setDmgFromMonsterData(
    *d, eng->actorDataHandler->dataList[actor_shadowSpider]);
  dataList[d->id] = d;

  d = new ItemData(item_lengSpiderBite);
  resetData(d, itemData_meleeWpnIntr);
  d->meleeAttackMessages = ItemAttackMessages("", "bites me");
  setDmgFromMonsterData(
    *d, eng->actorDataHandler->dataList[actor_lengSpider]);
  dataList[d->id] = d;

  d = new ItemData(item_fireHoundBreath);
  resetData(d, itemData_rangedWpnIntr);
  d->rangedAttackMessages = ItemAttackMessages("", "breaths fire at me");
  d->rangedSoundMessage = "I hear a burst of flames.";
  setDmgFromMonsterData(*d, eng->actorDataHandler->dataList[actor_fireHound]);
  d->propAppliedOnRanged = new PropBurning(eng, propTurnsStandard);
  d->rangedMissileColor = clrRedLgt;
  d->rangedMissileGlyph = '*';
  d->rangedMissileLeavesTrail = true;
  d->rangedMissileLeavesSmoke = true;
  d->rangedDmgType = dmgType_fire;
  dataList[d->id] = d;

  d = new ItemData(item_fireHoundBite);
  resetData(d, itemData_meleeWpnIntr);
  d->meleeAttackMessages = ItemAttackMessages("", "bites me");
  setDmgFromMonsterData(*d, eng->actorDataHandler->dataList[actor_fireHound]);
  dataList[d->id] = d;

  d = new ItemData(item_dustVortexEngulf);
  resetData(d, itemData_meleeWpnIntr);
  d->meleeAttackMessages = ItemAttackMessages("", "engulfs me");
  setDmgFromMonsterData(
    *d, eng->actorDataHandler->dataList[actor_dustVortex]);
  d->propAppliedOnMelee = new PropBlind(eng, propTurnsStandard);
  dataList[d->id] = d;

  d = new ItemData(item_fireVortexEngulf);
  resetData(d, itemData_meleeWpnIntr);
  d->meleeAttackMessages = ItemAttackMessages("", "engulfs me");
  setDmgFromMonsterData(
    *d, eng->actorDataHandler->dataList[actor_fireVortex]);
  d->propAppliedOnMelee = new PropBurning(eng, propTurnsStandard);
  dataList[d->id] = d;

  d = new ItemData(item_ghostClaw);
  resetData(d, itemData_meleeWpnIntr);
  d->meleeAttackMessages = ItemAttackMessages("", "claws me");
  setDmgFromMonsterData(*d, eng->actorDataHandler->dataList[actor_ghost]);
  d->propAppliedOnMelee = new PropTerrified(eng, propTurnsSpecified, 4);
  dataList[d->id] = d;

  d = new ItemData(item_phantasmSickle);
  resetData(d, itemData_meleeWpnIntr);
  d->meleeAttackMessages = ItemAttackMessages("", "slices me with a sickle");
  setDmgFromMonsterData(*d, eng->actorDataHandler->dataList[actor_phantasm]);
  d->propAppliedOnMelee = new PropTerrified(eng, propTurnsSpecified, 4);
  dataList[d->id] = d;

  d = new ItemData(item_wraithClaw);
  resetData(d, itemData_meleeWpnIntr);
  d->meleeAttackMessages = ItemAttackMessages("", "claws me");
  setDmgFromMonsterData(*d, eng->actorDataHandler->dataList[actor_wraith]);
  d->propAppliedOnMelee = new PropTerrified(eng, propTurnsSpecified, 4);
  dataList[d->id] = d;

  d = new ItemData(item_giantBatBite);
  resetData(d, itemData_meleeWpnIntr);
  d->meleeAttackMessages = ItemAttackMessages("", "bites me");
  setDmgFromMonsterData(*d, eng->actorDataHandler->dataList[actor_giantBat]);
  dataList[d->id] = d;

  d = new ItemData(item_miGoElectricGun);
  resetData(d, itemData_rangedWpnIntr);
  d->rangedMissileLeavesTrail = true;
  d->rangedMissileColor = clrYellow;
  d->rangedMissileGlyph = '/';
  d->rangedAttackMessages = ItemAttackMessages("", "fires an electric gun");
  d->rangedDmgType = dmgType_electric;
  d->propAppliedOnRanged = new PropParalyzed(eng, propTurnsSpecified, 2);
  d->rangedSoundMessage = "I hear a bolt of electricity.";
  setDmgFromMonsterData(*d, eng->actorDataHandler->dataList[actor_miGo]);
  d->rangedSoundIsLoud = true;
  dataList[d->id] = d;

  d = new ItemData(item_ghoulClaw);
  resetData(d, itemData_meleeWpnIntr);
  d->meleeAttackMessages = ItemAttackMessages("", "claws me");
  setDmgFromMonsterData(*d, eng->actorDataHandler->dataList[actor_ghoul]);
  d->propAppliedOnMelee = new PropDiseased(eng, propTurnsStandard);
  dataList[d->id] = d;

  d = new ItemData(item_shadowClaw);
  resetData(d, itemData_meleeWpnIntr);
  d->meleeAttackMessages = ItemAttackMessages("", "claws me");
  setDmgFromMonsterData(*d, eng->actorDataHandler->dataList[actor_shadow]);
  dataList[d->id] = d;

  d = new ItemData(item_byakheeClaw);
  resetData(d, itemData_meleeWpnIntr);
  d->meleeAttackMessages = ItemAttackMessages("", "claws me");
  setDmgFromMonsterData(*d, eng->actorDataHandler->dataList[actor_byakhee]);
  dataList[d->id] = d;

  d = new ItemData(item_giantMantisClaw);
  resetData(d, itemData_meleeWpnIntr);
  d->meleeAttackMessages = ItemAttackMessages("", "claws me");
  setDmgFromMonsterData(
    *d, eng->actorDataHandler->dataList[actor_giantMantis]);
  dataList[d->id] = d;

  d = new ItemData(item_giantLocustBite);
  resetData(d, itemData_meleeWpnIntr);
  d->meleeAttackMessages = ItemAttackMessages("", "bites me");
  setDmgFromMonsterData(
    *d, eng->actorDataHandler->dataList[actor_giantLocust]);
  dataList[d->id] = d;

  d = new ItemData(item_mummyMaul);
  resetData(d, itemData_meleeWpnIntr);
  d->meleeAttackMessages = ItemAttackMessages("", "mauls me");
  setDmgFromMonsterData(*d, eng->actorDataHandler->dataList[actor_mummy]);
  d->propAppliedOnMelee = new PropCursed(eng, propTurnsStandard);
  d->meleeCausesKnockBack = true;
  dataList[d->id] = d;

  d = new ItemData(item_deepOneJavelinAttack);
  resetData(d, itemData_rangedWpnIntr);
  d->rangedAttackMessages = ItemAttackMessages("", "throws a Javelin at me");
  setDmgFromMonsterData(*d, eng->actorDataHandler->dataList[actor_deepOne]);
  d->rangedSoundMessage = "";
  d->rangedMissileColor = clrBrown;
  d->rangedMissileGlyph = '/';
  d->rangedSoundIsLoud = false;
  dataList[d->id] = d;

  d = new ItemData(item_deepOneSpearAttack);
  resetData(d, itemData_meleeWpnIntr);
  d->meleeAttackMessages = ItemAttackMessages("", "thrusts a spear at me");
  setDmgFromMonsterData(*d, eng->actorDataHandler->dataList[actor_deepOne]);
  dataList[d->id] = d;

  d = new ItemData(item_oozeBlackSpewPus);
  resetData(d, itemData_meleeWpnIntr);
  d->meleeAttackMessages = ItemAttackMessages("", "spews pus on me");
  setDmgFromMonsterData(*d, eng->actorDataHandler->dataList[actor_oozeBlack]);
  dataList[d->id] = d;

  d = new ItemData(item_oozeClearSpewPus);
  resetData(d, itemData_meleeWpnIntr);
  d->meleeAttackMessages = ItemAttackMessages("", "spews pus on me");
  setDmgFromMonsterData(
    *d, eng->actorDataHandler->dataList[actor_oozeClear]);
  dataList[d->id] = d;

  d = new ItemData(item_oozePutridSpewPus);
  resetData(d, itemData_meleeWpnIntr);
  d->meleeAttackMessages = ItemAttackMessages("", "spews infected pus on me");
  setDmgFromMonsterData(
    *d, eng->actorDataHandler->dataList[actor_oozePutrid]);
  d->propAppliedOnMelee = new PropDiseased(eng, propTurnsStandard);
  dataList[d->id] = d;

  d = new ItemData(item_oozePoisonSpewPus);
  resetData(d, itemData_meleeWpnIntr);
  d->meleeAttackMessages = ItemAttackMessages("", "spews poisonous pus on me");
  setDmgFromMonsterData(
    *d, eng->actorDataHandler->dataList[actor_oozePoison]);
  d->propAppliedOnMelee = new PropPoisoned(eng, propTurnsStandard);
  dataList[d->id] = d;

  d = new ItemData(item_colourOutOfSpaceTouch);
  resetData(d, itemData_meleeWpnIntr);
  d->meleeAttackMessages = ItemAttackMessages("", "touches me");
  setDmgFromMonsterData(
    *d, eng->actorDataHandler->dataList[actor_colourOutOfSpace]);
  dataList[d->id] = d;

  d = new ItemData(item_huntingHorrorBite);
  resetData(d, itemData_meleeWpnIntr);
  d->meleeAttackMessages = ItemAttackMessages("", "bites me");
  setDmgFromMonsterData(
    *d, eng->actorDataHandler->dataList[actor_huntingHorror]);
  d->propAppliedOnMelee = new PropParalyzed(eng, propTurnsStandard);
  dataList[d->id] = d;

  d = new ItemData(item_armorLeatherJacket);
  resetData(d, itemData_armor);
  d->name = ItemName("Leather Jacket", "", "a Leather Jacket");
  d->itemWeight = itemWeight_light;
  d->color = clrBrown;
  d->spawnStandardMinDLVL = 1;
  d->armorData.absorptionPoints = 1;
  d->armorData.damageToDurabilityFactor = 1.0;
  d->landOnHardSurfaceSoundMessage = "";
  addFeatureFoundIn(d, feature_cabinet);
  dataList[d->id] = d;

  d = new ItemData(item_armorIronSuit);
  resetData(d, itemData_armor);
  d->name = ItemName("Iron Suit", "", "an Iron Suit");
  d->itemWeight = itemWeight_heavy;
  d->color = clrWhite;
  d->spawnStandardMinDLVL = 2;
  d->armorData.absorptionPoints = 4;
  d->armorData.damageToDurabilityFactor = 0.5;
  d->landOnHardSurfaceSoundMessage = "I hear a crashing sound.";
  addFeatureFoundIn(d, feature_cabinet);
  dataList[d->id] = d;

  d = new ItemData(item_armorFlackJacket);
  resetData(d, itemData_armor);
  d->name = ItemName("Flak Jacket", "", "a Flak Jacket");
  d->itemWeight = itemWeight_heavy;
  d->color = clrGreen;
  d->spawnStandardMinDLVL = 3;
  d->armorData.absorptionPoints = 3;
  d->armorData.damageToDurabilityFactor = 0.5;
  d->landOnHardSurfaceSoundMessage = "I hear a thudding sound.";
  addFeatureFoundIn(d, feature_cabinet);
  dataList[d->id] = d;

  d = new ItemData(item_armorAsbestosSuit);
  resetData(d, itemData_armor);
  d->name = ItemName("Asbestos Suit", "", "an Asbestos Suit");
  d->itemWeight = itemWeight_medium;
  d->color = clrRedLgt;
  d->spawnStandardMinDLVL = 3;
  d->armorData.absorptionPoints = 1;
  d->armorData.damageToDurabilityFactor = 1.0;
  d->landOnHardSurfaceSoundMessage = "";
  addFeatureFoundIn(d, feature_cabinet);
  addFeatureFoundIn(d, feature_chest);
  dataList[d->id] = d;

  d = new ItemData(item_armorHeavyCoat);
  resetData(d, itemData_armor);
  d->name = ItemName("Heavy Coat", "", "a Heavy Coat");
  d->itemWeight = itemWeight_medium;
  d->color = clrBlueLgt;
  d->spawnStandardMinDLVL = 3;
  d->armorData.absorptionPoints = 1;
  d->armorData.damageToDurabilityFactor = 1.0;
  d->landOnHardSurfaceSoundMessage = "";
  addFeatureFoundIn(d, feature_cabinet);
  dataList[d->id] = d;

  d = new ItemData(item_scrollOfMayhem);
  resetData(d, itemData_scroll);
  d->spellCastFromScroll = spell_mayhem;
  dataList[d->id] = d;

  d = new ItemData(item_scrollOfTeleportation);
  resetData(d, itemData_scroll);
  d->spellCastFromScroll = spell_teleport;
  dataList[d->id] = d;

  d = new ItemData(item_scrollOfDescent);
  resetData(d, itemData_scroll);
  d->spellCastFromScroll = spell_descent;
  d->spawnStandardMinDLVL = 6;
  dataList[d->id] = d;

  d = new ItemData(item_scrollOfPestilence);
  resetData(d, itemData_scroll);
  d->spellCastFromScroll = spell_pestilence;
  dataList[d->id] = d;

  d = new ItemData(item_scrollOfEnfeebleEnemies);
  resetData(d, itemData_scroll);
  d->spellCastFromScroll = spell_enfeeble;
  dataList[d->id] = d;

  d = new ItemData(item_scrollOfDetectItems);
  resetData(d, itemData_scroll);
  d->spellCastFromScroll = spell_detectItems;
  dataList[d->id] = d;

  d = new ItemData(item_scrollOfDetectTraps);
  resetData(d, itemData_scroll);
  d->spellCastFromScroll = spell_detectTraps;
  dataList[d->id] = d;

  d = new ItemData(item_scrollOfIdentify);
  resetData(d, itemData_scroll);
  d->spellCastFromScroll = spell_identify;
  dataList[d->id] = d;

  d = new ItemData(item_scrollOfBlessing);
  resetData(d, itemData_scroll);
  d->spellCastFromScroll = spell_bless;
  dataList[d->id] = d;

  d = new ItemData(item_scrollOfClairvoyance);
  resetData(d, itemData_scroll);
  d->spellCastFromScroll = spell_clairvoyance;
  dataList[d->id] = d;

  d = new ItemData(item_scrollOfAzathothsBlast);
  resetData(d, itemData_scroll);
  d->spellCastFromScroll = spell_azathothsBlast;
  dataList[d->id] = d;

  d = new ItemData(item_scrollOfOpening);
  resetData(d, itemData_scroll);
  d->spellCastFromScroll = spell_opening;
  dataList[d->id] = d;

  d = new ItemData(item_thaumaturgicAlteration);
  resetData(d, itemData_scroll);
  d->spellCastFromScroll = spell_mthPower;
  d->isIntrinsic = true;
  dataList[d->id] = d;

//  d = new ItemData(item_scrollOfVoidChain);
//  resetData(d, itemData_scroll);
//  dataList[d->id] = d;

//  d = new ItemData(item_scrollOfIbnGhazisPowder);
//  resetData(d, itemData_scroll);
//  dataList[d->id] = d;

  d = new ItemData(item_potionOfHealing);
  resetData(d, itemData_potion);
  dataList[d->id] = d;

  d = new ItemData(item_potionOfSpirit);
  resetData(d, itemData_potion);
  dataList[d->id] = d;

  d = new ItemData(item_potionOfBlindness);
  resetData(d, itemData_potion);
  dataList[d->id] = d;

//  d = new ItemData(item_potionOfCorruption);
//  resetData(d, itemData_potion);
//  dataList[d->id] = d;

  d = new ItemData(item_potionOfBerserk);
  resetData(d, itemData_potion);
  dataList[d->id] = d;

//  d = new ItemData(item_potionOfStealth);
//  resetData(d, itemData_potion);
//  dataList[d->id] = d;

  d = new ItemData(item_potionOfFortitude);
  resetData(d, itemData_potion);
  dataList[d->id] = d;

//  d = new ItemData(item_potionOfToughness);
//  resetData(d, itemData_potion);
//  dataList[d->id] = d;

  d = new ItemData(item_potionOfParalyzation);
  resetData(d, itemData_potion);
  dataList[d->id] = d;

  d = new ItemData(item_potionOfConfusion);
  resetData(d, itemData_potion);
  dataList[d->id] = d;

//  d = new ItemData(item_potionOfSorcery);
//  resetData(d, itemData_potion);
//  dataList[d->id] = d;

  d = new ItemData(item_potionOfPoison);
  resetData(d, itemData_potion);
  dataList[d->id] = d;

  d = new ItemData(item_potionOfKnowledge);
  resetData(d, itemData_potion);
  dataList[d->id] = d;

  d = new ItemData(item_potionOfRFire);
  resetData(d, itemData_potion);
  dataList[d->id] = d;

  d = new ItemData(item_potionOfRCold);
  resetData(d, itemData_potion);
  dataList[d->id] = d;

  d = new ItemData(item_potionOfAntidote);
  resetData(d, itemData_potion);
  dataList[d->id] = d;

  d = new ItemData(item_potionOfRElec);
  resetData(d, itemData_potion);
  dataList[d->id] = d;

  d = new ItemData(item_potionOfRAcid);
  resetData(d, itemData_potion);
  dataList[d->id] = d;

  d = new ItemData(item_deviceSentry);
  resetData(d, itemData_device);
  d->name = ItemName("Sentry Device", "Sentry Devices", "a Sentry Device");
  d->itemValue = itemValue_majorTreasure;
  d->isIdentified = false;
  d->color = clrGray;
  addFeatureFoundIn(d, feature_chest, 10);
  addFeatureFoundIn(d, feature_tomb, 10);
  addFeatureFoundIn(d, feature_cocoon, 10);
  dataList[d->id] = d;

  d = new ItemData(item_deviceRepeller);
  resetData(d, itemData_device);
  d->name = ItemName("Repeller Device", "Repeller Devices", "a Repeller Device");
  d->itemValue = itemValue_majorTreasure;
  d->isIdentified = false;
  d->color = clrGray;
  addFeatureFoundIn(d, feature_chest, 10);
  addFeatureFoundIn(d, feature_tomb, 10);
  addFeatureFoundIn(d, feature_cocoon, 10);
  dataList[d->id] = d;

  d = new ItemData(item_deviceRejuvenator);
  resetData(d, itemData_device);
  d->name = ItemName("Rejuvenator Device", "Rejuvenator Devices", "a Rejuvenator Device");
  d->itemValue = itemValue_majorTreasure;
  d->isIdentified = false;
  d->color = clrGray;
  addFeatureFoundIn(d, feature_chest, 10);
  addFeatureFoundIn(d, feature_tomb, 10);
  addFeatureFoundIn(d, feature_cocoon, 10);
  dataList[d->id] = d;

  d = new ItemData(item_deviceTranslocator);
  resetData(d, itemData_device);
  d->name = ItemName("Translocator Device", "Translocator Devices", "a Translocator Device");
  d->itemValue = itemValue_majorTreasure;
  d->isIdentified = false;
  d->color = clrGray;
  addFeatureFoundIn(d, feature_chest, 10);
  addFeatureFoundIn(d, feature_tomb, 10);
  addFeatureFoundIn(d, feature_cocoon, 10);
  dataList[d->id] = d;

//  d = new ItemData(item_deviceSpellReflector);
//  resetData(d, itemData_device);
//  d->name = ItemName("Spell Reflector Device", "Spell Reflector Devices", "a Spell Reflector Device");
//  d->color = clrGray;
//  d->featuresCanBeFoundIn.push_back(feature_chest);
//  d->featuresCanBeFoundIn.push_back(feature_tomb);
//  dataList[d->id] = d;

  d = new ItemData(item_deviceElectricLantern);
  resetData(d, itemData_device);
  d->name = ItemName("Electric Lantern", "Electric Lanterns", "an Electric Lantern");
  d->spawnStandardMinDLVL = 1;
  d->spawnStandardMaxDLVL = 10;
  d->chanceToIncludeInSpawnList = 50;
  d->isIdentified = true;
  d->tile = tile_electricLantern;
  d->color = clrYellow;
  addFeatureFoundIn(d, feature_chest);
  addFeatureFoundIn(d, feature_cabinet);
  addFeatureFoundIn(d, feature_cocoon);
  dataList[d->id] = d;

  d = new ItemData(item_medicalBag);
  resetData(d, itemData_general);
  d->isMedicalBag = true;
  d->name = ItemName("Medical Bag", "Medical Bags", "a Medical Bag");
  d->itemValue = itemValue_normal;
  d->itemWeight = itemWeight_medium;
  d->spawnStandardMinDLVL = 1;
  d->spawnStandardMaxDLVL = LAST_ROOM_AND_CORRIDOR_LEVEL;
  d->isStackable = false;
  d->glyph = '~';
  d->color = clrBrownDark;
  d->tile = tile_medicalBag;
//  d->nativeRooms.resize(0);
  dataList[d->id] = d;
}

void ItemDataHandler::addSaveLines(vector<string>& lines) const {
  for(unsigned int i = 1; i < endOfItemIds; i++) {
    lines.push_back(dataList[i]->isIdentified ? "1" : "0");

    if(dataList[i]->isScroll) {
      lines.push_back(dataList[i]->isTried ? "1" : "0");
    }
  }
}

void ItemDataHandler::setParametersFromSaveLines(vector<string>& lines) {
  for(unsigned int i = 1; i < endOfItemIds; i++) {
    dataList[i]->isIdentified = lines.front() == "0" ? false : true;
    lines.erase(lines.begin());

    if(dataList[i]->isScroll) {
      dataList[i]->isTried = lines.front() == "0" ? false : true;
      lines.erase(lines.begin());
    }
  }
}

bool ItemDataHandler::isWeaponStronger(
  const ItemData& data1, const ItemData& data2, const bool IS_MELEE) {
  (void)data1;
  (void)data2;
  (void)IS_MELEE;
//  int rolls1 = 0;
//  int sides1 = 0;
//  int plus1 = 0;
//  int rolls2 = 0;
//  int sides2 = 0;
//  int plus2 = 0;
//
//  if(melee == true) {
//    rolls1 = oldDef.meleeDmg.first;
//    sides1 = oldDef.meleeDmg.second;
//    plus1 = ;
//
//    rolls2 = newDef.meleeDmg.rolls;
//    sides2 = newDef.meleeDmg.sides;
//    plus2 = newDef.meleeDmg.plus;
//  } else {
//    rolls1 = oldDef.rangedDmg.rolls;
//    sides1 = oldDef.rangedDmg.sides;
//    plus1 = oldDef.rangedDmg.plus;
//
//    rolls2 = newDef.rangedDmg.rolls;
//    sides2 = newDef.rangedDmg.sides;
//    plus2 = newDef.rangedDmg.plus;
//  }
//
//  bool newWeaponIsStronger = rolls2 * sides2 + plus2 > rolls1 * sides1 + plus1;
//
//  return newWeaponIsStronger;
  return false;
}

string ItemDataHandler::getItemRef(
  const Item& item, const ItemRef_t itemRefForm,
  const bool SKIP_EXTRA_INFO) const {
  const ItemData& d = item.getData();
  string ret = "";

  if(d.isDevice && d.isIdentified == false) {
    return itemRefForm == itemRef_plain ?
           "Strange Device" : "a Strange Device";
  }

  if(d.isStackable && item.nrItems > 1 && itemRefForm == itemRef_plural) {
    ret = intToString(item.nrItems) + " ";
    ret += d.name.name_plural;
  } else {
    ret += itemRefForm == itemRef_plain ? d.name.name : d.name.name_a;
  }

  if(d.isAmmoClip) {
    const ItemAmmoClip* const ammoItem =
      dynamic_cast<const ItemAmmoClip*>(&item);
    return ret + " {" + intToString(ammoItem->ammo) + "}";
  }

  if(d.isMedicalBag) {
    const MedicalBag* const medicalBag =
      dynamic_cast<const MedicalBag*>(&item);
    return ret + " {" + intToString(medicalBag->getNrSupplies()) + "}";
  }

//  if(d.isArmor) {
//    const Armor* armor = dynamic_cast<const Armor*>(&item);
//    const string armorDataLine = armor->getArmorDataLine(true);
//    return armorDataLine == "" ? ret : ret + " " + armorDataLine;
//  }

  if(SKIP_EXTRA_INFO == false) {
    if(d.isRangedWeapon) {
      string ammoLoadedStr = "";
      if(d.rangedHasInfiniteAmmo == false) {
        const Weapon* const w = dynamic_cast<const Weapon*>(&item);
        ammoLoadedStr = " " + intToString(w->ammoLoaded) + "/" +
                        intToString(w->ammoCapacity);
      }
      return ret + ammoLoadedStr;
    }

    if((d.isScroll || d.isQuaffable) &&
        d.isTried && d.isIdentified == false) {
      return ret + " {tried}";
    }
  }

  return ret;
}

string ItemDataHandler::getItemInterfaceRef(
  const Item& item, const bool ADD_A,
  const PrimaryAttackMode_t attackMode) const {
  const ItemData& d = item.getData();

  if(d.isDevice && d.isIdentified == false) {
    return ADD_A ? "a Strange Device" : "Strange Device";
  }

  string ret = "";

  if(d.isStackable && item.nrItems > 1) {
    ret = intToString(item.nrItems) + " " + d.name.name_plural;
  } else {
    ret = (ADD_A ? d.name.name_a : d.name.name);
  }

  const int PLAYER_RANGED_SKILL =
    eng->player->getData()->abilityVals.getVal(
      ability_accuracyRanged, true, *(eng->player));

  if(
    (attackMode == primaryAttackMode_none &&
     d.primaryAttackMode == primaryAttackMode_melee) ||
    (attackMode == primaryAttackMode_melee && d.isMeleeWeapon)) {
    const string rollsStr = intToString(d.meleeDmg.first);
    const string sidesStr = intToString(d.meleeDmg.second);
    const int PLUS = dynamic_cast<const Weapon*>(&item)->meleeDmgPlus;
    const string plusStr = PLUS ==  0 ? "" : ((PLUS > 0 ? "+" : "") +
                           intToString(PLUS));
    const int ITEM_SKILL = d.meleeHitChanceMod;
    const int PLAYER_MELEE_SKILL =
      eng->player->getData()->abilityVals.getVal(
        ability_accuracyMelee, true, *(eng->player));
    const int TOTAL_SKILL = max(0, min(100, ITEM_SKILL + PLAYER_MELEE_SKILL));
    const string skillStr = intToString(TOTAL_SKILL) + "%";
    return ret + " " + rollsStr + "d" + sidesStr + plusStr + " " + skillStr;
  }

  if(
    (attackMode == primaryAttackMode_none &&
     d.primaryAttackMode == primaryAttackMode_ranged) ||
    (attackMode == primaryAttackMode_ranged && d.isRangedWeapon)) {
    const int MULTIPL = d.isMachineGun == true ?
                        NR_MACHINEGUN_PROJECTILES : 1;
    const string rollsStr = intToString(d.rangedDmg.rolls * MULTIPL);
    const string sidesStr = intToString(d.rangedDmg.sides);
    const int PLUS = d.rangedDmg.plus * MULTIPL;
    const string plusStr = PLUS ==  0 ? "" : ((PLUS > 0 ? "+" : "") +
                           intToString(PLUS));
    const int ITEM_SKILL = d.rangedHitChanceMod;
    const int TOTAL_SKILL = max(0, min(100, ITEM_SKILL + PLAYER_RANGED_SKILL));
    const string skillStr = intToString(TOTAL_SKILL) + "%";
    string ammoLoadedStr = "";
    if(d.rangedHasInfiniteAmmo == false) {
      const Weapon* const w = dynamic_cast<const Weapon*>(&item);
      ammoLoadedStr = " " + intToString(w->ammoLoaded) + "/" +
                      intToString(w->ammoCapacity);
    }
    return ret + " " + rollsStr + "d" + sidesStr + plusStr + " " + skillStr +
           ammoLoadedStr;
  }

  if(
    (attackMode == primaryAttackMode_none &&
     d.primaryAttackMode == primaryAttackMode_missile) ||
    (attackMode == primaryAttackMode_missile && d.isMissileWeapon)) {
    const string rollsStr = intToString(d.missileDmg.rolls);
    const string sidesStr = intToString(d.missileDmg.sides);
    const int PLUS = d.missileDmg.plus;
    const string plusStr = PLUS ==  0 ? "" : ((PLUS > 0 ? "+" : "") +
                           intToString(PLUS));
    const int ITEM_SKILL = d.missileHitChanceMod;
    const int TOTAL_SKILL = max(0, min(100, ITEM_SKILL + PLAYER_RANGED_SKILL));
    const string skillStr = intToString(TOTAL_SKILL) + "%";
    return ret + " " + rollsStr + "d" + sidesStr + plusStr + " " + skillStr;
  }

  if(d.isMedicalBag) {
    const MedicalBag* const medicalBag = dynamic_cast<const MedicalBag*>(&item);
    return ret + " {" + intToString(medicalBag->getNrSupplies()) + "}";
  }

  if(d.isAmmoClip) {
    const ItemAmmoClip* const clip = dynamic_cast<const ItemAmmoClip*>(&item);
    return ret + " {" + intToString(clip->ammo) + "}";
  }

  if(d.isArmor) {
    const string armorDataLine =
      dynamic_cast<const Armor*>(&item)->getArmorDataLine(true);
    return armorDataLine == "" ? ret : ret + " " + armorDataLine;
  }

  if((d.isScroll || d.isQuaffable) && d.isTried && d.isIdentified == false) {
    return ret + " {tried}";
  }

  return ret;
}

