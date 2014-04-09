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
#include "Sound.h"
#include "ItemDevice.h"

using namespace std;

//------------------------------- ITEM ARCHETYPES (DEFAULTS)
void ItemDataHandler::resetData(ItemData* const d,
                                ItemDataArchetypes const archetype) const {
  switch(archetype) {
    case itemData_general: {
      d->itemValue = ItemValue::normal;
      d->itemWeight = itemWeight_none;
      d->spawnStandardMinDLVL = 1;
      d->spawnStandardMaxDLVL = 999999;
      d->maxStackSizeAtSpawn = 1;
      d->chanceToIncludeInSpawnList = 100;
      d->isStackable = true;
      d->isIdentified = true;
      d->isTried = false;
      d->glyph = 'X';
      d->clr = clrWhite;
      d->tile = tile_empty;
      d->primaryAttackMode = PrimaryAttackMode::none;
      d->isScroll = d->isPotion = d->isEatable = false;
      d->isArmor = d->isCloak = d->isRing = d->isAmulet = false;
      d->isIntrinsic = d->isMeleeWeapon = d->isRangedWeapon = false;
      d->isMissileWeapon = d->isShotgun = d->isMachineGun = false;
      d->isAmmo = d->isAmmoClip = d->isDevice = d->isMedicalBag = false;
      d->spellCastFromScroll = SpellId::endOfSpellId;
      d->ammoContainedInClip = 0;
      d->meleeHitChanceMod = 0;
      d->propAppliedOnMelee = NULL;
      d->meleeCausesKnockBack = false;
      d->rangedCausesKnockBack = false;
      d->meleeDmgType = DmgType::physical;
      d->rangedHitChanceMod = 0;
      d->rangedDmgLabelOverRide = "";
      d->rangedAmmoTypeUsed = ItemId::empty;
      d->rangedDmgType = DmgType::physical;
      d->rangedHasInfiniteAmmo = false;
      d->rangedMissileGlyph = 'X';
      d->rangedMissileTile = tile_projectileStandardFrontSlash;
      d->rangedMissileClr = clrWhite;
      d->rangedMissileLeavesTrail = false;
      d->rangedMissileLeavesSmoke = false;
      d->rangedSndMsg = "";
      d->rangedSndVol = SndVol::low;
      d->rangedMakesRicochetSound = false;
      d->landOnHardSurfaceSoundMsg = "I hear a thudding sound.";
      d->landOnHardSurfaceSfx = SfxId::endOfSfxId;
      d->rangedAttackSfx = SfxId::endOfSfxId;
      d->meleeHitSmallSfx = SfxId::endOfSfxId;
      d->meleeHitMediumSfx = SfxId::endOfSfxId;
      d->meleeHitHardSfx = SfxId::endOfSfxId;
      d->meleeMissSfx = SfxId::endOfSfxId;
      d->reloadSfx = SfxId::endOfSfxId;
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
      d->clr = clrWhite;
      d->primaryAttackMode = PrimaryAttackMode::melee;
      d->isMeleeWeapon = true;
      d->meleeHitSmallSfx = SfxId::hitSmall;
      d->meleeHitMediumSfx = SfxId::hitMedium;
      d->meleeHitHardSfx = SfxId::hitHard;
    } break;

    case itemData_meleeWpnIntr: {
      resetData(d, itemData_meleeWpn);
      d->isIntrinsic = true;
      d->spawnStandardMinDLVL = -1;
      d->spawnStandardMaxDLVL = -1;
      d->meleeHitSmallSfx = SfxId::hitSmall;
      d->meleeHitMediumSfx = SfxId::hitMedium;
      d->meleeHitHardSfx = SfxId::hitHard;
      d->meleeMissSfx = SfxId::missMedium;
    } break;

    case itemData_rangedWpn: {
      resetData(d, itemData_general);
      d->isStackable = false;
      d->itemWeight = itemWeight_medium;
      d->glyph = '}';
      d->clr = clrWhite;
      d->isMeleeWeapon = true;
      d->meleeDmg = pair<int, int>(1, 6);
      d->primaryAttackMode = PrimaryAttackMode::ranged;
      d->isRangedWeapon = true;
      d->rangedMissileGlyph = '/';
      d->rangedMissileClr = clrWhite;
      d->spawnStandardMaxDLVL = FIRST_CAVERN_LEVEL - 1;
      d->meleeHitSmallSfx = SfxId::hitSmall;
      d->meleeHitMediumSfx = SfxId::hitMedium;
      d->meleeHitHardSfx = SfxId::hitHard;
      d->meleeMissSfx = SfxId::missMedium;
      d->rangedSndVol = SndVol::high;
    } break;

    case itemData_rangedWpnIntr: {
      resetData(d, itemData_rangedWpn);
      d->isIntrinsic = true;
      d->rangedHasInfiniteAmmo = true;
      d->spawnStandardMinDLVL = -1;
      d->spawnStandardMaxDLVL = -1;
      d->isMeleeWeapon = false;
      d->rangedMissileGlyph = '*';
      d->rangedSndVol = SndVol::low;
    } break;

    case itemData_missileWeapon: {
      resetData(d, itemData_general);
      d->itemWeight = itemWeight_extraLight;
      d->isStackable = true;
      d->isMissileWeapon = true;
      d->spawnStandardMaxDLVL = FIRST_CAVERN_LEVEL - 1;
      d->rangedSndVol = SndVol::low;
    } break;

    case itemData_ammo: {
      resetData(d, itemData_general);
      d->itemWeight = itemWeight_extraLight;
      d->glyph = '{';
      d->clr = clrWhite;
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
      d->itemValue = ItemValue::minorTreasure;
      d->chanceToIncludeInSpawnList = 40;
      d->itemWeight = itemWeight_none;
      d->isIdentified = false;
      d->glyph = '?';
      d->clr = clrWhite;
      d->tile = tile_scroll;
      d->isScroll = true;
      d->maxStackSizeAtSpawn = 1;
      d->landOnHardSurfaceSoundMsg = "";
      addFeatureFoundIn(d, feature_chest);
      addFeatureFoundIn(d, feature_tomb);
      addFeatureFoundIn(d, feature_cabinet, 25);
      addFeatureFoundIn(d, feature_cocoon, 25);
      eng.scrollNameHandler->setFalseScrollName(*d);
    } break;

    case itemData_potion: {
      resetData(d, itemData_general);
      d->itemValue = ItemValue::minorTreasure;
      d->chanceToIncludeInSpawnList = 55;
      d->itemWeight = itemWeight_light;
      d->isIdentified = false;
      d->glyph = '!';
      d->tile = tile_potion;
      d->isPotion = true;
      d->isMissileWeapon = true;
      d->missileHitChanceMod = -5;
      d->missileDmg = DiceParam(1, 3, 0);
      d->maxStackSizeAtSpawn = 2;
      d->landOnHardSurfaceSoundMsg = "";
      addFeatureFoundIn(d, feature_chest);
      addFeatureFoundIn(d, feature_tomb);
      addFeatureFoundIn(d, feature_cabinet, 25);
      addFeatureFoundIn(d, feature_cocoon, 25);
      eng.potionNameHandler->setClrAndFalseName(d);
    } break;

    case itemData_device: {
      resetData(d, itemData_general);
      d->isDevice = true;
      d->chanceToIncludeInSpawnList = 12;
      d->itemWeight = itemWeight_light;
      d->isIdentified = true;
      d->glyph = '~';
      d->tile = tile_device1;
      d->isStackable = false;
      d->spawnStandardMinDLVL = 1;
      d->spawnStandardMaxDLVL = 999;
      d->landOnHardSurfaceSoundMsg = "I hear a clanking sound.";
      d->landOnHardSurfaceSfx = SfxId::metalClank;
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
      d->landOnHardSurfaceSoundMsg = "";
    } break;

    default: {} break;
  }
}

void ItemDataHandler::addFeatureFoundIn(ItemData* const itemData,
                                        const FeatureId featureId,
                                        const int CHANCE_TO_INCLUDE) const {
  itemData->featuresCanBeFoundIn.push_back(
    pair<FeatureId, int>(featureId, CHANCE_TO_INCLUDE));
}

void ItemDataHandler::setDmgFromMonsterData(
  ItemData& itemData, const ActorData& actorData) const {

  itemData.meleeDmg  = pair<int, int>(1, actorData.dmgMelee);
  itemData.rangedDmg = DiceParam(1, actorData.dmgRanged, 0);
}

//------------------------------- LIST OF ITEMS
void ItemDataHandler::initDataList() {
  ItemData* d = NULL;

  d = new ItemData(ItemId::trapezohedron);
  resetData(d, itemData_general);
  d->name = ItemName("Shining Trapezohedron", "Shining Trapezohedrons", "The Shining Trapezohedron");
  d->spawnStandardMinDLVL = -1;
  d->spawnStandardMaxDLVL = -1;
  d->isStackable = false;
  d->glyph = '*';
  d->clr = clrRedLgt;
  d->tile = tile_trapezohedron;
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::sawedOff);
  resetData(d, itemData_rangedWpn);
  d->name = ItemName("Sawed-off Shotgun", "Sawed-off shotguns", "a Sawed-off Shotgun");
  d->itemWeight = itemWeight_medium;
  d->tile = tile_shotgun;
  d->isShotgun = true;
  d->meleeAttackMessages = ItemAttackMessages("strike", "strikes me with a shotgun");
  d->rangedDmg = DiceParam(8, 3);
  d->rangedAmmoTypeUsed = ItemId::shotgunShell;
  d->rangedAttackMessages = ItemAttackMessages("fire", "fires a shotgun");
  d->rangedSndMsg = "I hear a shotgun blast.";
  d->rangedAttackSfx = SfxId::shotgunSawedOffFire;
  d->rangedMakesRicochetSound = true;
  d->reloadSfx = SfxId::shotgunReload;
  addFeatureFoundIn(d, feature_chest);
  addFeatureFoundIn(d, feature_cabinet);
  addFeatureFoundIn(d, feature_cocoon);
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::pumpShotgun);
  resetData(d, itemData_rangedWpn);
  d->name = ItemName("Pump Shotgun", "Pump shotguns", "a Pump Shotgun");
  d->itemWeight = itemWeight_medium;
  d->tile = tile_shotgun;
  d->isShotgun = true;
  d->meleeAttackMessages = ItemAttackMessages("strike", "strikes me with a shotgun");
  d->rangedDmg = DiceParam(6, 3);
  d->rangedAmmoTypeUsed = ItemId::shotgunShell;
  d->rangedAttackMessages = ItemAttackMessages("fire", "fires a shotgun");
  d->rangedSndMsg = "I hear a shotgun blast.";
  d->rangedAttackSfx = SfxId::shotgunPumpFire ;
  d->rangedMakesRicochetSound = true;
  d->reloadSfx = SfxId::shotgunReload;
  addFeatureFoundIn(d, feature_chest);
  addFeatureFoundIn(d, feature_cabinet);
  addFeatureFoundIn(d, feature_cocoon);
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::shotgunShell);
  resetData(d, itemData_ammo);
  d->name = ItemName("Shotgun shell", "Shotgun shells", "a shotgun shell");
  d->maxStackSizeAtSpawn = 10;
  addFeatureFoundIn(d, feature_chest);
  addFeatureFoundIn(d, feature_cabinet);
  addFeatureFoundIn(d, feature_cocoon);
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::incinerator);
  resetData(d, itemData_rangedWpn);
  d->name = ItemName("Incinerator", "Incinerators", "an Incinerator");
  d->itemWeight = itemWeight_heavy;
  d->tile = tile_incinerator;
  d->meleeAttackMessages = ItemAttackMessages("strike", "strikes me with an Incinerator");
  d->rangedDmg = DiceParam(1, 3);
  d->rangedAmmoTypeUsed = ItemId::napalmCartridge;
  d->rangedAttackMessages = ItemAttackMessages("fire", "fires an incinerator");
  d->rangedSndMsg = "I hear the blast of a launched missile.";
  d->rangedMissileGlyph = '*';
  d->rangedMissileClr = clrRedLgt;
  d->spawnStandardMinDLVL = 10;
  d->rangedDmgLabelOverRide = "?";
  addFeatureFoundIn(d, feature_chest, 25);
  addFeatureFoundIn(d, feature_cabinet, 25);
  addFeatureFoundIn(d, feature_cocoon, 25);
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::napalmCartridge);
  resetData(d, itemData_ammoClip);
  d->name = ItemName("Napalm Cartridge", "Napalm Cartridges", "a Napalm Cartridge");
  d->itemWeight = itemWeight_light;
  d->ammoContainedInClip = 3;
  d->spawnStandardMinDLVL = 8;
  d->maxStackSizeAtSpawn = 1;
  addFeatureFoundIn(d, feature_chest, 25);
  addFeatureFoundIn(d, feature_cabinet, 25);
  addFeatureFoundIn(d, feature_cocoon, 25);
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::machineGun);
  resetData(d, itemData_rangedWpn);
  d->name = ItemName("Tommy Gun", "Tommy Guns", "a Tommy Gun");
  d->itemWeight = itemWeight_medium;
  d->tile = tile_tommyGun;
  d->meleeAttackMessages = ItemAttackMessages("strike", "strikes me with a Tommy Gun");
  d->isMachineGun = true;
  d->rangedDmg = DiceParam(2, 2, 2);
  d->rangedHitChanceMod = -10;
  d->rangedAmmoTypeUsed = ItemId::drumOfBullets;
  d->rangedAttackMessages = ItemAttackMessages("fire", "fires a Tommy Gun");
  d->rangedSndMsg = "I hear the burst of a machine gun.";
  d->rangedAttackSfx = SfxId::machineGunFire;
  d->rangedMakesRicochetSound = true;
  d->reloadSfx = SfxId::machineGunReload;
  addFeatureFoundIn(d, feature_chest);
  addFeatureFoundIn(d, feature_cabinet);
  addFeatureFoundIn(d, feature_cocoon);
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::drumOfBullets);
  resetData(d, itemData_ammoClip);
  d->name = ItemName("Drum of .45 ACP", "Drums of .45 ACP", "a Drum of .45 ACP");
  d->ammoContainedInClip = 50;
  addFeatureFoundIn(d, feature_chest);
  addFeatureFoundIn(d, feature_cabinet);
  addFeatureFoundIn(d, feature_cocoon);
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::pistol);
  resetData(d, itemData_rangedWpn);
  d->name = ItemName("M1911 Colt", "M1911 Colt", "an M1911 Colt");
  d->itemWeight = itemWeight_light;
  d->tile = tile_pistol;
  d->rangedDmg = DiceParam(1, 8, 4);
  d->rangedAmmoTypeUsed = ItemId::pistolClip;
  d->meleeAttackMessages = ItemAttackMessages("strike", "strikes me with a pistol");
  d->rangedAttackMessages = ItemAttackMessages("fire", "fires a pistol");
  d->rangedSndMsg = "I hear a pistol being fired.";
  d->rangedAttackSfx = SfxId::pistolFire;
  d->rangedMakesRicochetSound = true;
  d->reloadSfx = SfxId::pistolReload;
  addFeatureFoundIn(d, feature_chest);
  addFeatureFoundIn(d, feature_cabinet);
  addFeatureFoundIn(d, feature_cocoon);
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::flareGun);
  resetData(d, itemData_rangedWpn);
  d->name = ItemName("Flare Gun", "Flare Gun", "a Flare Gun");
  d->itemWeight = itemWeight_light;
  d->tile = tile_flareGun;
  d->rangedDmg = DiceParam(1, 3, 0);
  d->rangedDmgLabelOverRide = "?";
  d->rangedAmmoTypeUsed = ItemId::flare;
  d->meleeAttackMessages = ItemAttackMessages("strike", "strikes me with a flare gun");
  d->rangedAttackMessages = ItemAttackMessages("fire", "fires a flare gun");
  d->rangedSndMsg = "I hear a flare gun being fired.";
  d->propAppliedOnRanged = new PropFlared(eng, propTurnsStd);
  addFeatureFoundIn(d, feature_chest);
  addFeatureFoundIn(d, feature_cabinet);
  addFeatureFoundIn(d, feature_cocoon);
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::pistolClip);
  resetData(d, itemData_ammoClip);
  d->name = ItemName(".45ACP Colt cartridge", ".45ACP Colt cartridges", "a .45ACP Colt cartridge");
  d->ammoContainedInClip = 7;
  addFeatureFoundIn(d, feature_chest);
  addFeatureFoundIn(d, feature_cabinet);
  addFeatureFoundIn(d, feature_cocoon);
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::teslaCannon);
  resetData(d, itemData_rangedWpn);
  d->name = ItemName("Tesla Cannon", "Tesla Cannons", "a Tesla Cannon");
  d->itemWeight = itemWeight_medium;
  d->tile = tile_teslaCannon;
  d->meleeAttackMessages = ItemAttackMessages("strike", "strikes me with a Tesla Cannon");
  d->isMachineGun = true;
  d->rangedHitChanceMod = -15;
  d->rangedDmg = DiceParam(2, 3, 3);
  d->rangedDmgType = DmgType::electric;
  d->rangedAmmoTypeUsed = ItemId::teslaCanister;
  d->rangedAttackMessages = ItemAttackMessages("fire", "fires a Tesla Cannon");
  d->rangedSndMsg = "I hear loud electric crackle.";
  d->rangedMissileGlyph = '*';
  d->rangedMissileClr = clrYellow;
  d->spawnStandardMinDLVL = 7;
  addFeatureFoundIn(d, feature_chest, 50);
  addFeatureFoundIn(d, feature_cabinet, 50);
  addFeatureFoundIn(d, feature_cocoon, 50);
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::teslaCanister);
  resetData(d, itemData_ammoClip);
  d->name = ItemName("Nuclear battery", "Nuclear batteries", "a Nuclear battery");
  d->ammoContainedInClip = 30;
  d->spawnStandardMinDLVL = 6;
  addFeatureFoundIn(d, feature_chest, 50);
  addFeatureFoundIn(d, feature_cabinet, 50);
  addFeatureFoundIn(d, feature_cocoon, 50);
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::spikeGun);
  resetData(d, itemData_rangedWpn);
  d->name = ItemName("Spike Gun", "Spike Guns", "a Spike Gun");
  d->itemWeight = itemWeight_medium;
  d->tile = tile_tommyGun;
  d->clr = clrBlueLgt;
  d->meleeAttackMessages = ItemAttackMessages("strike", "strikes me with a Spike Gun");
  d->isMachineGun = false;
  d->rangedHitChanceMod = 0;
  d->rangedDmg = DiceParam(1, 7, 0);
  d->rangedDmgType = DmgType::physical;
  d->rangedCausesKnockBack = true;
  d->rangedAmmoTypeUsed = ItemId::ironSpike;
  d->rangedAttackMessages = ItemAttackMessages("fire", "fires a Spike Gun");
  d->rangedSndMsg = "I hear a very crude gun being fired.";
  d->rangedMakesRicochetSound = true;
  d->rangedMissileGlyph = '/';
  d->rangedMissileClr = clrGray;
  d->spawnStandardMinDLVL = 4;
  d->rangedSndVol = SndVol::low;
  addFeatureFoundIn(d, feature_chest, 50);
  addFeatureFoundIn(d, feature_cabinet, 50);
  addFeatureFoundIn(d, feature_cocoon, 50);
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::dynamite);
  resetData(d, itemData_explosive);
  d->name = ItemName("Dynamite", "Sticks of Dynamite", "a Stick of Dynamite");
  d->itemWeight = itemWeight_light;
  d->tile = tile_dynamite;
  d->clr = clrRedLgt;
  addFeatureFoundIn(d, feature_chest);
  addFeatureFoundIn(d, feature_cabinet);
  addFeatureFoundIn(d, feature_cocoon);
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::flare);
  resetData(d, itemData_explosive);
  d->name = ItemName("Flare", "Flares", "a Flare");
  d->itemWeight = itemWeight_light;
  d->tile = tile_flare;
  d->clr = clrGray;
  d->isAmmo = true;
  addFeatureFoundIn(d, feature_chest);
  addFeatureFoundIn(d, feature_cabinet);
  addFeatureFoundIn(d, feature_cocoon);
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::molotov);
  resetData(d, itemData_explosive);
  d->name = ItemName("Molotov Cocktail", "Molotov Cocktails", "a Molotov Cocktail");
  d->itemWeight = itemWeight_light;
  d->tile = tile_molotov;
  d->clr = clrWhite;
  addFeatureFoundIn(d, feature_chest);
  addFeatureFoundIn(d, feature_cabinet);
  addFeatureFoundIn(d, feature_cocoon);
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::throwingKnife);
  resetData(d, itemData_missileWeapon);
  d->name = ItemName("Throwing Knife", "Throwing Knives", "a Throwing Knife");
  d->itemWeight = itemWeight_extraLight;
  d->tile = tile_dagger;
  d->glyph = '/';
  d->clr = clrWhite;
  d->missileHitChanceMod = 0;
  d->missileDmg = DiceParam(2, 4);
  d->maxStackSizeAtSpawn = 8;
  d->landOnHardSurfaceSoundMsg = "I hear a clanking sound.";
  d->landOnHardSurfaceSfx = SfxId::metalClank;
  d->primaryAttackMode = PrimaryAttackMode::missile;
  addFeatureFoundIn(d, feature_chest);
  addFeatureFoundIn(d, feature_cabinet);
  addFeatureFoundIn(d, feature_cocoon);
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::rock);
  resetData(d, itemData_missileWeapon);
  d->name = ItemName("Rock", "Rocks", "a Rock");
  d->itemWeight = itemWeight_extraLight;
  d->tile = tile_rock;
  d->glyph = '*';
  d->clr = clrGray;
  d->missileHitChanceMod = 10;
  d->missileDmg = DiceParam(1, 3);
  d->maxStackSizeAtSpawn = 6;
  d->primaryAttackMode = PrimaryAttackMode::missile;
  addFeatureFoundIn(d, feature_cabinet);
  addFeatureFoundIn(d, feature_cocoon);
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::dagger);
  resetData(d, itemData_meleeWpn);
  d->name = ItemName("Dagger", "Daggers", "a Dagger");
  d->itemWeight = itemWeight_light;
  d->tile = tile_dagger;
  d->meleeAttackMessages = ItemAttackMessages("strike", "strikes me with a Dagger");
  d->meleeDmg = pair<int, int>(1, 4);
  d->meleeHitChanceMod = 20;
  d->meleeHitMediumSfx = SfxId::hitSharp;
  d->meleeHitHardSfx = SfxId::hitSharp;
  d->meleeMissSfx = SfxId::missLight;
  addFeatureFoundIn(d, feature_chest);
  addFeatureFoundIn(d, feature_cabinet);
  addFeatureFoundIn(d, feature_tomb);
  addFeatureFoundIn(d, feature_cocoon);
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::hatchet);
  resetData(d, itemData_meleeWpn);
  d->name = ItemName("Hatchet", "Hatchets", "a Hatchet");
  d->itemWeight = itemWeight_light;
  d->tile = tile_axe;
  d->meleeAttackMessages = ItemAttackMessages("strike", "strikes me with a Hatchet");
  d->meleeDmg = pair<int, int>(1, 5);
  d->meleeHitChanceMod = 15;
  d->missileHitChanceMod = -5;
  d->missileDmg = DiceParam(1, 10);
  d->isMissileWeapon = false;
  d->meleeHitMediumSfx = SfxId::hitSharp;
  d->meleeHitHardSfx = SfxId::hitSharp;
  d->meleeMissSfx = SfxId::missLight;
  addFeatureFoundIn(d, feature_chest);
  addFeatureFoundIn(d, feature_cabinet);
  addFeatureFoundIn(d, feature_cocoon);
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::club);
  resetData(d, itemData_meleeWpn);
  d->name = ItemName("Club", "Clubs", "a Club");
  d->spawnStandardMinDLVL = FIRST_CAVERN_LEVEL;
  d->spawnStandardMaxDLVL = 999;
  d->itemWeight = itemWeight_medium;
  d->tile = tile_club;
  d->clr = clrBrown;
  d->meleeAttackMessages = ItemAttackMessages("strike", "strikes me with a Club");
  d->meleeDmg = pair<int, int>(2, 3);
  d->meleeHitChanceMod = 10;
  d->meleeMissSfx = SfxId::missMedium;
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::hammer);
  resetData(d, itemData_meleeWpn);
  d->name = ItemName("Hammer", "Hammers", "a Hammer");
  d->itemWeight = itemWeight_medium;
  d->tile = tile_hammer;
  d->meleeAttackMessages = ItemAttackMessages("strike", "strikes me with a Hammer");
  d->meleeDmg = pair<int, int>(2, 4);
  d->meleeHitChanceMod = 5;
  d->meleeMissSfx = SfxId::missMedium;
  addFeatureFoundIn(d, feature_cabinet);
  addFeatureFoundIn(d, feature_cocoon);
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::machete);
  resetData(d, itemData_meleeWpn);
  d->name = ItemName("Machete", "Machetes", "a Machete");
  d->itemWeight = itemWeight_medium;
  d->tile = tile_machete;
  d->meleeAttackMessages = ItemAttackMessages("strike", "strikes me with a Machete");
  d->meleeDmg = pair<int, int>(2, 5);
  d->meleeHitChanceMod = 0;
  d->meleeHitSmallSfx = SfxId::hitSharp;
  d->meleeHitMediumSfx = SfxId::hitSharp;
  d->meleeMissSfx = SfxId::missMedium;
  addFeatureFoundIn(d, feature_cabinet);
  addFeatureFoundIn(d, feature_cocoon);
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::axe);
  resetData(d, itemData_meleeWpn);
  d->name = ItemName("Axe", "Axes", "an Axe");
  d->itemWeight = itemWeight_medium;
  d->tile = tile_axe;
  d->meleeAttackMessages = ItemAttackMessages("strike", "strikes me with an axe");
  d->meleeDmg = pair<int, int>(2, 6);
  d->meleeHitChanceMod = -5;
  d->meleeHitSmallSfx = SfxId::hitSmall;
  d->meleeHitMediumSfx = SfxId::hitMedium;
  d->meleeHitHardSfx = SfxId::hitHard;
  d->meleeMissSfx = SfxId::missMedium;
  addFeatureFoundIn(d, feature_cabinet);
  addFeatureFoundIn(d, feature_tomb);
  addFeatureFoundIn(d, feature_cocoon);
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::pitchFork);
  resetData(d, itemData_meleeWpn);
  d->name = ItemName("Pitchfork", "Pitchforks", "a Pitchfork");
  d->itemWeight = itemWeight_heavy;
  d->tile = tile_pitchfork;
  d->meleeAttackMessages = ItemAttackMessages("strike", "strikes me with a Pitchfork");
  d->meleeDmg = pair<int, int>(3, 4);
  d->meleeHitChanceMod = -5;
  d->meleeCausesKnockBack = true;
  d->meleeHitSmallSfx = SfxId::hitSharp;
  d->meleeHitMediumSfx = SfxId::hitSharp;
  d->meleeMissSfx = SfxId::missHeavy;
  addFeatureFoundIn(d, feature_cabinet);
  addFeatureFoundIn(d, feature_cocoon);
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::sledgeHammer);
  resetData(d, itemData_meleeWpn);
  d->name = ItemName("Sledgehammer", "Sledgehammers", "a Sledgehammer");
  d->itemWeight = itemWeight_heavy;
  d->tile = tile_sledgeHammer;
  d->meleeAttackMessages = ItemAttackMessages("strike", "strikes me with a Sledgehammer");
  d->meleeDmg = pair<int, int>(3, 5);
  d->meleeHitChanceMod = -10;
  d->meleeCausesKnockBack = true;
  d->meleeMissSfx = SfxId::missHeavy;
  addFeatureFoundIn(d, feature_cabinet);
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::ironSpike);
  resetData(d, itemData_missileWeapon);
  d->name = ItemName("Iron Spike", "Iron Spikes", "an Iron Spike");
  d->isAmmo = true;
  d->itemWeight = itemWeight_extraLight;
  d->tile = tile_ironSpike;
  d->isStackable = true;
  d->clr = clrGray;
  d->glyph = '/';
  d->missileHitChanceMod = -5;
  d->missileDmg = DiceParam(1, 3);
  d->maxStackSizeAtSpawn = 12;
  d->landOnHardSurfaceSoundMsg = "I hear a clanking sound.";
  d->landOnHardSurfaceSfx = SfxId::metalClank;
  d->primaryAttackMode = PrimaryAttackMode::missile;
  addFeatureFoundIn(d, feature_cabinet);
  addFeatureFoundIn(d, feature_cocoon);
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::playerKick);
  resetData(d, itemData_meleeWpnIntr);
  d->meleeAttackMessages = ItemAttackMessages("kick", "");
  d->meleeHitChanceMod = 20;
  d->meleeDmg = pair<int, int>(1, 3);
  d->meleeCausesKnockBack = true;
  d->meleeMissSfx = SfxId::missMedium;
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::playerStomp);
  resetData(d, itemData_meleeWpnIntr);
  d->meleeAttackMessages = ItemAttackMessages("stomp", "");
  d->meleeHitChanceMod = 20;
  d->meleeDmg = pair<int, int>(1, 3);
  d->meleeCausesKnockBack = false;
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::playerPunch);
  resetData(d, itemData_meleeWpnIntr);
  d->meleeAttackMessages = ItemAttackMessages("punch", "");
  d->meleeHitChanceMod = 25;
  d->meleeDmg = pair<int, int>(1, 2);
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::zombieClaw);
  resetData(d, itemData_meleeWpnIntr);
  d->meleeAttackMessages = ItemAttackMessages("", "claws me");
  setDmgFromMonsterData(*d, eng.actorDataHandler->dataList[actor_zombie]);
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::zombieClawDiseased);
  resetData(d, itemData_meleeWpnIntr);
  d->meleeAttackMessages = ItemAttackMessages("", "claws me");
  setDmgFromMonsterData(*d, eng.actorDataHandler->dataList[actor_zombie]);
  d->propAppliedOnMelee = new PropInfected(eng, propTurnsStd);
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::zombieAxe);
  resetData(d, itemData_meleeWpnIntr);
  d->meleeAttackMessages = ItemAttackMessages("", "chops me with a rusty axe");
  d->meleeHitSmallSfx = SfxId::hitSmall;
  d->meleeHitMediumSfx = SfxId::hitMedium;
  d->meleeHitHardSfx = SfxId::hitHard;
  setDmgFromMonsterData(*d, eng.actorDataHandler->dataList[actor_zombieAxe]);
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::bloatedZombiePunch);
  resetData(d, itemData_meleeWpnIntr);
  d->meleeAttackMessages = ItemAttackMessages("", "mauls me");
  setDmgFromMonsterData(
    *d, eng.actorDataHandler->dataList[actor_bloatedZombie]);
  d->meleeCausesKnockBack = true;
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::bloatedZombieSpit);
  resetData(d, itemData_rangedWpnIntr);
  d->rangedAttackMessages = ItemAttackMessages("", "spits acid pus at me");
  setDmgFromMonsterData(
    *d, eng.actorDataHandler->dataList[actor_bloatedZombie]);
  d->rangedSndMsg = "I hear spitting.";
  d->rangedMissileClr = clrGreenLgt;
  d->rangedDmgType = DmgType::acid;
  d->rangedMissileGlyph = '*';
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::ratBite);
  resetData(d, itemData_meleeWpnIntr);
  d->meleeAttackMessages = ItemAttackMessages("", "bites me");
  setDmgFromMonsterData(*d, eng.actorDataHandler->dataList[actor_rat]);
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::ratBiteDiseased);
  resetData(d, itemData_meleeWpnIntr);
  d->meleeAttackMessages = ItemAttackMessages("", "bites me");
  setDmgFromMonsterData(*d, eng.actorDataHandler->dataList[actor_rat]);
  d->propAppliedOnMelee = new PropInfected(eng, propTurnsStd);
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::ratThingBite);
  resetData(d, itemData_meleeWpnIntr);
  d->meleeAttackMessages = ItemAttackMessages("", "bites me");
  setDmgFromMonsterData(*d, eng.actorDataHandler->dataList[actor_ratThing]);
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::wormMassBite);
  resetData(d, itemData_meleeWpnIntr);
  d->meleeAttackMessages = ItemAttackMessages("", "bites me");
  setDmgFromMonsterData(*d, eng.actorDataHandler->dataList[actor_wormMass]);
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::wolfBite);
  resetData(d, itemData_meleeWpnIntr);
  d->meleeAttackMessages = ItemAttackMessages("", "bites me");
  setDmgFromMonsterData(*d, eng.actorDataHandler->dataList[actor_wolf]);
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::greenSpiderBite);
  resetData(d, itemData_meleeWpnIntr);
  d->meleeAttackMessages = ItemAttackMessages("", "bites me");
  setDmgFromMonsterData(
    *d, eng.actorDataHandler->dataList[actor_greenSpider]);
  d->propAppliedOnMelee = new PropBlind(eng, propTurnsSpecific, 4);
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::whiteSpiderBite);
  resetData(d, itemData_meleeWpnIntr);
  d->meleeAttackMessages = ItemAttackMessages("", "bites me");
  setDmgFromMonsterData(
    *d, eng.actorDataHandler->dataList[actor_whiteSpider]);
  d->propAppliedOnMelee = new PropParalyzed(eng, propTurnsSpecific, 3);
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::redSpiderBite);
  resetData(d, itemData_meleeWpnIntr);
  d->meleeAttackMessages = ItemAttackMessages("", "bites me");
  setDmgFromMonsterData(*d, eng.actorDataHandler->dataList[actor_redSpider]);
  d->propAppliedOnMelee = new PropWeakened(eng, propTurnsStd);
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::shadowSpiderBite);
  resetData(d, itemData_meleeWpnIntr);
  d->meleeAttackMessages = ItemAttackMessages("", "bites me");
  setDmgFromMonsterData(
    *d, eng.actorDataHandler->dataList[actor_shadowSpider]);
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::lengSpiderBite);
  resetData(d, itemData_meleeWpnIntr);
  d->meleeAttackMessages = ItemAttackMessages("", "bites me");
  setDmgFromMonsterData(
    *d, eng.actorDataHandler->dataList[actor_lengSpider]);
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::fireHoundBreath);
  resetData(d, itemData_rangedWpnIntr);
  d->rangedAttackMessages = ItemAttackMessages("", "breaths fire at me");
  d->rangedSndMsg = "I hear a burst of flames.";
  setDmgFromMonsterData(*d, eng.actorDataHandler->dataList[actor_fireHound]);
  d->propAppliedOnRanged = new PropBurning(eng, propTurnsStd);
  d->rangedMissileClr = clrRedLgt;
  d->rangedMissileGlyph = '*';
  d->rangedMissileLeavesTrail = true;
  d->rangedMissileLeavesSmoke = true;
  d->rangedDmgType = DmgType::fire;
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::fireHoundBite);
  resetData(d, itemData_meleeWpnIntr);
  d->meleeAttackMessages = ItemAttackMessages("", "bites me");
  setDmgFromMonsterData(*d, eng.actorDataHandler->dataList[actor_fireHound]);
  d->meleeDmgType = DmgType::fire;
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::frostHoundBreath);
  resetData(d, itemData_rangedWpnIntr);
  d->rangedAttackMessages = ItemAttackMessages("", "breaths frost at me");
  d->rangedSndMsg = "I hear a chilling sound.";
  setDmgFromMonsterData(*d, eng.actorDataHandler->dataList[actor_frostHound]);
  d->rangedMissileClr = clrBlueLgt;
  d->rangedMissileGlyph = '*';
  d->rangedMissileLeavesTrail = true;
  d->rangedMissileLeavesSmoke = true;
  d->rangedDmgType = DmgType::cold;
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::frostHoundBite);
  resetData(d, itemData_meleeWpnIntr);
  d->meleeAttackMessages = ItemAttackMessages("", "bites me");
  setDmgFromMonsterData(*d, eng.actorDataHandler->dataList[actor_frostHound]);
  d->meleeDmgType = DmgType::cold;
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::zuulBite);
  resetData(d, itemData_meleeWpnIntr);
  d->meleeAttackMessages = ItemAttackMessages("", "bites me");
  setDmgFromMonsterData(*d, eng.actorDataHandler->dataList[actor_zuul]);
  d->meleeDmgType = DmgType::physical;
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::dustVortexEngulf);
  resetData(d, itemData_meleeWpnIntr);
  d->meleeAttackMessages = ItemAttackMessages("", "engulfs me");
  setDmgFromMonsterData(
    *d, eng.actorDataHandler->dataList[actor_dustVortex]);
  d->propAppliedOnMelee = new PropBlind(eng, propTurnsStd);
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::fireVortexEngulf);
  resetData(d, itemData_meleeWpnIntr);
  d->meleeAttackMessages = ItemAttackMessages("", "engulfs me");
  setDmgFromMonsterData(
    *d, eng.actorDataHandler->dataList[actor_fireVortex]);
  d->propAppliedOnMelee = new PropBurning(eng, propTurnsStd);
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::frostVortexEngulf);
  resetData(d, itemData_meleeWpnIntr);
  d->meleeAttackMessages = ItemAttackMessages("", "engulfs me");
  setDmgFromMonsterData(
    *d, eng.actorDataHandler->dataList[actor_frostVortex]);
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::ghostClaw);
  resetData(d, itemData_meleeWpnIntr);
  d->meleeAttackMessages = ItemAttackMessages("", "claws me");
  setDmgFromMonsterData(*d, eng.actorDataHandler->dataList[actor_ghost]);
  d->propAppliedOnMelee = new PropTerrified(eng, propTurnsSpecific, 4);
  d->meleeDmgType = DmgType::spirit;
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::phantasmSickle);
  resetData(d, itemData_meleeWpnIntr);
  d->meleeAttackMessages = ItemAttackMessages("", "slices me with a sickle");
  setDmgFromMonsterData(*d, eng.actorDataHandler->dataList[actor_phantasm]);
  d->propAppliedOnMelee = new PropTerrified(eng, propTurnsSpecific, 4);
  d->meleeDmgType = DmgType::spirit;
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::wraithClaw);
  resetData(d, itemData_meleeWpnIntr);
  d->meleeAttackMessages = ItemAttackMessages("", "claws me");
  setDmgFromMonsterData(*d, eng.actorDataHandler->dataList[actor_wraith]);
  d->propAppliedOnMelee = new PropTerrified(eng, propTurnsSpecific, 4);
  d->meleeDmgType = DmgType::spirit;
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::giantBatBite);
  resetData(d, itemData_meleeWpnIntr);
  d->meleeAttackMessages = ItemAttackMessages("", "bites me");
  setDmgFromMonsterData(*d, eng.actorDataHandler->dataList[actor_giantBat]);
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::miGoElectricGun);
  resetData(d, itemData_rangedWpnIntr);
  d->rangedMissileLeavesTrail = true;
  d->rangedMissileClr = clrYellow;
  d->rangedMissileGlyph = '/';
  d->rangedAttackMessages = ItemAttackMessages("", "fires an electric gun");
  d->rangedDmgType = DmgType::electric;
  d->propAppliedOnRanged = new PropParalyzed(eng, propTurnsSpecific, 2);
  d->rangedSndMsg = "I hear a bolt of electricity.";
  setDmgFromMonsterData(*d, eng.actorDataHandler->dataList[actor_miGo]);
  d->rangedSndVol = SndVol::high;
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::polypTentacle);
  resetData(d, itemData_meleeWpnIntr);
  d->meleeAttackMessages = ItemAttackMessages("", "grips me with a tentacle");
  d->propAppliedOnMelee = new PropParalyzed(eng, propTurnsSpecific, 1);
  setDmgFromMonsterData(*d, eng.actorDataHandler->dataList[actor_flyingPolyp]);
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::ghoulClaw);
  resetData(d, itemData_meleeWpnIntr);
  d->meleeAttackMessages = ItemAttackMessages("", "claws me");
  setDmgFromMonsterData(*d, eng.actorDataHandler->dataList[actor_ghoul]);
  d->propAppliedOnMelee = new PropInfected(eng, propTurnsStd);
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::shadowClaw);
  resetData(d, itemData_meleeWpnIntr);
  d->meleeAttackMessages = ItemAttackMessages("", "claws me");
  setDmgFromMonsterData(*d, eng.actorDataHandler->dataList[actor_shadow]);
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::byakheeClaw);
  resetData(d, itemData_meleeWpnIntr);
  d->meleeAttackMessages = ItemAttackMessages("", "claws me");
  setDmgFromMonsterData(*d, eng.actorDataHandler->dataList[actor_byakhee]);
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::giantMantisClaw);
  resetData(d, itemData_meleeWpnIntr);
  d->meleeAttackMessages = ItemAttackMessages("", "claws me");
  setDmgFromMonsterData(
    *d, eng.actorDataHandler->dataList[actor_giantMantis]);
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::giantLocustBite);
  resetData(d, itemData_meleeWpnIntr);
  d->meleeAttackMessages = ItemAttackMessages("", "bites me");
  setDmgFromMonsterData(
    *d, eng.actorDataHandler->dataList[actor_giantLocust]);
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::mummyMaul);
  resetData(d, itemData_meleeWpnIntr);
  d->meleeAttackMessages = ItemAttackMessages("", "mauls me");
  setDmgFromMonsterData(*d, eng.actorDataHandler->dataList[actor_mummy]);
  d->propAppliedOnMelee = new PropCursed(eng, propTurnsStd);
  d->meleeCausesKnockBack = true;
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::deepOneJavelinAttack);
  resetData(d, itemData_rangedWpnIntr);
  d->rangedAttackMessages = ItemAttackMessages("", "throws a Javelin at me");
  setDmgFromMonsterData(*d, eng.actorDataHandler->dataList[actor_deepOne]);
  d->rangedSndMsg = "";
  d->rangedMissileClr = clrBrown;
  d->rangedMissileGlyph = '/';
  d->rangedSndVol = SndVol::low;
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::deepOneSpearAttack);
  resetData(d, itemData_meleeWpnIntr);
  d->meleeAttackMessages = ItemAttackMessages("", "thrusts a spear at me");
  setDmgFromMonsterData(*d, eng.actorDataHandler->dataList[actor_deepOne]);
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::oozeBlackSpewPus);
  resetData(d, itemData_meleeWpnIntr);
  d->meleeAttackMessages = ItemAttackMessages("", "spews pus on me");
  setDmgFromMonsterData(*d, eng.actorDataHandler->dataList[actor_oozeBlack]);
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::oozeClearSpewPus);
  resetData(d, itemData_meleeWpnIntr);
  d->meleeAttackMessages = ItemAttackMessages("", "spews pus on me");
  setDmgFromMonsterData(
    *d, eng.actorDataHandler->dataList[actor_oozeClear]);
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::oozePutridSpewPus);
  resetData(d, itemData_meleeWpnIntr);
  d->meleeAttackMessages = ItemAttackMessages("", "spews infected pus on me");
  setDmgFromMonsterData(
    *d, eng.actorDataHandler->dataList[actor_oozePutrid]);
  d->propAppliedOnMelee = new PropInfected(eng, propTurnsStd);
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::oozePoisonSpewPus);
  resetData(d, itemData_meleeWpnIntr);
  d->meleeAttackMessages = ItemAttackMessages("", "spews poisonous pus on me");
  setDmgFromMonsterData(
    *d, eng.actorDataHandler->dataList[actor_oozePoison]);
  d->propAppliedOnMelee = new PropPoisoned(eng, propTurnsStd);
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::colourOutOfSpaceTouch);
  resetData(d, itemData_meleeWpnIntr);
  d->meleeAttackMessages = ItemAttackMessages("", "touches me");
  setDmgFromMonsterData(
    *d, eng.actorDataHandler->dataList[actor_colourOutOfSpace]);
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::chthonianBite);
  resetData(d, itemData_meleeWpnIntr);
  d->meleeAttackMessages = ItemAttackMessages("", "strikes me with a tentacle");
  d->meleeCausesKnockBack = true;
  setDmgFromMonsterData(
    *d, eng.actorDataHandler->dataList[actor_chthonian]);
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::huntingHorrorBite);
  resetData(d, itemData_meleeWpnIntr);
  d->meleeAttackMessages = ItemAttackMessages("", "bites me");
  setDmgFromMonsterData(
    *d, eng.actorDataHandler->dataList[actor_huntingHorror]);
  d->propAppliedOnMelee = new PropParalyzed(eng, propTurnsStd);
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::armorLeatherJacket);
  resetData(d, itemData_armor);
  d->name = ItemName("Leather Jacket", "", "a Leather Jacket");
  d->itemWeight = itemWeight_light;
  d->clr = clrBrown;
  d->spawnStandardMinDLVL = 1;
  d->armorData.absorptionPoints = 1;
  d->armorData.dmgToDurabilityFactor = 1.0;
  d->landOnHardSurfaceSoundMsg = "";
  addFeatureFoundIn(d, feature_cabinet);
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::armorIronSuit);
  resetData(d, itemData_armor);
  d->name = ItemName("Iron Suit", "", "an Iron Suit");
  d->itemWeight = itemWeight_heavy;
  d->clr = clrWhite;
  d->spawnStandardMinDLVL = 2;
  d->armorData.absorptionPoints = 4;
  d->armorData.dmgToDurabilityFactor = 0.5;
  d->landOnHardSurfaceSoundMsg = "I hear a crashing sound.";
  addFeatureFoundIn(d, feature_cabinet);
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::armorFlackJacket);
  resetData(d, itemData_armor);
  d->name = ItemName("Flak Jacket", "", "a Flak Jacket");
  d->itemWeight = itemWeight_heavy;
  d->clr = clrGreen;
  d->spawnStandardMinDLVL = 3;
  d->armorData.absorptionPoints = 3;
  d->armorData.dmgToDurabilityFactor = 0.5;
  d->landOnHardSurfaceSoundMsg = "I hear a thudding sound.";
  addFeatureFoundIn(d, feature_cabinet);
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::armorAsbestosSuit);
  resetData(d, itemData_armor);
  d->name = ItemName("Asbestos Suit", "", "an Asbestos Suit");
  d->itemWeight = itemWeight_medium;
  d->clr = clrRedLgt;
  d->spawnStandardMinDLVL = 3;
  d->armorData.absorptionPoints = 1;
  d->armorData.dmgToDurabilityFactor = 1.0;
  d->landOnHardSurfaceSoundMsg = "";
  addFeatureFoundIn(d, feature_cabinet);
  addFeatureFoundIn(d, feature_chest);
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::armorHeavyCoat);
  resetData(d, itemData_armor);
  d->name = ItemName("Heavy Coat", "", "a Heavy Coat");
  d->itemWeight = itemWeight_medium;
  d->clr = clrBlueLgt;
  d->spawnStandardMinDLVL = 3;
  d->armorData.absorptionPoints = 1;
  d->armorData.dmgToDurabilityFactor = 1.0;
  d->landOnHardSurfaceSoundMsg = "";
  addFeatureFoundIn(d, feature_cabinet);
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::scrollOfMayhem);
  resetData(d, itemData_scroll);
  d->spellCastFromScroll = SpellId::mayhem;
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::scrollOfTelep);
  resetData(d, itemData_scroll);
  d->spellCastFromScroll = SpellId::teleport;
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::scrollOfPestilence);
  resetData(d, itemData_scroll);
  d->spellCastFromScroll = SpellId::pestilence;
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::scrollOfSlowEnemies);
  resetData(d, itemData_scroll);
  d->spellCastFromScroll = SpellId::slowEnemies;
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::scrollOfTerrifyEnemies);
  resetData(d, itemData_scroll);
  d->spellCastFromScroll = SpellId::terrifyEnemies;
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::scrollOfParalyzeEnemies);
  resetData(d, itemData_scroll);
  d->spellCastFromScroll = SpellId::paralyzeEnemies;
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::scrollOfDetItems);
  resetData(d, itemData_scroll);
  d->spellCastFromScroll = SpellId::detectItems;
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::scrollOfDetTraps);
  resetData(d, itemData_scroll);
  d->spellCastFromScroll = SpellId::detectTraps;
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::scrollOfBlessing);
  resetData(d, itemData_scroll);
  d->spellCastFromScroll = SpellId::bless;
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::scrollOfDarkbolt);
  resetData(d, itemData_scroll);
  d->spellCastFromScroll = SpellId::darkbolt;
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::scrollOfAzathothsWrath);
  resetData(d, itemData_scroll);
  d->spellCastFromScroll = SpellId::azathothsWrath;
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::scrollOfOpening);
  resetData(d, itemData_scroll);
  d->spellCastFromScroll = SpellId::opening;
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::scrollOfSacrLife);
  resetData(d, itemData_scroll);
  d->spellCastFromScroll = SpellId::sacrificeLife;
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::scrollOfSacrSpi);
  resetData(d, itemData_scroll);
  d->spellCastFromScroll = SpellId::sacrificeSpirit;
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::scrollOfDetMon);
  resetData(d, itemData_scroll);
  d->spellCastFromScroll = SpellId::detectMonsters;
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::potionOfVitality);
  resetData(d, itemData_potion);
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::potionOfSpirit);
  resetData(d, itemData_potion);
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::potionOfBlindness);
  resetData(d, itemData_potion);
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::potionOfFrenzy);
  resetData(d, itemData_potion);
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::potionOfFortitude);
  resetData(d, itemData_potion);
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::potionOfParalyzation);
  resetData(d, itemData_potion);
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::potionOfRElec);
  resetData(d, itemData_potion);
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::potionOfConfusion);
  resetData(d, itemData_potion);
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::potionOfPoison);
  resetData(d, itemData_potion);
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::potionOfInsight);
  resetData(d, itemData_potion);
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::potionOfClairv);
  resetData(d, itemData_potion);
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::potionOfRFire);
  resetData(d, itemData_potion);
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::potionOfAntidote);
  resetData(d, itemData_potion);
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::potionOfDescent);
  resetData(d, itemData_potion);
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::deviceSentry);
  resetData(d, itemData_device);
  d->name = ItemName("Sentry Device", "Sentry Devices", "a Sentry Device");
  d->itemValue = ItemValue::majorTreasure;
  d->isIdentified = false;
  d->clr = clrGray;
  addFeatureFoundIn(d, feature_chest, 10);
  addFeatureFoundIn(d, feature_tomb, 10);
  addFeatureFoundIn(d, feature_cocoon, 10);
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::deviceRepeller);
  resetData(d, itemData_device);
  d->name = ItemName("Repeller Device", "Repeller Devices", "a Repeller Device");
  d->itemValue = ItemValue::majorTreasure;
  d->isIdentified = false;
  d->clr = clrGray;
  addFeatureFoundIn(d, feature_chest, 10);
  addFeatureFoundIn(d, feature_tomb, 10);
  addFeatureFoundIn(d, feature_cocoon, 10);
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::deviceRejuvenator);
  resetData(d, itemData_device);
  d->name = ItemName("Rejuvenator Device", "Rejuvenator Devices", "a Rejuvenator Device");
  d->itemValue = ItemValue::majorTreasure;
  d->isIdentified = false;
  d->clr = clrGray;
  addFeatureFoundIn(d, feature_chest, 10);
  addFeatureFoundIn(d, feature_tomb, 10);
  addFeatureFoundIn(d, feature_cocoon, 10);
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::deviceTranslocator);
  resetData(d, itemData_device);
  d->name = ItemName("Translocator Device", "Translocator Devices", "a Translocator Device");
  d->itemValue = ItemValue::majorTreasure;
  d->isIdentified = false;
  d->clr = clrGray;
  addFeatureFoundIn(d, feature_chest, 10);
  addFeatureFoundIn(d, feature_tomb, 10);
  addFeatureFoundIn(d, feature_cocoon, 10);
  dataList[int(d->id)] = d;

//  d = new ItemData(ItemId::deviceSpellReflector);
//  resetData(d, itemData_device);
//  d->name = ItemName("Spell Reflector Device", "Spell Reflector Devices", "a Spell Reflector Device");
//  d->clr = clrGray;
//  d->featuresCanBeFoundIn.push_back(feature_chest);
//  d->featuresCanBeFoundIn.push_back(feature_tomb);
//  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::electricLantern);
  resetData(d, itemData_device);
  d->name = ItemName("Electric Lantern", "Electric Lanterns", "an Electric Lantern");
  d->spawnStandardMinDLVL = 1;
  d->spawnStandardMaxDLVL = 10;
  d->chanceToIncludeInSpawnList = 50;
  d->isIdentified = true;
  d->tile = tile_electricLantern;
  d->clr = clrYellow;
  addFeatureFoundIn(d, feature_chest);
  addFeatureFoundIn(d, feature_cabinet);
  addFeatureFoundIn(d, feature_cocoon);
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::medicalBag);
  resetData(d, itemData_general);
  d->isMedicalBag = true;
  d->name = ItemName("Medical Bag", "Medical Bags", "a Medical Bag");
  d->itemValue = ItemValue::normal;
  d->itemWeight = itemWeight_medium;
  d->spawnStandardMinDLVL = 1;
  d->spawnStandardMaxDLVL = LAST_ROOM_AND_CORRIDOR_LEVEL;
  d->isStackable = false;
  d->glyph = '~';
  d->clr = clrBrownDrk;
  d->tile = tile_medicalBag;
//  d->nativeRooms.resize(0);
  dataList[int(d->id)] = d;
}

void ItemDataHandler::addSaveLines(vector<string>& lines) const {
  for(int i = 1; i < int(ItemId::endOfItemIds); i++) {
    lines.push_back(dataList[i]->isIdentified ? "1" : "0");

    if(dataList[i]->isScroll) {
      lines.push_back(dataList[i]->isTried ? "1" : "0");
    }
  }
}

void ItemDataHandler::setParamsFromSaveLines(vector<string>& lines) {
  for(int i = 1; i < int(ItemId::endOfItemIds); i++) {
    dataList[i]->isIdentified = lines.front() == "0" ? false : true;
    lines.erase(lines.begin());

    if(dataList[i]->isScroll) {
      dataList[i]->isTried = lines.front() == "0" ? false : true;
      lines.erase(lines.begin());
    }
  }
}

//TODO Remove this function
bool ItemDataHandler::isWeaponStronger(
  const ItemData& data1, const ItemData& data2, const bool IS_MELEE) {
  (void)data1;
  (void)data2;
  (void)IS_MELEE;

  return false;
}

//TODO Remove this function and make getName() function in Item instead
string ItemDataHandler::getItemRef(
  const Item& item, const ItemRefType itemRefForm,
  const bool SKIP_EXTRA_INFO) const {

  const ItemData& d = item.getData();
  string ret = "";

  if(d.isDevice && d.id != ItemId::electricLantern) {
    if(d.isIdentified) {
      ret = itemRefForm == ItemRefType::plain ? d.name.name : d.name.name_a;
      if(SKIP_EXTRA_INFO == false) {
        const Condition cond = dynamic_cast<const Device*>(&item)->condition_;
        switch(cond) {
          case Condition::breaking: ret += " {breaking}"; break;
          case Condition::shoddy:   ret += " {shoddy}";   break;
          case Condition::fine:     ret += " {fine}";     break;
        }
      }
      return ret;
    } else {
      ret = itemRefForm == ItemRefType::plain ? "" : "a ";
      return ret + "Strange Device";
    }
  }

  if(d.isStackable && item.nrItems > 1 && itemRefForm == ItemRefType::plural) {
    ret = toStr(item.nrItems) + " ";
    ret += d.name.name_plural;
  } else {
    ret = itemRefForm == ItemRefType::plain ? d.name.name : d.name.name_a;
  }

  if(d.isAmmoClip) {
    const ItemAmmoClip* const ammoItem =
      dynamic_cast<const ItemAmmoClip*>(&item);
    return ret + " {" + toStr(ammoItem->ammo) + "}";
  }

  if(d.isMedicalBag) {
    const MedicalBag* const medicalBag =
      dynamic_cast<const MedicalBag*>(&item);
    return ret + " {" + toStr(medicalBag->getNrSupplies()) + "}";
  }

  if(SKIP_EXTRA_INFO == false) {
    if(d.isRangedWeapon) {
      string ammoLoadedStr = "";
      if(d.rangedHasInfiniteAmmo == false) {
        const Weapon* const w = dynamic_cast<const Weapon*>(&item);
        ammoLoadedStr = " " + toStr(w->nrAmmoLoaded) + "/" +
                        toStr(w->ammoCapacity);
      }
      return ret + ammoLoadedStr;
    }

    if((d.isScroll || d.isPotion) && d.isTried && d.isIdentified == false) {
      return ret + " {tried}";
    }
  }

  return ret;
}

//TODO Remove this function and make getName() function in Item instead
string ItemDataHandler::getItemInterfaceRef(
  const Item& item, const bool ADD_A,
  const PrimaryAttackMode attackMode) const {
  const ItemData& d = item.getData();

  if(d.isDevice && d.id != ItemId::electricLantern) {
    if(d.isIdentified) {
      string ret = ADD_A ? d.name.name_a : d.name.name;
      const Condition cond = dynamic_cast<const Device*>(&item)->condition_;
      switch(cond) {
        case Condition::breaking: ret += " {breaking}"; break;
        case Condition::shoddy:   ret += " {shoddy}";   break;
        case Condition::fine:     ret += " {fine}";     break;
      }
      return ret;
    } else {
      string ret = ADD_A ? "a " : "";
      return ret + "Strange Device";
    }
  }

  string ret = "";

  if(d.isStackable && item.nrItems > 1) {
    ret = toStr(item.nrItems) + " " + d.name.name_plural;
  } else {
    ret = (ADD_A ? d.name.name_a : d.name.name);
  }

  const int PLAYER_RANGED_SKILL =
    eng.player->getData().abilityVals.getVal(
      ability_accuracyRanged, true, *(eng.player));

  if(
    (attackMode == PrimaryAttackMode::none &&
     d.primaryAttackMode == PrimaryAttackMode::melee) ||
    (attackMode == PrimaryAttackMode::melee && d.isMeleeWeapon)) {
    const string rollsStr = toStr(d.meleeDmg.first);
    const string sidesStr = toStr(d.meleeDmg.second);
    const int PLUS = dynamic_cast<const Weapon*>(&item)->meleeDmgPlus;
    const string plusStr = PLUS ==  0 ? "" : ((PLUS > 0 ? "+" : "") +
                           toStr(PLUS));
    const int ITEM_SKILL = d.meleeHitChanceMod;
    const int PLAYER_MELEE_SKILL =
      eng.player->getData().abilityVals.getVal(
        ability_accuracyMelee, true, *(eng.player));
    const int TOTAL_SKILL = max(0, min(100, ITEM_SKILL + PLAYER_MELEE_SKILL));
    const string skillStr = toStr(TOTAL_SKILL) + "%";
    return ret + " " + rollsStr + "d" + sidesStr + plusStr + " " + skillStr;
  }

  if(
    (attackMode == PrimaryAttackMode::none &&
     d.primaryAttackMode == PrimaryAttackMode::ranged) ||
    (attackMode == PrimaryAttackMode::ranged && d.isRangedWeapon)) {
    const int MULTIPL = d.isMachineGun == true ?
                        NR_MG_PROJECTILES : 1;
    const string rollsStr = toStr(d.rangedDmg.rolls * MULTIPL);
    const string sidesStr = toStr(d.rangedDmg.sides);
    const int PLUS = d.rangedDmg.plus * MULTIPL;
    const string plusStr = PLUS ==  0 ? "" : ((PLUS > 0 ? "+" : "") +
                           toStr(PLUS));
    const int ITEM_SKILL = d.rangedHitChanceMod;
    const int TOTAL_SKILL = max(0, min(100, ITEM_SKILL + PLAYER_RANGED_SKILL));
    const string skillStr = toStr(TOTAL_SKILL) + "%";
    string ammoLoadedStr = "";
    if(d.rangedHasInfiniteAmmo == false) {
      const Weapon* const w = dynamic_cast<const Weapon*>(&item);
      ammoLoadedStr = " " + toStr(w->nrAmmoLoaded) + "/" +
                      toStr(w->ammoCapacity);
    }
    return ret + " " + rollsStr + "d" + sidesStr + plusStr + " " + skillStr +
           ammoLoadedStr;
  }

  if(
    (attackMode == PrimaryAttackMode::none &&
     d.primaryAttackMode == PrimaryAttackMode::missile) ||
    (attackMode == PrimaryAttackMode::missile && d.isMissileWeapon)) {
    const string rollsStr = toStr(d.missileDmg.rolls);
    const string sidesStr = toStr(d.missileDmg.sides);
    const int PLUS = d.missileDmg.plus;
    const string plusStr = PLUS ==  0 ? "" : ((PLUS > 0 ? "+" : "") +
                           toStr(PLUS));
    const int ITEM_SKILL = d.missileHitChanceMod;
    const int TOTAL_SKILL = max(0, min(100, ITEM_SKILL + PLAYER_RANGED_SKILL));
    const string skillStr = toStr(TOTAL_SKILL) + "%";
    return ret + " " + rollsStr + "d" + sidesStr + plusStr + " " + skillStr;
  }

  if(d.isMedicalBag) {
    const MedicalBag* const medicalBag = dynamic_cast<const MedicalBag*>(&item);
    return ret + " {" + toStr(medicalBag->getNrSupplies()) + "}";
  }

  if(d.isAmmoClip) {
    const ItemAmmoClip* const clip = dynamic_cast<const ItemAmmoClip*>(&item);
    return ret + " {" + toStr(clip->ammo) + "}";
  }

  if(d.isArmor) {
    const string armorDataLine =
      dynamic_cast<const Armor*>(&item)->getArmorDataLine(true);
    return armorDataLine.empty() ? ret : ret + " " + armorDataLine;
  }

  if((d.isScroll || d.isPotion) && d.isTried && d.isIdentified == false) {
    return ret + " {tried}";
  }

  return ret;
}

