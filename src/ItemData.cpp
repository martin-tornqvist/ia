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
void ItemData::resetDef(ItemDef* const d,
                        ItemDefArchetypes_t const archetype) const {
  switch(archetype) {
    case itemDef_general: {
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
      d->meleeStatusEffect = NULL;
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
      d->rangedAudio = audio_none;
      d->meleeAudio = audio_none;
      d->reloadAudio = audio_none;
      d->rangedSoundMessage = "";
      d->rangedSoundIsLoud = false;
      d->landOnHardSurfaceSoundMessage = "I hear a thudding sound.";
      d->rangedStatusEffect = NULL;
      d->isExplosive = false;
      d->armorData = ArmorData();
      d->nativeRooms.resize(0);
      d->featuresCanBeFoundIn.resize(0);
    } break;

    case itemDef_meleeWpn: {
      resetDef(d, itemDef_general);
      d->isStackable = false;
      d->itemWeight = itemWeight_medium;
      d->glyph = '(';
      d->color = clrWhite;
      d->primaryAttackMode = primaryAttackMode_melee;
      d->isMeleeWeapon = true;
    } break;

    case itemDef_meleeWpnIntr: {
      resetDef(d, itemDef_meleeWpn);
      d->isIntrinsic = true;
      d->spawnStandardMinDLVL = -1;
      d->spawnStandardMaxDLVL = -1;
    } break;

    case itemDef_rangedWpn: {
      resetDef(d, itemDef_general);
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

    case itemDef_rangedWpnIntr: {
      resetDef(d, itemDef_rangedWpn);
      d->isIntrinsic = true;
      d->rangedHasInfiniteAmmo = true;
      d->spawnStandardMinDLVL = -1;
      d->spawnStandardMaxDLVL = -1;
      d->isMeleeWeapon = false;
      d->rangedMissileGlyph = '*';
      d->rangedSoundIsLoud = false;
    } break;

    case itemDef_missileWeapon: {
      resetDef(d, itemDef_general);
      d->itemWeight = itemWeight_extraLight;
      d->isStackable = true;
      d->isMissileWeapon = true;
      d->spawnStandardMaxDLVL = FIRST_CAVERN_LEVEL - 1;
      d->rangedSoundIsLoud = false;
    } break;

    case itemDef_ammo: {
      resetDef(d, itemDef_general);
      d->itemWeight = itemWeight_extraLight;
      d->glyph = '{';
      d->color = clrWhite;
      d->tile = tile_ammo;
      d->isAmmo = true;
      d->spawnStandardMaxDLVL = FIRST_CAVERN_LEVEL - 1;
    } break;

    case itemDef_ammoClip: {
      resetDef(d, itemDef_ammo);
      d->itemWeight = itemWeight_light;
      d->isStackable = false;
      d->isAmmoClip = true;
      d->spawnStandardMaxDLVL = FIRST_CAVERN_LEVEL - 1;
    } break;

    case itemDef_scroll: {
      resetDef(d, itemDef_general);
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

    case itemDef_potion: {
      resetDef(d, itemDef_general);
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

    case itemDef_device: {
      resetDef(d, itemDef_general);
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

    case itemDef_armor: {
      resetDef(d, itemDef_general);
      d->itemWeight = itemWeight_heavy;
      d->glyph = '[';
      d->tile = tile_armor;
      d->isArmor = true;
      d->isStackable = false;
    } break;

    case itemDef_explosive: {
      resetDef(d, itemDef_general);
      d->itemWeight = itemWeight_light;
      d->isExplosive = true;
      d->glyph = '-';
      d->maxStackSizeAtSpawn = 2;
      d->landOnHardSurfaceSoundMessage = "";
    } break;

    default: {} break;
  }
}

void ItemData::addFeatureFoundIn(ItemDef* const itemDef,
                                 const Feature_t featureId,
                                 const int CHANCE_TO_INCLUDE) const {
  itemDef->featuresCanBeFoundIn.push_back(
    pair<Feature_t, int>(featureId, CHANCE_TO_INCLUDE));
}

void ItemData::setDmgFromMonsterDef(ItemDef& d, const ActorDef& def) const {
//  const int ACTOR_LVL = owningActor.monsterLvl;

//  //Set 1dY dmg from formula
//  const int DMG_ROLLS_CAP = 10;
//  const double DMG_ROLLS_BASE_DB = 1.0;
//  const double DMG_ROLLS_INCR_DB = 0.4;
//  const double DMG_STRENGTH_FACTOR = EntityStrength::getFactor(dmgStrength);
//  const double DMG_ROLLS_BEFORE_STR = DMG_ROLLS_BASE_DB + (double(ACTOR_LVL - 1) * DMG_ROLLS_INCR_DB);
//  const int DMG_ROLLS_AFTER_STR = int(ceil(DMG_ROLLS_BEFORE_STR * DMG_STRENGTH_FACTOR));
//  const int DMG_ROLLS_AFTER_CAP = max(1, min(DMG_ROLLS_CAP, DMG_ROLLS_AFTER_STR));
//
//  d.meleeDmg = pair<int, int>(1, DMG_ROLLS_AFTER_CAP);
//  d.rangedDmg = DiceParam(1, DMG_ROLLS_AFTER_CAP, 0);

  d.meleeDmg  = pair<int, int>(1, def.dmgMelee);
  d.rangedDmg = DiceParam(1, def.dmgRanged, 0);
}

//------------------------------- LIST OF ITEMS
void ItemData::makeList() {
  ItemDef* d = NULL;

  d = new ItemDef(item_trapezohedron);
  resetDef(d, itemDef_general);
  d->name = ItemName("Shining Trapezohedron", "Shining Trapezohedrons", "The Shining Trapezohedron");
  d->spawnStandardMinDLVL = -1;
  d->spawnStandardMaxDLVL = -1;
  d->isStackable = false;
  d->glyph = '*';
  d->color = clrRedLgt;
  d->tile = tile_trapezohedron;
  itemDefs[d->id] = d;

  d = new ItemDef(item_sawedOff);
  resetDef(d, itemDef_rangedWpn);
  d->name = ItemName("Sawed-off Shotgun", "Sawed-off shotguns", "a Sawed-off Shotgun");
  d->itemWeight = itemWeight_medium;
  d->tile = tile_shotgun;
  d->isShotgun = true;
  d->meleeAttackMessages = ItemAttackMessages("strike", "strikes me with a shotgun");
  d->rangedDmg = DiceParam(8, 3);
  d->rangedAmmoTypeUsed = item_shotgunShell;
  d->rangedAttackMessages = ItemAttackMessages("fire", "fires a shotgun");
  d->rangedSoundMessage = "I hear a shotgun blast.";
  d->rangedAbilityUsed = ability_accuracyRanged;
  d->meleeAbilityUsed = ability_accuracyMelee;
  d->reloadAudio = audio_shotgun_load_shell;
  addFeatureFoundIn(d, feature_chest);
  addFeatureFoundIn(d, feature_cabinet);
  addFeatureFoundIn(d, feature_cocoon);
  itemDefs[d->id] = d;

  d = new ItemDef(item_pumpShotgun);
  resetDef(d, itemDef_rangedWpn);
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
  d->rangedAudio = audio_shotgunPump_fire;
  d->reloadAudio = audio_shotgun_load_shell;
  addFeatureFoundIn(d, feature_chest);
  addFeatureFoundIn(d, feature_cabinet);
  addFeatureFoundIn(d, feature_cocoon);
  itemDefs[d->id] = d;

  d = new ItemDef(item_shotgunShell);
  resetDef(d, itemDef_ammo);
  d->name = ItemName("Shotgun shell", "Shotgun shells", "a shotgun shell");
  d->maxStackSizeAtSpawn = 10;
  addFeatureFoundIn(d, feature_chest);
  addFeatureFoundIn(d, feature_cabinet);
  addFeatureFoundIn(d, feature_cocoon);
  itemDefs[d->id] = d;

  d = new ItemDef(item_incinerator);
  resetDef(d, itemDef_rangedWpn);
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
  itemDefs[d->id] = d;

  d = new ItemDef(item_napalmCartridge);
  resetDef(d, itemDef_ammoClip);
  d->name = ItemName("Napalm Cartridge", "Napalm Cartridges", "a Napalm Cartridge");
  d->itemWeight = itemWeight_light;
  d->ammoContainedInClip = 3;
  d->spawnStandardMinDLVL = 8;
  d->maxStackSizeAtSpawn = 1;
  addFeatureFoundIn(d, feature_chest, 25);
  addFeatureFoundIn(d, feature_cabinet, 25);
  addFeatureFoundIn(d, feature_cocoon, 25);
  itemDefs[d->id] = d;

  d = new ItemDef(item_machineGun);
  resetDef(d, itemDef_rangedWpn);
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
  d->rangedAudio = audio_tommygun_fire;
  addFeatureFoundIn(d, feature_chest);
  addFeatureFoundIn(d, feature_cabinet);
  addFeatureFoundIn(d, feature_cocoon);
  itemDefs[d->id] = d;

  d = new ItemDef(item_drumOfBullets);
  resetDef(d, itemDef_ammoClip);
  d->name = ItemName("Drum of .45 ACP", "Drums of .45 ACP", "a Drum of .45 ACP");
  d->ammoContainedInClip = 50;
  addFeatureFoundIn(d, feature_chest);
  addFeatureFoundIn(d, feature_cabinet);
  addFeatureFoundIn(d, feature_cocoon);
  itemDefs[d->id] = d;

  d = new ItemDef(item_pistol);
  resetDef(d, itemDef_rangedWpn);
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
  d->rangedAudio = audio_pistol_fire;
  d->reloadAudio = audio_pistol_reload;
  addFeatureFoundIn(d, feature_chest);
  addFeatureFoundIn(d, feature_cabinet);
  addFeatureFoundIn(d, feature_cocoon);
  itemDefs[d->id] = d;

  d = new ItemDef(item_flareGun);
  resetDef(d, itemDef_rangedWpn);
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
  d->rangedStatusEffect = new StatusFlared(eng);
  addFeatureFoundIn(d, feature_chest);
  addFeatureFoundIn(d, feature_cabinet);
  addFeatureFoundIn(d, feature_cocoon);
  itemDefs[d->id] = d;

  d = new ItemDef(item_pistolClip);
  resetDef(d, itemDef_ammoClip);
  d->name = ItemName(".45ACP Colt cartridge", ".45ACP Colt cartridges", "a .45ACP Colt cartridge");
  d->ammoContainedInClip = 7;
  addFeatureFoundIn(d, feature_chest);
  addFeatureFoundIn(d, feature_cabinet);
  addFeatureFoundIn(d, feature_cocoon);
  itemDefs[d->id] = d;

  d = new ItemDef(item_teslaCanon);
  resetDef(d, itemDef_rangedWpn);
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
  itemDefs[d->id] = d;

  d = new ItemDef(item_teslaCanister);
  resetDef(d, itemDef_ammoClip);
  d->name = ItemName("Nuclear battery", "Nuclear batteries", "a Nuclear battery");
  d->ammoContainedInClip = 30;
  d->spawnStandardMinDLVL = 6;
  addFeatureFoundIn(d, feature_chest, 50);
  addFeatureFoundIn(d, feature_cabinet, 50);
  addFeatureFoundIn(d, feature_cocoon, 50);
  itemDefs[d->id] = d;

  d = new ItemDef(item_spikeGun);
  resetDef(d, itemDef_rangedWpn);
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
  itemDefs[d->id] = d;

  d = new ItemDef(item_dynamite);
  resetDef(d, itemDef_explosive);
  d->name = ItemName("Dynamite", "Sticks of Dynamite", "a Stick of Dynamite");
  d->itemWeight = itemWeight_light;
  d->tile = tile_dynamite;
  d->color = clrRedLgt;
  addFeatureFoundIn(d, feature_chest);
  addFeatureFoundIn(d, feature_cabinet);
  addFeatureFoundIn(d, feature_cocoon);
  itemDefs[d->id] = d;

  d = new ItemDef(item_flare);
  resetDef(d, itemDef_explosive);
  d->name = ItemName("Flare", "Flares", "a Flare");
  d->itemWeight = itemWeight_light;
  d->tile = tile_flare;
  d->color = clrGray;
  d->isAmmo = true;
  addFeatureFoundIn(d, feature_chest);
  addFeatureFoundIn(d, feature_cabinet);
  addFeatureFoundIn(d, feature_cocoon);
  itemDefs[d->id] = d;

  d = new ItemDef(item_molotov);
  resetDef(d, itemDef_explosive);
  d->name = ItemName("Molotov Cocktail", "Molotov Cocktails", "a Molotov Cocktail");
  d->itemWeight = itemWeight_light;
  d->tile = tile_molotov;
  d->color = clrWhite;
  addFeatureFoundIn(d, feature_chest);
  addFeatureFoundIn(d, feature_cabinet);
  addFeatureFoundIn(d, feature_cocoon);
  itemDefs[d->id] = d;

  d = new ItemDef(item_throwingKnife);
  resetDef(d, itemDef_missileWeapon);
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
  itemDefs[d->id] = d;

  d = new ItemDef(item_rock);
  resetDef(d, itemDef_missileWeapon);
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
  itemDefs[d->id] = d;

  d = new ItemDef(item_dagger);
  resetDef(d, itemDef_meleeWpn);
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
  itemDefs[d->id] = d;

  d = new ItemDef(item_hatchet);
  resetDef(d, itemDef_meleeWpn);
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
  itemDefs[d->id] = d;

  d = new ItemDef(item_club);
  resetDef(d, itemDef_meleeWpn);
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
  itemDefs[d->id] = d;

  d = new ItemDef(item_hammer);
  resetDef(d, itemDef_meleeWpn);
  d->name = ItemName("Hammer", "Hammers", "a Hammer");
  d->itemWeight = itemWeight_medium;
  d->tile = tile_hammer;
  d->meleeAttackMessages = ItemAttackMessages("strike", "strikes me with a Hammer");
  d->meleeDmg = pair<int, int>(2, 4);
  d->meleeHitChanceMod = 5;
  d->meleeAbilityUsed = ability_accuracyMelee;
  addFeatureFoundIn(d, feature_cabinet);
  addFeatureFoundIn(d, feature_cocoon);
  itemDefs[d->id] = d;

  d = new ItemDef(item_machete);
  resetDef(d, itemDef_meleeWpn);
  d->name = ItemName("Machete", "Machetes", "a Machete");
  d->itemWeight = itemWeight_medium;
  d->tile = tile_machete;
  d->meleeAttackMessages = ItemAttackMessages("strike", "strikes me with a Machete");
  d->meleeDmg = pair<int, int>(2, 5);
  d->meleeHitChanceMod = 0;
  d->meleeAbilityUsed = ability_accuracyMelee;
  addFeatureFoundIn(d, feature_cabinet);
  addFeatureFoundIn(d, feature_cocoon);
  itemDefs[d->id] = d;

  d = new ItemDef(item_axe);
  resetDef(d, itemDef_meleeWpn);
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
  itemDefs[d->id] = d;

  d = new ItemDef(item_pitchFork);
  resetDef(d, itemDef_meleeWpn);
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
  itemDefs[d->id] = d;

  d = new ItemDef(item_sledgeHammer);
  resetDef(d, itemDef_meleeWpn);
  d->name = ItemName("Sledgehammer", "Sledgehammers", "a Sledgehammer");
  d->itemWeight = itemWeight_heavy;
  d->tile = tile_sledgeHammer;
  d->meleeAttackMessages = ItemAttackMessages("strike", "strikes me with a Sledgehammer");
  d->meleeDmg = pair<int, int>(3, 5);
  d->meleeHitChanceMod = -10;
  d->meleeAbilityUsed = ability_accuracyMelee;
  d->meleeCausesKnockBack = true;
  addFeatureFoundIn(d, feature_cabinet);
  itemDefs[d->id] = d;

  d = new ItemDef(item_ironSpike);
  resetDef(d, itemDef_missileWeapon);
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
  itemDefs[d->id] = d;

//  d = new ItemDef(item_crowbar);
//  resetDef(d, itemDef_general);
//  d->name = ItemName("Crowbar", "Crowbars", "a Crowbar");
//  d->chanceToIncludeInSpawnList = 70;
//  d->itemWeight = itemWeight_light;
//  d->tile = tile_crowbar;
//  d->isStackable = false;
//  d->color = clrGray;
//  d->glyph = '~';
//  d->landOnHardSurfaceSoundMessage = "I hear a clanking sound.";
//  addFeatureFoundIn(d, feature_cabinet);
//  itemDefs[d->id] = d;
//
//  d = new ItemDef(item_lockpick);
//  resetDef(d, itemDef_general);
//  d->name = ItemName("Lockpick", "Lockpicks", "a Lockpick");
//  d->itemWeight = itemWeight_extraLight;
//  d->tile = tile_lockpick;
//  d->isStackable = true;
//  d->maxStackSizeAtSpawn = 1;
//  d->color = clrWhite;
//  d->glyph = '~';
//  d->landOnHardSurfaceSoundMessage = "I clinking sound.";
//  addFeatureFoundIn(d, feature_cabinet);
//  itemDefs[d->id] = d;

  d = new ItemDef(item_playerKick);
  resetDef(d, itemDef_meleeWpnIntr);
  d->meleeAttackMessages = ItemAttackMessages("kick", "");
  d->meleeAbilityUsed = ability_accuracyMelee;
  d->meleeHitChanceMod = 20;
  d->meleeDmg = pair<int, int>(1, 3);
  d->meleeCausesKnockBack = true;
  itemDefs[d->id] = d;

  d = new ItemDef(item_playerStomp);
  resetDef(d, itemDef_meleeWpnIntr);
  d->meleeAttackMessages = ItemAttackMessages("stomp", "");
  d->meleeAbilityUsed = ability_accuracyMelee;
  d->meleeHitChanceMod = 20;
  d->meleeDmg = pair<int, int>(1, 3);
  d->meleeCausesKnockBack = false;
  itemDefs[d->id] = d;

  d = new ItemDef(item_playerPunch);
  resetDef(d, itemDef_meleeWpnIntr);
  d->meleeAttackMessages = ItemAttackMessages("punch", "");
  d->meleeAbilityUsed = ability_accuracyMelee;
  d->meleeHitChanceMod = 25;
  d->meleeDmg = pair<int, int>(1, 2);
  itemDefs[d->id] = d;

  d = new ItemDef(item_zombieClaw);
  resetDef(d, itemDef_meleeWpnIntr);
  d->meleeAttackMessages = ItemAttackMessages("", "claws me");
  setDmgFromMonsterDef(*d, eng->actorData->actorDefs[actor_zombie]);
  itemDefs[d->id] = d;

  d = new ItemDef(item_zombieClawDiseased);
  resetDef(d, itemDef_meleeWpnIntr);
  d->meleeAttackMessages = ItemAttackMessages("", "claws me");
  setDmgFromMonsterDef(*d, eng->actorData->actorDefs[actor_zombie]);
  d->meleeStatusEffect = new StatusDiseased(eng);
  itemDefs[d->id] = d;

  d = new ItemDef(item_zombieAxe);
  resetDef(d, itemDef_meleeWpnIntr);
  d->meleeAttackMessages = ItemAttackMessages("", "chops me with a rusty axe");
  setDmgFromMonsterDef(*d, eng->actorData->actorDefs[actor_zombieAxe]);
  itemDefs[d->id] = d;

  d = new ItemDef(item_bloatedZombiePunch);
  resetDef(d, itemDef_meleeWpnIntr);
  d->meleeAttackMessages = ItemAttackMessages("", "mauls me");
  setDmgFromMonsterDef(*d, eng->actorData->actorDefs[actor_bloatedZombie]);
  d->meleeCausesKnockBack = true;
  itemDefs[d->id] = d;

  d = new ItemDef(item_bloatedZombieSpit);
  resetDef(d, itemDef_rangedWpnIntr);
  d->rangedAttackMessages = ItemAttackMessages("", "spits pus at me");
  setDmgFromMonsterDef(*d, eng->actorData->actorDefs[actor_zombieAxe]);
  d->rangedSoundMessage = "I hear spitting.";
  d->rangedMissileColor = clrGreenLgt;
  d->rangedDmgType = dmgType_acid;
  d->rangedMissileGlyph = '*';
  itemDefs[d->id] = d;

  d = new ItemDef(item_ratBite);
  resetDef(d, itemDef_meleeWpnIntr);
  d->meleeAttackMessages = ItemAttackMessages("", "bites me");
  setDmgFromMonsterDef(*d, eng->actorData->actorDefs[actor_rat]);
  itemDefs[d->id] = d;

  d = new ItemDef(item_ratBiteDiseased);
  resetDef(d, itemDef_meleeWpnIntr);
  d->meleeAttackMessages = ItemAttackMessages("", "bites me");
  setDmgFromMonsterDef(*d, eng->actorData->actorDefs[actor_rat]);
  d->meleeStatusEffect = new StatusDiseased(eng);
  itemDefs[d->id] = d;

  d = new ItemDef(item_ratThingBite);
  resetDef(d, itemDef_meleeWpnIntr);
  d->meleeAttackMessages = ItemAttackMessages("", "bites me");
  setDmgFromMonsterDef(*d, eng->actorData->actorDefs[actor_ratThing]);
  itemDefs[d->id] = d;

  d = new ItemDef(item_wormMassBite);
  resetDef(d, itemDef_meleeWpnIntr);
  d->meleeAttackMessages = ItemAttackMessages("", "bites me");
  setDmgFromMonsterDef(*d, eng->actorData->actorDefs[actor_wormMass]);
  itemDefs[d->id] = d;

  d = new ItemDef(item_wolfBite);
  resetDef(d, itemDef_meleeWpnIntr);
  d->meleeAttackMessages = ItemAttackMessages("", "bites me");
  setDmgFromMonsterDef(*d, eng->actorData->actorDefs[actor_wolf]);
  itemDefs[d->id] = d;

  d = new ItemDef(item_greenSpiderBite);
  resetDef(d, itemDef_meleeWpnIntr);
  d->meleeAttackMessages = ItemAttackMessages("", "bites me");
  setDmgFromMonsterDef(*d, eng->actorData->actorDefs[actor_greenSpider]);
  d->meleeStatusEffect = new StatusBlind(4);
  itemDefs[d->id] = d;

  d = new ItemDef(item_whiteSpiderBite);
  resetDef(d, itemDef_meleeWpnIntr);
  d->meleeAttackMessages = ItemAttackMessages("", "bites me");
  setDmgFromMonsterDef(*d, eng->actorData->actorDefs[actor_whiteSpider]);
  d->meleeStatusEffect = new StatusParalyzed(3);
  itemDefs[d->id] = d;

  d = new ItemDef(item_redSpiderBite);
  resetDef(d, itemDef_meleeWpnIntr);
  d->meleeAttackMessages = ItemAttackMessages("", "bites me");
  setDmgFromMonsterDef(*d, eng->actorData->actorDefs[actor_redSpider]);
  d->meleeStatusEffect = new StatusWeak(eng);
  itemDefs[d->id] = d;

  d = new ItemDef(item_shadowSpiderBite);
  resetDef(d, itemDef_meleeWpnIntr);
  d->meleeAttackMessages = ItemAttackMessages("", "bites me");
  setDmgFromMonsterDef(*d, eng->actorData->actorDefs[actor_shadowSpider]);
  itemDefs[d->id] = d;

  d = new ItemDef(item_lengSpiderBite);
  resetDef(d, itemDef_meleeWpnIntr);
  d->meleeAttackMessages = ItemAttackMessages("", "bites me");
  setDmgFromMonsterDef(*d, eng->actorData->actorDefs[actor_lengSpider]);
  itemDefs[d->id] = d;

  d = new ItemDef(item_hellHoundFireBreath);
  resetDef(d, itemDef_rangedWpnIntr);
  d->rangedAttackMessages = ItemAttackMessages("", "breaths fire at me");
  d->rangedSoundMessage = "I hear a burst of flames.";
  setDmgFromMonsterDef(*d, eng->actorData->actorDefs[actor_fireHound]);
  d->rangedStatusEffect = new StatusBurning(eng);
  d->rangedMissileColor = clrRedLgt;
  d->rangedMissileGlyph = '*';
  d->rangedMissileLeavesTrail = true;
  d->rangedMissileLeavesSmoke = true;
  d->rangedDmgType = dmgType_fire;
  itemDefs[d->id] = d;

  d = new ItemDef(item_hellHoundBite);
  resetDef(d, itemDef_meleeWpnIntr);
  d->meleeAttackMessages = ItemAttackMessages("", "bites me");
  setDmgFromMonsterDef(*d, eng->actorData->actorDefs[actor_fireHound]);
  itemDefs[d->id] = d;

  d = new ItemDef(item_dustVortexEngulf);
  resetDef(d, itemDef_meleeWpnIntr);
  d->meleeAttackMessages = ItemAttackMessages("", "engulfs me");
  setDmgFromMonsterDef(*d, eng->actorData->actorDefs[actor_dustVortex]);
  d->meleeStatusEffect = new StatusBlind(eng);
  itemDefs[d->id] = d;

  d = new ItemDef(item_fireVortexEngulf);
  resetDef(d, itemDef_meleeWpnIntr);
  d->meleeAttackMessages = ItemAttackMessages("", "engulfs me");
  setDmgFromMonsterDef(*d, eng->actorData->actorDefs[actor_fireVortex]);
  d->meleeStatusEffect = new StatusBurning(eng);
  itemDefs[d->id] = d;

  d = new ItemDef(item_ghostClaw);
  resetDef(d, itemDef_meleeWpnIntr);
  d->meleeAttackMessages = ItemAttackMessages("", "claws me");
  setDmgFromMonsterDef(*d, eng->actorData->actorDefs[actor_ghost]);
  d->meleeStatusEffect = new StatusTerrified(4);
  itemDefs[d->id] = d;

  d = new ItemDef(item_phantasmSickle);
  resetDef(d, itemDef_meleeWpnIntr);
  d->meleeAttackMessages = ItemAttackMessages("", "slices me with a sickle");
  setDmgFromMonsterDef(*d, eng->actorData->actorDefs[actor_phantasm]);
  d->meleeStatusEffect = new StatusTerrified(4);
  itemDefs[d->id] = d;

  d = new ItemDef(item_wraithClaw);
  resetDef(d, itemDef_meleeWpnIntr);
  d->meleeAttackMessages = ItemAttackMessages("", "claws me");
  setDmgFromMonsterDef(*d, eng->actorData->actorDefs[actor_wraith]);
  d->meleeStatusEffect = new StatusTerrified(4);
  itemDefs[d->id] = d;

  d = new ItemDef(item_giantBatBite);
  resetDef(d, itemDef_meleeWpnIntr);
  d->meleeAttackMessages = ItemAttackMessages("", "bites me");
  setDmgFromMonsterDef(*d, eng->actorData->actorDefs[actor_giantBat]);
  itemDefs[d->id] = d;

  d = new ItemDef(item_miGoElectricGun);
  resetDef(d, itemDef_rangedWpnIntr);
  d->rangedMissileLeavesTrail = true;
  d->rangedMissileColor = clrYellow;
  d->rangedMissileGlyph = '/';
  d->rangedAttackMessages = ItemAttackMessages("", "fires an electric gun");
  d->rangedDmgType = dmgType_electric;
  d->rangedStatusEffect = new StatusParalyzed(2);
  d->rangedSoundMessage = "I hear a bolt of electricity.";
  setDmgFromMonsterDef(*d, eng->actorData->actorDefs[actor_miGo]);
  d->rangedSoundIsLoud = true;
  itemDefs[d->id] = d;

  d = new ItemDef(item_ghoulClaw);
  resetDef(d, itemDef_meleeWpnIntr);
  d->meleeAttackMessages = ItemAttackMessages("", "claws me");
  setDmgFromMonsterDef(*d, eng->actorData->actorDefs[actor_ghoul]);
  d->meleeStatusEffect = new StatusDiseased(eng);
  itemDefs[d->id] = d;

  d = new ItemDef(item_shadowClaw);
  resetDef(d, itemDef_meleeWpnIntr);
  d->meleeAttackMessages = ItemAttackMessages("", "claws me");
  setDmgFromMonsterDef(*d, eng->actorData->actorDefs[actor_shadow]);
  itemDefs[d->id] = d;

  d = new ItemDef(item_byakheeClaw);
  resetDef(d, itemDef_meleeWpnIntr);
  d->meleeAttackMessages = ItemAttackMessages("", "claws me");
  setDmgFromMonsterDef(*d, eng->actorData->actorDefs[actor_byakhee]);
  itemDefs[d->id] = d;

  d = new ItemDef(item_giantMantisClaw);
  resetDef(d, itemDef_meleeWpnIntr);
  d->meleeAttackMessages = ItemAttackMessages("", "claws me");
  setDmgFromMonsterDef(*d, eng->actorData->actorDefs[actor_giantMantis]);
  itemDefs[d->id] = d;

  d = new ItemDef(item_giantLocustBite);
  resetDef(d, itemDef_meleeWpnIntr);
  d->meleeAttackMessages = ItemAttackMessages("", "bites me");
  setDmgFromMonsterDef(*d, eng->actorData->actorDefs[actor_giantLocust]);
  itemDefs[d->id] = d;

  d = new ItemDef(item_mummyMaul);
  resetDef(d, itemDef_meleeWpnIntr);
  d->meleeAttackMessages = ItemAttackMessages("", "mauls me");
  setDmgFromMonsterDef(*d, eng->actorData->actorDefs[actor_mummy]);
  d->meleeStatusEffect = new StatusCursed(eng);
  d->meleeCausesKnockBack = true;
  itemDefs[d->id] = d;

  d = new ItemDef(item_deepOneJavelinAttack);
  resetDef(d, itemDef_rangedWpnIntr);
  d->rangedAttackMessages = ItemAttackMessages("", "throws a Javelin at me");
  setDmgFromMonsterDef(*d, eng->actorData->actorDefs[actor_deepOne]);
  d->rangedSoundMessage = "";
  d->rangedMissileColor = clrBrown;
  d->rangedMissileGlyph = '/';
  d->rangedSoundIsLoud = false;
  itemDefs[d->id] = d;

  d = new ItemDef(item_deepOneSpearAttack);
  resetDef(d, itemDef_meleeWpnIntr);
  d->meleeAttackMessages = ItemAttackMessages("", "thrusts a spear at me");
  setDmgFromMonsterDef(*d, eng->actorData->actorDefs[actor_deepOne]);
  itemDefs[d->id] = d;

  d = new ItemDef(item_oozeBlackSpewPus);
  resetDef(d, itemDef_meleeWpnIntr);
  d->meleeAttackMessages = ItemAttackMessages("", "spews pus on me");
  setDmgFromMonsterDef(*d, eng->actorData->actorDefs[actor_oozeBlack]);
  itemDefs[d->id] = d;

  d = new ItemDef(item_oozeClearSpewPus);
  resetDef(d, itemDef_meleeWpnIntr);
  d->meleeAttackMessages = ItemAttackMessages("", "spews pus on me");
  setDmgFromMonsterDef(*d, eng->actorData->actorDefs[actor_oozeClear]);
  itemDefs[d->id] = d;

  d = new ItemDef(item_oozePutridSpewPus);
  resetDef(d, itemDef_meleeWpnIntr);
  d->meleeAttackMessages = ItemAttackMessages("", "spews infected pus on me");
  setDmgFromMonsterDef(*d, eng->actorData->actorDefs[actor_oozePutrid]);
  d->meleeStatusEffect = new StatusDiseased(eng);
  itemDefs[d->id] = d;

  d = new ItemDef(item_oozePoisonSpewPus);
  resetDef(d, itemDef_meleeWpnIntr);
  d->meleeAttackMessages = ItemAttackMessages("", "spews poisonous pus on me");
  setDmgFromMonsterDef(*d, eng->actorData->actorDefs[actor_oozePoison]);
  d->meleeStatusEffect = new StatusPoisoned(eng);
  itemDefs[d->id] = d;

  d = new ItemDef(item_colourOutOfSpaceTouch);
  resetDef(d, itemDef_meleeWpnIntr);
  d->meleeAttackMessages = ItemAttackMessages("", "touches me");
  setDmgFromMonsterDef(*d, eng->actorData->actorDefs[actor_colourOutOfSpace]);
  itemDefs[d->id] = d;

  d = new ItemDef(item_huntingHorrorBite);
  resetDef(d, itemDef_meleeWpnIntr);
  d->meleeAttackMessages = ItemAttackMessages("", "bites me");
  setDmgFromMonsterDef(*d, eng->actorData->actorDefs[actor_huntingHorror]);
  d->meleeStatusEffect = new StatusParalyzed(eng);
  itemDefs[d->id] = d;

  d = new ItemDef(item_armorLeatherJacket);
  resetDef(d, itemDef_armor);
  d->name = ItemName("Leather Jacket", "", "a Leather Jacket");
  d->itemWeight = itemWeight_light;
  d->color = clrBrown;
  d->spawnStandardMinDLVL = 1;
  d->armorData.absorptionPoints[dmgType_acid] = 1;
  d->armorData.damageToDurabilityFactors[dmgType_acid] = 2.0;
  d->armorData.absorptionPoints[dmgType_electric] = 2;
  d->armorData.damageToDurabilityFactors[dmgType_electric] = 0.0;
  d->armorData.absorptionPoints[dmgType_fire] = 1;
  d->armorData.damageToDurabilityFactors[dmgType_fire] = 2.0;
  d->armorData.absorptionPoints[dmgType_physical] = 1;
  d->armorData.damageToDurabilityFactors[dmgType_physical] = 1.0;
  d->armorData.chanceToDeflectTouchAttacks = 20;
  d->landOnHardSurfaceSoundMessage = "";
  addFeatureFoundIn(d, feature_cabinet);
  itemDefs[d->id] = d;

  d = new ItemDef(item_armorIronSuit);
  resetDef(d, itemDef_armor);
  d->name = ItemName("Iron Suit", "", "an Iron Suit");
  d->itemWeight = itemWeight_heavy;
  d->color = clrWhite;
  d->spawnStandardMinDLVL = 2;
  d->armorData.absorptionPoints[dmgType_acid] = 1;
  d->armorData.damageToDurabilityFactors[dmgType_acid] = 2.0;
  d->armorData.absorptionPoints[dmgType_electric] = 0;
  d->armorData.damageToDurabilityFactors[dmgType_electric] = 0.0;
  d->armorData.absorptionPoints[dmgType_fire] = 1;
  d->armorData.damageToDurabilityFactors[dmgType_fire] = 2.0;
  d->armorData.absorptionPoints[dmgType_physical] = 4;
  d->armorData.damageToDurabilityFactors[dmgType_physical] = 0.5;
  d->armorData.chanceToDeflectTouchAttacks = 80;
  d->landOnHardSurfaceSoundMessage = "I hear a crashing sound.";
  addFeatureFoundIn(d, feature_cabinet);
  itemDefs[d->id] = d;

  d = new ItemDef(item_armorFlackJacket);
  resetDef(d, itemDef_armor);
  d->name = ItemName("Flak Jacket", "", "a Flak Jacket");
  d->itemWeight = itemWeight_heavy;
  d->color = clrGreen;
  d->spawnStandardMinDLVL = 3;
  d->armorData.absorptionPoints[dmgType_acid] = 1;
  d->armorData.damageToDurabilityFactors[dmgType_acid] = 2.0;
  d->armorData.absorptionPoints[dmgType_electric] = 0;
  d->armorData.damageToDurabilityFactors[dmgType_electric] = 0.0;
  d->armorData.absorptionPoints[dmgType_fire] = 1;
  d->armorData.damageToDurabilityFactors[dmgType_fire] = 2.0;
  d->armorData.absorptionPoints[dmgType_physical] = 3;
  d->armorData.damageToDurabilityFactors[dmgType_physical] = 0.5;
  d->armorData.chanceToDeflectTouchAttacks = 20;
  d->landOnHardSurfaceSoundMessage = "I hear a thudding sound.";
  addFeatureFoundIn(d, feature_cabinet);
  itemDefs[d->id] = d;

//  d = new ItemDef(item_armorAsbestosSuit);
//  resetDef(d, itemDef_armor);
//  d->name = ItemName("Asbestos Suit", "", "an Asbestos Suit");
//  d->color = clrRedLgt;
//  d->spawnStandardMinDLVL = 3;
//  d->armorData.absorptionPoints[dmgType_acid] = 999;
//  d->armorData.damageToDurabilityFactors[dmgType_acid] = 0.1;
//  d->armorData.absorptionPoints[dmgType_electricity] = 999;
//  d->armorData.damageToDurabilityFactors[dmgType_electricity] = 0.0;
//  d->armorData.absorptionPoints[dmgType_fire] = 999;
//  d->armorData.damageToDurabilityFactors[dmgType_fire] = 0.1;
//  d->armorData.absorptionPoints[dmgType_physical] = 0;
//  d->armorData.damageToDurabilityFactors[dmgType_physical] = 2.0;
//  d->armorData.chanceToDeflectTouchAttacks = 95;
//  d->armorData.protectsAgainstStatusBurning = true;
//  d->armorData.overRideAbsorptionPointLabel = "?";
//  itemDefs[d->id] = d;

  d = new ItemDef(item_scrollOfMayhem);
  resetDef(d, itemDef_scroll);
  d->spellCastFromScroll = spell_mayhem;
  itemDefs[d->id] = d;

  d = new ItemDef(item_scrollOfTeleportation);
  resetDef(d, itemDef_scroll);
  d->spellCastFromScroll = spell_teleport;
  itemDefs[d->id] = d;

  d = new ItemDef(item_scrollOfDescent);
  resetDef(d, itemDef_scroll);
  d->spellCastFromScroll = spell_descent;
  d->spawnStandardMinDLVL = 6;
  itemDefs[d->id] = d;

  d = new ItemDef(item_scrollOfPestilence);
  resetDef(d, itemDef_scroll);
  d->spellCastFromScroll = spell_pestilence;
  itemDefs[d->id] = d;

  d = new ItemDef(item_scrollOfEnfeebleEnemies);
  resetDef(d, itemDef_scroll);
  d->spellCastFromScroll = spell_enfeeble;
  itemDefs[d->id] = d;

  d = new ItemDef(item_scrollOfDetectItems);
  resetDef(d, itemDef_scroll);
  d->spellCastFromScroll = spell_detectItems;
  itemDefs[d->id] = d;

  d = new ItemDef(item_scrollOfDetectTraps);
  resetDef(d, itemDef_scroll);
  d->spellCastFromScroll = spell_detectTraps;
  itemDefs[d->id] = d;

  d = new ItemDef(item_scrollOfIdentify);
  resetDef(d, itemDef_scroll);
  d->spellCastFromScroll = spell_identify;
  itemDefs[d->id] = d;

  d = new ItemDef(item_scrollOfBlessing);
  resetDef(d, itemDef_scroll);
  d->spellCastFromScroll = spell_bless;
  itemDefs[d->id] = d;

  d = new ItemDef(item_scrollOfClairvoyance);
  resetDef(d, itemDef_scroll);
  d->spellCastFromScroll = spell_clairvoyance;
  itemDefs[d->id] = d;

  d = new ItemDef(item_scrollOfAzathothsBlast);
  resetDef(d, itemDef_scroll);
  d->spellCastFromScroll = spell_azathothsBlast;
  itemDefs[d->id] = d;

  d = new ItemDef(item_scrollOfOpening);
  resetDef(d, itemDef_scroll);
  d->spellCastFromScroll = spell_opening;
  itemDefs[d->id] = d;

  d = new ItemDef(item_thaumaturgicAlteration);
  resetDef(d, itemDef_scroll);
  d->spellCastFromScroll = spell_mthPower;
  d->isIntrinsic = true;
  itemDefs[d->id] = d;

//  d = new ItemDef(item_scrollOfVoidChain);
//  resetDef(d, itemDef_scroll);
//  itemDefs[d->id] = d;

//  d = new ItemDef(item_scrollOfIbnGhazisPowder);
//  resetDef(d, itemDef_scroll);
//  itemDefs[d->id] = d;

  d = new ItemDef(item_potionOfHealing);
  resetDef(d, itemDef_potion);
  itemDefs[d->id] = d;

  d = new ItemDef(item_potionOfSpirit);
  resetDef(d, itemDef_potion);
  itemDefs[d->id] = d;

  d = new ItemDef(item_potionOfBlindness);
  resetDef(d, itemDef_potion);
  itemDefs[d->id] = d;

//  d = new ItemDef(item_potionOfCorruption);
//  resetDef(d, itemDef_potion);
//  itemDefs[d->id] = d;

  d = new ItemDef(item_potionOfTheCobra);
  resetDef(d, itemDef_potion);
  itemDefs[d->id] = d;

//  d = new ItemDef(item_potionOfStealth);
//  resetDef(d, itemDef_potion);
//  itemDefs[d->id] = d;

  d = new ItemDef(item_potionOfFortitude);
  resetDef(d, itemDef_potion);
  itemDefs[d->id] = d;

  d = new ItemDef(item_potionOfToughness);
  resetDef(d, itemDef_potion);
  itemDefs[d->id] = d;

  d = new ItemDef(item_potionOfParalyzation);
  resetDef(d, itemDef_potion);
  itemDefs[d->id] = d;

  d = new ItemDef(item_potionOfConfusion);
  resetDef(d, itemDef_potion);
  itemDefs[d->id] = d;

//  d = new ItemDef(item_potionOfSorcery);
//  resetDef(d, itemDef_potion);
//  itemDefs[d->id] = d;

  d = new ItemDef(item_potionOfPoison);
  resetDef(d, itemDef_potion);
  itemDefs[d->id] = d;

  d = new ItemDef(item_potionOfKnowledge);
  resetDef(d, itemDef_potion);
  itemDefs[d->id] = d;

  d = new ItemDef(item_deviceSentry);
  resetDef(d, itemDef_device);
  d->name = ItemName("Sentry Device", "Sentry Devices", "a Sentry Device");
  d->itemValue = itemValue_majorTreasure;
  d->isIdentified = false;
  d->color = clrGray;
  addFeatureFoundIn(d, feature_chest, 10);
  addFeatureFoundIn(d, feature_tomb, 10);
  addFeatureFoundIn(d, feature_cocoon, 10);
  itemDefs[d->id] = d;

  d = new ItemDef(item_deviceRepeller);
  resetDef(d, itemDef_device);
  d->name = ItemName("Repeller Device", "Repeller Devices", "a Repeller Device");
  d->itemValue = itemValue_majorTreasure;
  d->isIdentified = false;
  d->color = clrGray;
  addFeatureFoundIn(d, feature_chest, 10);
  addFeatureFoundIn(d, feature_tomb, 10);
  addFeatureFoundIn(d, feature_cocoon, 10);
  itemDefs[d->id] = d;

  d = new ItemDef(item_deviceRejuvenator);
  resetDef(d, itemDef_device);
  d->name = ItemName("Rejuvenator Device", "Rejuvenator Devices", "a Rejuvenator Device");
  d->itemValue = itemValue_majorTreasure;
  d->isIdentified = false;
  d->color = clrGray;
  addFeatureFoundIn(d, feature_chest, 10);
  addFeatureFoundIn(d, feature_tomb, 10);
  addFeatureFoundIn(d, feature_cocoon, 10);
  itemDefs[d->id] = d;

  d = new ItemDef(item_deviceTranslocator);
  resetDef(d, itemDef_device);
  d->name = ItemName("Translocator Device", "Translocator Devices", "a Translocator Device");
  d->itemValue = itemValue_majorTreasure;
  d->isIdentified = false;
  d->color = clrGray;
  addFeatureFoundIn(d, feature_chest, 10);
  addFeatureFoundIn(d, feature_tomb, 10);
  addFeatureFoundIn(d, feature_cocoon, 10);
  itemDefs[d->id] = d;

//  d = new ItemDef(item_deviceSpellReflector);
//  resetDef(d, itemDef_device);
//  d->name = ItemName("Spell Reflector Device", "Spell Reflector Devices", "a Spell Reflector Device");
//  d->color = clrGray;
//  d->featuresCanBeFoundIn.push_back(feature_chest);
//  d->featuresCanBeFoundIn.push_back(feature_tomb);
//  itemDefs[d->id] = d;

  d = new ItemDef(item_deviceElectricLantern);
  resetDef(d, itemDef_device);
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
  itemDefs[d->id] = d;

  d = new ItemDef(item_medicalBag);
  resetDef(d, itemDef_general);
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
  itemDefs[d->id] = d;
}

void ItemData::addSaveLines(vector<string>& lines) const {
  for(unsigned int i = 1; i < endOfItemIds; i++) {
    lines.push_back(itemDefs[i]->isIdentified ? "1" : "0");

    if(itemDefs[i]->isScroll) {
      lines.push_back(itemDefs[i]->isTried ? "1" : "0");
    }
  }
}

void ItemData::setParametersFromSaveLines(vector<string>& lines) {
  for(unsigned int i = 1; i < endOfItemIds; i++) {
    itemDefs[i]->isIdentified = lines.front() == "0" ? false : true;
    lines.erase(lines.begin());

    if(itemDefs[i]->isScroll) {
      itemDefs[i]->isTried = lines.front() == "0" ? false : true;
      lines.erase(lines.begin());
    }
  }
}

bool ItemData::isWeaponStronger(const ItemDef& oldDef, const ItemDef& newDef, bool melee) {
  (void)oldDef;
  (void)newDef;
  (void)melee;
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

string ItemData::getItemRef(const Item& item, const ItemRef_t itemRefForm,
                            const bool SKIP_EXTRA_INFO) const {
  const ItemDef& d = item.getDef();
  string ret = "";

  if(d.isDevice && d.isIdentified == false) {
    return itemRefForm == itemRef_plain ?
           "Strange Device" : "a Strange Device";
  }

  if(d.isStackable && item.numberOfItems > 1 && itemRefForm == itemRef_plural) {
    ret = intToString(item.numberOfItems) + " ";
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

    if(d.isArmor) {
      const Armor* armor = dynamic_cast<const Armor*>(&item);
      const string armorDataLine = armor->getArmorDataLine(true);
      return armorDataLine == "" ? ret : ret + " " + armorDataLine;
    }

    if((d.isScroll || d.isQuaffable) && d.isTried && d.isIdentified == false) {
      return ret + " {tried}";
    }
  }

  return ret;
}

string ItemData::getItemInterfaceRef(
  const Item& item, const bool ADD_A,
  const PrimaryAttackMode_t attackMode) const {
  const ItemDef& d = item.getDef();

  if(d.isDevice && d.isIdentified == false) {
    return ADD_A ? "a Strange Device" : "Strange Device";
  }

  string ret = "";

  if(d.isStackable && item.numberOfItems > 1) {
    ret = intToString(item.numberOfItems) + " " + d.name.name_plural;
  } else {
    ret = (ADD_A ? d.name.name_a : d.name.name);
  }

  const int PLAYER_RANGED_SKILL =
    eng->player->getDef()->abilityVals.getVal(
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
      eng->player->getDef()->abilityVals.getVal(
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

