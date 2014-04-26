#include "ItemData.h"

#include <iostream>

#include "Colors.h"
#include "Item.h"
#include "ItemWeapon.h"
#include "ItemAmmo.h"
#include "CmnData.h"
#include "ActorData.h"
#include "ItemArmor.h"
#include "ItemScroll.h"
#include "ItemPotion.h"
#include "ActorPlayer.h"
#include "ItemMedicalBag.h"
#include "Sound.h"
#include "ItemDevice.h"

using namespace std;

namespace ItemData {

namespace {

//------------------------------- ITEM ARCHETYPES (DEFAULTS)
void ItemDataHandler::resetData(ItemData* const d,
                                ItemType const itemType) const {
  switch(itemType) {
    case ItemType::general: {
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
      d->primaryAttackMode = PrimaryAttMode::none;
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
      d->rangedAmmoTypeUsed = ItemId::empty;
      d->rangedDmg = d->missileDmg = DiceParam(0, 0, 0);
      d->meleeDmg = pair<int, int>(0, 0);
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

    case ItemType::meleeWpn: {
      resetData(d, ItemType::general);
      d->isStackable = false;
      d->itemWeight = itemWeight_medium;
      d->glyph = '(';
      d->clr = clrWhite;
      d->primaryAttackMode = PrimaryAttMode::melee;
      d->isMeleeWeapon = true;
      d->meleeHitSmallSfx = SfxId::hitSmall;
      d->meleeHitMediumSfx = SfxId::hitMedium;
      d->meleeHitHardSfx = SfxId::hitHard;
    } break;

    case ItemType::meleeWpnIntr: {
      resetData(d, ItemType::meleeWpn);
      d->isIntrinsic = true;
      d->spawnStandardMinDLVL = -1;
      d->spawnStandardMaxDLVL = -1;
      d->meleeHitSmallSfx = SfxId::hitSmall;
      d->meleeHitMediumSfx = SfxId::hitMedium;
      d->meleeHitHardSfx = SfxId::hitHard;
      d->meleeMissSfx = SfxId::missMedium;
    } break;

    case ItemType::rangedWpn: {
      resetData(d, ItemType::general);
      d->isStackable = false;
      d->itemWeight = itemWeight_medium;
      d->glyph = '}';
      d->clr = clrWhite;
      d->isMeleeWeapon = true;
      d->meleeDmg = pair<int, int>(1, 6);
      d->primaryAttackMode = PrimaryAttMode::ranged;
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

    case ItemType::rangedWpnIntr: {
      resetData(d, ItemType::rangedWpn);
      d->isIntrinsic = true;
      d->rangedHasInfiniteAmmo = true;
      d->spawnStandardMinDLVL = -1;
      d->spawnStandardMaxDLVL = -1;
      d->isMeleeWeapon = false;
      d->rangedMissileGlyph = '*';
      d->rangedSndVol = SndVol::low;
    } break;

    case ItemType::missileWeapon: {
      resetData(d, ItemType::general);
      d->itemWeight = itemWeight_extraLight;
      d->isStackable = true;
      d->isMissileWeapon = true;
      d->spawnStandardMaxDLVL = FIRST_CAVERN_LEVEL - 1;
      d->rangedSndVol = SndVol::low;
    } break;

    case ItemType::ammo: {
      resetData(d, ItemType::general);
      d->itemWeight = itemWeight_extraLight;
      d->glyph = '{';
      d->clr = clrWhite;
      d->tile = tile_ammo;
      d->isAmmo = true;
      d->spawnStandardMaxDLVL = FIRST_CAVERN_LEVEL - 1;
    } break;

    case ItemType::ammoClip: {
      resetData(d, ItemType::ammo);
      d->itemWeight = itemWeight_light;
      d->isStackable = false;
      d->isAmmoClip = true;
      d->spawnStandardMaxDLVL = FIRST_CAVERN_LEVEL - 1;
    } break;

    case ItemType::scroll: {
      resetData(d, ItemType::general);
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

    case ItemType::potion: {
      resetData(d, ItemType::general);
      d->itemValue = ItemValue::minorTreasure;
      d->chanceToIncludeInSpawnList = 55;
      d->itemWeight = itemWeight_light;
      d->isIdentified = false;
      d->glyph = '!';
      d->tile = tile_potion;
      d->isPotion = true;
      d->isMissileWeapon = true;
      d->missileHitChanceMod = 15;
      d->missileDmg = DiceParam(1, 3, 0);
      d->maxStackSizeAtSpawn = 2;
      d->landOnHardSurfaceSoundMsg = "";
      addFeatureFoundIn(d, feature_chest);
      addFeatureFoundIn(d, feature_tomb);
      addFeatureFoundIn(d, feature_cabinet, 25);
      addFeatureFoundIn(d, feature_cocoon, 25);
      eng.potionNameHandler->setClrAndFalseName(d);
    } break;

    case ItemType::device: {
      resetData(d, ItemType::general);
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

    case ItemType::armor: {
      resetData(d, ItemType::general);
      d->itemWeight = itemWeight_heavy;
      d->glyph = '[';
      d->tile = tile_armor;
      d->isArmor = true;
      d->isStackable = false;
    } break;

    case ItemType::explosive: {
      resetData(d, ItemType::general);
      d->itemWeight = itemWeight_light;
      d->isExplosive = true;
      d->glyph = '-';
      d->maxStackSizeAtSpawn = 2;
      d->landOnHardSurfaceSoundMsg = "";
    } break;

    default: {} break;
  }
}

void ItemDataHandler::addFeatureFoundIn(
  ItemData* const itemData, const FeatureId featureId,
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
  resetData(d, ItemType::general);
  d->name = ItemName("Shining Trapezohedron", "Shining Trapezohedrons",
                     "The Shining Trapezohedron");
  d->spawnStandardMinDLVL = -1;
  d->spawnStandardMaxDLVL = -1;
  d->isStackable = false;
  d->glyph = '*';
  d->clr = clrRedLgt;
  d->tile = tile_trapezohedron;
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::sawedOff);
  resetData(d, ItemType::rangedWpn);
  d->name = ItemName("Sawed-off Shotgun", "Sawed-off shotguns",
                     "a Sawed-off Shotgun");
  d->itemWeight = itemWeight_medium;
  d->tile = tile_shotgun;
  d->isShotgun = true;
  d->meleeAttMsgs = ItemAttMsgs("strike", "strikes me with a shotgun");
  d->rangedDmg = DiceParam(8, 3);
  d->rangedAmmoTypeUsed = ItemId::shotgunShell;
  d->rangedAttMsgs = ItemAttMsgs("fire", "fires a shotgun");
  d->rangedSndMsg = "I hear a shotgun blast.";
  d->rangedAttackSfx = SfxId::shotgunSawedOffFire;
  d->rangedMakesRicochetSound = true;
  d->reloadSfx = SfxId::shotgunReload;
  addFeatureFoundIn(d, feature_chest);
  addFeatureFoundIn(d, feature_cabinet);
  addFeatureFoundIn(d, feature_cocoon);
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::pumpShotgun);
  resetData(d, ItemType::rangedWpn);
  d->name = ItemName("Pump Shotgun", "Pump shotguns", "a Pump Shotgun");
  d->itemWeight = itemWeight_medium;
  d->tile = tile_shotgun;
  d->isShotgun = true;
  d->meleeAttMsgs = ItemAttMsgs("strike", "strikes me with a shotgun");
  d->rangedDmg = DiceParam(6, 3);
  d->rangedAmmoTypeUsed = ItemId::shotgunShell;
  d->rangedAttMsgs = ItemAttMsgs("fire", "fires a shotgun");
  d->rangedSndMsg = "I hear a shotgun blast.";
  d->rangedAttackSfx = SfxId::shotgunPumpFire ;
  d->rangedMakesRicochetSound = true;
  d->reloadSfx = SfxId::shotgunReload;
  addFeatureFoundIn(d, feature_chest);
  addFeatureFoundIn(d, feature_cabinet);
  addFeatureFoundIn(d, feature_cocoon);
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::shotgunShell);
  resetData(d, ItemType::ammo);
  d->name = ItemName("Shotgun shell", "Shotgun shells", "a shotgun shell");
  d->maxStackSizeAtSpawn = 10;
  addFeatureFoundIn(d, feature_chest);
  addFeatureFoundIn(d, feature_cabinet);
  addFeatureFoundIn(d, feature_cocoon);
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::incinerator);
  resetData(d, ItemType::rangedWpn);
  d->name = ItemName("Incinerator", "Incinerators", "an Incinerator");
  d->itemWeight = itemWeight_heavy;
  d->tile = tile_incinerator;
  d->meleeAttMsgs = ItemAttMsgs("strike", "strikes me with an Incinerator");
  d->rangedDmg = DiceParam(1, 3);
  d->rangedDmgInfoOverride = "*";
  d->rangedAmmoTypeUsed = ItemId::napalmCartridge;
  d->rangedAttMsgs = ItemAttMsgs("fire", "fires an incinerator");
  d->rangedSndMsg = "I hear the blast of a launched missile.";
  d->rangedMissileGlyph = '*';
  d->rangedMissileClr = clrRedLgt;
  d->spawnStandardMinDLVL = 5;
  addFeatureFoundIn(d, feature_chest, 25);
  addFeatureFoundIn(d, feature_cabinet, 25);
  addFeatureFoundIn(d, feature_cocoon, 25);
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::napalmCartridge);
  resetData(d, ItemType::ammoClip);
  d->name = ItemName("Napalm Cartridge", "Napalm Cartridges",
                     "a Napalm Cartridge");
  d->itemWeight = itemWeight_light;
  d->ammoContainedInClip = 3;
  d->spawnStandardMinDLVL = 5;
  d->maxStackSizeAtSpawn = 1;
  addFeatureFoundIn(d, feature_chest, 25);
  addFeatureFoundIn(d, feature_cabinet, 25);
  addFeatureFoundIn(d, feature_cocoon, 25);
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::machineGun);
  resetData(d, ItemType::rangedWpn);
  d->name = ItemName("Tommy Gun", "Tommy Guns", "a Tommy Gun");
  d->itemWeight = itemWeight_medium;
  d->tile = tile_tommyGun;
  d->meleeAttMsgs = ItemAttMsgs("strike", "strikes me with a Tommy Gun");
  d->isMachineGun = true;
  d->rangedDmg = DiceParam(2, 2, 2);
  d->rangedHitChanceMod = -10;
  d->rangedAmmoTypeUsed = ItemId::drumOfBullets;
  d->rangedAttMsgs = ItemAttMsgs("fire", "fires a Tommy Gun");
  d->rangedSndMsg = "I hear the burst of a machine gun.";
  d->rangedAttackSfx = SfxId::machineGunFire;
  d->rangedMakesRicochetSound = true;
  d->reloadSfx = SfxId::machineGunReload;
  addFeatureFoundIn(d, feature_chest);
  addFeatureFoundIn(d, feature_cabinet);
  addFeatureFoundIn(d, feature_cocoon);
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::drumOfBullets);
  resetData(d, ItemType::ammoClip);
  d->name = ItemName("Drum of .45 ACP", "Drums of .45 ACP",
                     "a Drum of .45 ACP");
  d->ammoContainedInClip = 50;
  addFeatureFoundIn(d, feature_chest);
  addFeatureFoundIn(d, feature_cabinet);
  addFeatureFoundIn(d, feature_cocoon);
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::pistol);
  resetData(d, ItemType::rangedWpn);
  d->name = ItemName("M1911 Colt", "M1911 Colt", "an M1911 Colt");
  d->itemWeight = itemWeight_light;
  d->tile = tile_pistol;
  d->rangedDmg = DiceParam(1, 8, 4);
  d->rangedAmmoTypeUsed = ItemId::pistolClip;
  d->meleeAttMsgs = ItemAttMsgs("strike", "strikes me with a pistol");
  d->rangedAttMsgs = ItemAttMsgs("fire", "fires a pistol");
  d->rangedSndMsg = "I hear a pistol being fired.";
  d->rangedAttackSfx = SfxId::pistolFire;
  d->rangedMakesRicochetSound = true;
  d->reloadSfx = SfxId::pistolReload;
  addFeatureFoundIn(d, feature_chest);
  addFeatureFoundIn(d, feature_cabinet);
  addFeatureFoundIn(d, feature_cocoon);
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::flareGun);
  resetData(d, ItemType::rangedWpn);
  d->name = ItemName("Flare Gun", "Flare Gun", "a Flare Gun");
  d->itemWeight = itemWeight_light;
  d->tile = tile_flareGun;
  d->rangedDmg = DiceParam(1, 3, 0);
  d->rangedDmgInfoOverride = "*";
  d->rangedAmmoTypeUsed = ItemId::flare;
  d->meleeAttMsgs = ItemAttMsgs("strike", "strikes me with a flare gun");
  d->rangedAttMsgs = ItemAttMsgs("fire", "fires a flare gun");
  d->rangedSndMsg = "I hear a flare gun being fired.";
  d->propAppliedOnRanged = new PropFlared(eng, propTurnsStd);
  addFeatureFoundIn(d, feature_chest);
  addFeatureFoundIn(d, feature_cabinet);
  addFeatureFoundIn(d, feature_cocoon);
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::pistolClip);
  resetData(d, ItemType::ammoClip);
  d->name = ItemName(".45ACP Colt cartridge", ".45ACP Colt cartridges",
                     "a .45ACP Colt cartridge");
  d->ammoContainedInClip = 7;
  addFeatureFoundIn(d, feature_chest);
  addFeatureFoundIn(d, feature_cabinet);
  addFeatureFoundIn(d, feature_cocoon);
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::teslaCannon);
  resetData(d, ItemType::rangedWpn);
  d->name = ItemName("Tesla Cannon", "Tesla Cannons", "a Tesla Cannon");
  d->itemWeight = itemWeight_medium;
  d->tile = tile_teslaCannon;
  d->meleeAttMsgs = ItemAttMsgs("strike", "strikes me with a Tesla Cannon");
  d->isMachineGun = true;
  d->rangedHitChanceMod = -15;
  d->rangedDmg = DiceParam(2, 3, 3);
  d->rangedDmgType = DmgType::electric;
  d->rangedAmmoTypeUsed = ItemId::teslaCanister;
  d->rangedAttMsgs = ItemAttMsgs("fire", "fires a Tesla Cannon");
  d->rangedSndMsg = "I hear loud electric crackle.";
  d->rangedMissileGlyph = '*';
  d->rangedMissileClr = clrYellow;
  d->spawnStandardMinDLVL = 7;
  addFeatureFoundIn(d, feature_chest, 50);
  addFeatureFoundIn(d, feature_cabinet, 50);
  addFeatureFoundIn(d, feature_cocoon, 50);
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::teslaCanister);
  resetData(d, ItemType::ammoClip);
  d->name = ItemName("Nuclear battery", "Nuclear batteries",
                     "a Nuclear battery");
  d->ammoContainedInClip = 30;
  d->spawnStandardMinDLVL = 6;
  addFeatureFoundIn(d, feature_chest, 50);
  addFeatureFoundIn(d, feature_cabinet, 50);
  addFeatureFoundIn(d, feature_cocoon, 50);
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::spikeGun);
  resetData(d, ItemType::rangedWpn);
  d->name = ItemName("Spike Gun", "Spike Guns", "a Spike Gun");
  d->itemWeight = itemWeight_medium;
  d->tile = tile_tommyGun;
  d->clr = clrBlueLgt;
  d->meleeAttMsgs = ItemAttMsgs("strike", "strikes me with a Spike Gun");
  d->isMachineGun = false;
  d->rangedHitChanceMod = 0;
  d->rangedDmg = DiceParam(1, 7, 0);
  d->rangedDmgType = DmgType::physical;
  d->rangedCausesKnockBack = true;
  d->rangedAmmoTypeUsed = ItemId::ironSpike;
  d->rangedAttMsgs = ItemAttMsgs("fire", "fires a Spike Gun");
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
  resetData(d, ItemType::explosive);
  d->name = ItemName("Dynamite", "Sticks of Dynamite", "a Stick of Dynamite");
  d->itemWeight = itemWeight_light;
  d->tile = tile_dynamite;
  d->clr = clrRedLgt;
  addFeatureFoundIn(d, feature_chest);
  addFeatureFoundIn(d, feature_cabinet);
  addFeatureFoundIn(d, feature_cocoon);
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::flare);
  resetData(d, ItemType::explosive);
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
  resetData(d, ItemType::explosive);
  d->name = ItemName("Molotov Cocktail", "Molotov Cocktails",
                     "a Molotov Cocktail");
  d->itemWeight = itemWeight_light;
  d->tile = tile_molotov;
  d->clr = clrWhite;
  addFeatureFoundIn(d, feature_chest);
  addFeatureFoundIn(d, feature_cabinet);
  addFeatureFoundIn(d, feature_cocoon);
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::throwingKnife);
  resetData(d, ItemType::missileWeapon);
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
  d->primaryAttackMode = PrimaryAttMode::missile;
  addFeatureFoundIn(d, feature_chest);
  addFeatureFoundIn(d, feature_cabinet);
  addFeatureFoundIn(d, feature_cocoon);
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::rock);
  resetData(d, ItemType::missileWeapon);
  d->name = ItemName("Rock", "Rocks", "a Rock");
  d->itemWeight = itemWeight_extraLight;
  d->tile = tile_rock;
  d->glyph = '*';
  d->clr = clrGray;
  d->missileHitChanceMod = 10;
  d->missileDmg = DiceParam(1, 3);
  d->maxStackSizeAtSpawn = 6;
  d->primaryAttackMode = PrimaryAttMode::missile;
  addFeatureFoundIn(d, feature_cabinet);
  addFeatureFoundIn(d, feature_cocoon);
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::dagger);
  resetData(d, ItemType::meleeWpn);
  d->name = ItemName("Dagger", "Daggers", "a Dagger");
  d->itemWeight = itemWeight_light;
  d->tile = tile_dagger;
  d->meleeAttMsgs = ItemAttMsgs("strike", "strikes me with a Dagger");
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
  resetData(d, ItemType::meleeWpn);
  d->name = ItemName("Hatchet", "Hatchets", "a Hatchet");
  d->itemWeight = itemWeight_light;
  d->tile = tile_axe;
  d->meleeAttMsgs = ItemAttMsgs("strike", "strikes me with a Hatchet");
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
  resetData(d, ItemType::meleeWpn);
  d->name = ItemName("Club", "Clubs", "a Club");
  d->spawnStandardMinDLVL = FIRST_CAVERN_LEVEL;
  d->spawnStandardMaxDLVL = 999;
  d->itemWeight = itemWeight_medium;
  d->tile = tile_club;
  d->clr = clrBrown;
  d->meleeAttMsgs = ItemAttMsgs("strike", "strikes me with a Club");
  d->meleeDmg = pair<int, int>(2, 3);
  d->meleeHitChanceMod = 10;
  d->meleeMissSfx = SfxId::missMedium;
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::hammer);
  resetData(d, ItemType::meleeWpn);
  d->name = ItemName("Hammer", "Hammers", "a Hammer");
  d->itemWeight = itemWeight_medium;
  d->tile = tile_hammer;
  d->meleeAttMsgs = ItemAttMsgs("strike", "strikes me with a Hammer");
  d->meleeDmg = pair<int, int>(2, 4);
  d->meleeHitChanceMod = 5;
  d->meleeMissSfx = SfxId::missMedium;
  addFeatureFoundIn(d, feature_cabinet);
  addFeatureFoundIn(d, feature_cocoon);
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::machete);
  resetData(d, ItemType::meleeWpn);
  d->name = ItemName("Machete", "Machetes", "a Machete");
  d->itemWeight = itemWeight_medium;
  d->tile = tile_machete;
  d->meleeAttMsgs = ItemAttMsgs("strike", "strikes me with a Machete");
  d->meleeDmg = pair<int, int>(2, 5);
  d->meleeHitChanceMod = 0;
  d->meleeHitSmallSfx = SfxId::hitSharp;
  d->meleeHitMediumSfx = SfxId::hitSharp;
  d->meleeMissSfx = SfxId::missMedium;
  addFeatureFoundIn(d, feature_cabinet);
  addFeatureFoundIn(d, feature_cocoon);
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::axe);
  resetData(d, ItemType::meleeWpn);
  d->name = ItemName("Axe", "Axes", "an Axe");
  d->itemWeight = itemWeight_medium;
  d->tile = tile_axe;
  d->meleeAttMsgs = ItemAttMsgs("strike", "strikes me with an axe");
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
  resetData(d, ItemType::meleeWpn);
  d->name = ItemName("Pitchfork", "Pitchforks", "a Pitchfork");
  d->itemWeight = itemWeight_heavy;
  d->tile = tile_pitchfork;
  d->meleeAttMsgs = ItemAttMsgs("strike", "strikes me with a Pitchfork");
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
  resetData(d, ItemType::meleeWpn);
  d->name = ItemName("Sledgehammer", "Sledgehammers", "a Sledgehammer");
  d->itemWeight = itemWeight_heavy;
  d->tile = tile_sledgeHammer;
  d->meleeAttMsgs = ItemAttMsgs("strike", "strikes me with a Sledgehammer");
  d->meleeDmg = pair<int, int>(3, 5);
  d->meleeHitChanceMod = -10;
  d->meleeCausesKnockBack = true;
  d->meleeMissSfx = SfxId::missHeavy;
  addFeatureFoundIn(d, feature_cabinet);
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::ironSpike);
  resetData(d, ItemType::missileWeapon);
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
  d->primaryAttackMode = PrimaryAttMode::missile;
  addFeatureFoundIn(d, feature_cabinet);
  addFeatureFoundIn(d, feature_cocoon);
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::playerKick);
  resetData(d, ItemType::meleeWpnIntr);
  d->meleeAttMsgs = ItemAttMsgs("kick", "");
  d->meleeHitChanceMod = 20;
  d->meleeDmg = pair<int, int>(1, 3);
  d->meleeCausesKnockBack = true;
  d->meleeMissSfx = SfxId::missMedium;
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::playerStomp);
  resetData(d, ItemType::meleeWpnIntr);
  d->meleeAttMsgs = ItemAttMsgs("stomp", "");
  d->meleeHitChanceMod = 20;
  d->meleeDmg = pair<int, int>(1, 3);
  d->meleeCausesKnockBack = false;
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::playerPunch);
  resetData(d, ItemType::meleeWpnIntr);
  d->meleeAttMsgs = ItemAttMsgs("punch", "");
  d->meleeHitChanceMod = 25;
  d->meleeDmg = pair<int, int>(1, 2);
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::zombieClaw);
  resetData(d, ItemType::meleeWpnIntr);
  d->meleeAttMsgs = ItemAttMsgs("", "claws me");
  setDmgFromMonsterData(*d, ActorData::dataList[actor_zombie]);
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::zombieClawDiseased);
  resetData(d, ItemType::meleeWpnIntr);
  d->meleeAttMsgs = ItemAttMsgs("", "claws me");
  setDmgFromMonsterData(*d, ActorData::dataList[actor_zombie]);
  d->propAppliedOnMelee = new PropInfected(eng, propTurnsStd);
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::zombieAxe);
  resetData(d, ItemType::meleeWpnIntr);
  d->meleeAttMsgs = ItemAttMsgs("", "chops me with a rusty axe");
  d->meleeHitSmallSfx = SfxId::hitSmall;
  d->meleeHitMediumSfx = SfxId::hitMedium;
  d->meleeHitHardSfx = SfxId::hitHard;
  setDmgFromMonsterData(*d, ActorData::dataList[actor_zombieAxe]);
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::bloatedZombiePunch);
  resetData(d, ItemType::meleeWpnIntr);
  d->meleeAttMsgs = ItemAttMsgs("", "mauls me");
  setDmgFromMonsterData(*d, ActorData::dataList[actor_bloatedZombie]);
  d->meleeCausesKnockBack = true;
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::bloatedZombieSpit);
  resetData(d, ItemType::rangedWpnIntr);
  d->rangedAttMsgs = ItemAttMsgs("", "spits acid pus at me");
  setDmgFromMonsterData(*d, ActorData::dataList[actor_bloatedZombie]);
  d->rangedSndMsg = "I hear spitting.";
  d->rangedMissileClr = clrGreenLgt;
  d->rangedDmgType = DmgType::acid;
  d->rangedMissileGlyph = '*';
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::ratBite);
  resetData(d, ItemType::meleeWpnIntr);
  d->meleeAttMsgs = ItemAttMsgs("", "bites me");
  setDmgFromMonsterData(*d, ActorData::dataList[actor_rat]);
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::ratBiteDiseased);
  resetData(d, ItemType::meleeWpnIntr);
  d->meleeAttMsgs = ItemAttMsgs("", "bites me");
  setDmgFromMonsterData(*d, ActorData::dataList[actor_rat]);
  d->propAppliedOnMelee = new PropInfected(eng, propTurnsStd);
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::ratThingBite);
  resetData(d, ItemType::meleeWpnIntr);
  d->meleeAttMsgs = ItemAttMsgs("", "bites me");
  setDmgFromMonsterData(*d, ActorData::dataList[actor_ratThing]);
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::wormMassBite);
  resetData(d, ItemType::meleeWpnIntr);
  d->meleeAttMsgs = ItemAttMsgs("", "bites me");
  setDmgFromMonsterData(*d, ActorData::dataList[actor_wormMass]);
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::wolfBite);
  resetData(d, ItemType::meleeWpnIntr);
  d->meleeAttMsgs = ItemAttMsgs("", "bites me");
  setDmgFromMonsterData(*d, ActorData::dataList[actor_wolf]);
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::greenSpiderBite);
  resetData(d, ItemType::meleeWpnIntr);
  d->meleeAttMsgs = ItemAttMsgs("", "bites me");
  setDmgFromMonsterData(*d, ActorData::dataList[actor_greenSpider]);
  d->propAppliedOnMelee = new PropBlind(eng, propTurnsSpecific, 4);
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::whiteSpiderBite);
  resetData(d, ItemType::meleeWpnIntr);
  d->meleeAttMsgs = ItemAttMsgs("", "bites me");
  setDmgFromMonsterData(*d, ActorData::dataList[actor_whiteSpider]);
  d->propAppliedOnMelee = new PropParalyzed(eng, propTurnsSpecific, 2);
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::redSpiderBite);
  resetData(d, ItemType::meleeWpnIntr);
  d->meleeAttMsgs = ItemAttMsgs("", "bites me");
  setDmgFromMonsterData(*d, ActorData::dataList[actor_redSpider]);
  d->propAppliedOnMelee = new PropWeakened(eng, propTurnsStd);
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::shadowSpiderBite);
  resetData(d, ItemType::meleeWpnIntr);
  d->meleeAttMsgs = ItemAttMsgs("", "bites me");
  setDmgFromMonsterData(*d, ActorData::dataList[actor_shadowSpider]);
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::lengSpiderBite);
  resetData(d, ItemType::meleeWpnIntr);
  d->meleeAttMsgs = ItemAttMsgs("", "bites me");
  setDmgFromMonsterData(
    *d, ActorData::dataList[actor_lengSpider]);
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::fireHoundBreath);
  resetData(d, ItemType::rangedWpnIntr);
  d->rangedAttMsgs = ItemAttMsgs("", "breaths fire at me");
  d->rangedSndMsg = "I hear a burst of flames.";
  setDmgFromMonsterData(*d, ActorData::dataList[actor_fireHound]);
  d->propAppliedOnRanged = new PropBurning(eng, propTurnsStd);
  d->rangedMissileClr = clrRedLgt;
  d->rangedMissileGlyph = '*';
  d->rangedMissileLeavesTrail = true;
  d->rangedMissileLeavesSmoke = true;
  d->rangedDmgType = DmgType::fire;
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::fireHoundBite);
  resetData(d, ItemType::meleeWpnIntr);
  d->meleeAttMsgs = ItemAttMsgs("", "bites me");
  setDmgFromMonsterData(*d, ActorData::dataList[actor_fireHound]);
  d->meleeDmgType = DmgType::fire;
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::frostHoundBreath);
  resetData(d, ItemType::rangedWpnIntr);
  d->rangedAttMsgs = ItemAttMsgs("", "breaths frost at me");
  d->rangedSndMsg = "I hear a chilling sound.";
  setDmgFromMonsterData(*d, ActorData::dataList[actor_frostHound]);
  d->rangedMissileClr = clrBlueLgt;
  d->rangedMissileGlyph = '*';
  d->rangedMissileLeavesTrail = true;
  d->rangedMissileLeavesSmoke = true;
  d->rangedDmgType = DmgType::cold;
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::frostHoundBite);
  resetData(d, ItemType::meleeWpnIntr);
  d->meleeAttMsgs = ItemAttMsgs("", "bites me");
  setDmgFromMonsterData(*d, ActorData::dataList[actor_frostHound]);
  d->meleeDmgType = DmgType::cold;
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::zuulBite);
  resetData(d, ItemType::meleeWpnIntr);
  d->meleeAttMsgs = ItemAttMsgs("", "bites me");
  setDmgFromMonsterData(*d, ActorData::dataList[actor_zuul]);
  d->meleeDmgType = DmgType::physical;
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::dustVortexEngulf);
  resetData(d, ItemType::meleeWpnIntr);
  d->meleeAttMsgs = ItemAttMsgs("", "engulfs me");
  setDmgFromMonsterData(*d, ActorData::dataList[actor_dustVortex]);
  d->propAppliedOnMelee = new PropBlind(eng, propTurnsStd);
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::fireVortexEngulf);
  resetData(d, ItemType::meleeWpnIntr);
  d->meleeAttMsgs = ItemAttMsgs("", "engulfs me");
  setDmgFromMonsterData(
    *d, ActorData::dataList[actor_fireVortex]);
  d->propAppliedOnMelee = new PropBurning(eng, propTurnsStd);
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::frostVortexEngulf);
  resetData(d, ItemType::meleeWpnIntr);
  d->meleeAttMsgs = ItemAttMsgs("", "engulfs me");
  setDmgFromMonsterData(*d, ActorData::dataList[actor_frostVortex]);
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::ghostClaw);
  resetData(d, ItemType::meleeWpnIntr);
  d->meleeAttMsgs = ItemAttMsgs("", "claws me");
  setDmgFromMonsterData(*d, ActorData::dataList[actor_ghost]);
  d->propAppliedOnMelee = new PropTerrified(eng, propTurnsSpecific, 4);
  d->meleeDmgType = DmgType::spirit;
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::phantasmSickle);
  resetData(d, ItemType::meleeWpnIntr);
  d->meleeAttMsgs = ItemAttMsgs("", "slices me with a sickle");
  setDmgFromMonsterData(*d, ActorData::dataList[actor_phantasm]);
  d->propAppliedOnMelee = new PropTerrified(eng, propTurnsSpecific, 4);
  d->meleeDmgType = DmgType::spirit;
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::wraithClaw);
  resetData(d, ItemType::meleeWpnIntr);
  d->meleeAttMsgs = ItemAttMsgs("", "claws me");
  setDmgFromMonsterData(*d, ActorData::dataList[actor_wraith]);
  d->propAppliedOnMelee = new PropTerrified(eng, propTurnsSpecific, 4);
  d->meleeDmgType = DmgType::spirit;
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::giantBatBite);
  resetData(d, ItemType::meleeWpnIntr);
  d->meleeAttMsgs = ItemAttMsgs("", "bites me");
  setDmgFromMonsterData(*d, ActorData::dataList[actor_giantBat]);
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::miGoElectricGun);
  resetData(d, ItemType::rangedWpnIntr);
  d->rangedMissileLeavesTrail = true;
  d->rangedMissileClr = clrYellow;
  d->rangedMissileGlyph = '/';
  d->rangedAttMsgs = ItemAttMsgs("", "fires an electric gun");
  d->rangedDmgType = DmgType::electric;
  d->propAppliedOnRanged = new PropParalyzed(eng, propTurnsSpecific, 2);
  d->rangedSndMsg = "I hear a bolt of electricity.";
  setDmgFromMonsterData(*d, ActorData::dataList[actor_miGo]);
  d->rangedSndVol = SndVol::high;
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::polypTentacle);
  resetData(d, ItemType::meleeWpnIntr);
  d->meleeAttMsgs = ItemAttMsgs("", "grips me with a tentacle");
  d->propAppliedOnMelee = new PropParalyzed(eng, propTurnsSpecific, 1);
  setDmgFromMonsterData(*d, ActorData::dataList[actor_flyingPolyp]);
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::ghoulClaw);
  resetData(d, ItemType::meleeWpnIntr);
  d->meleeAttMsgs = ItemAttMsgs("", "claws me");
  setDmgFromMonsterData(*d, ActorData::dataList[actor_ghoul]);
  d->propAppliedOnMelee = new PropInfected(eng, propTurnsStd);
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::shadowClaw);
  resetData(d, ItemType::meleeWpnIntr);
  d->meleeAttMsgs = ItemAttMsgs("", "claws me");
  setDmgFromMonsterData(*d, ActorData::dataList[actor_shadow]);
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::byakheeClaw);
  resetData(d, ItemType::meleeWpnIntr);
  d->meleeAttMsgs = ItemAttMsgs("", "claws me");
  setDmgFromMonsterData(*d, ActorData::dataList[actor_byakhee]);
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::giantMantisClaw);
  resetData(d, ItemType::meleeWpnIntr);
  d->meleeAttMsgs = ItemAttMsgs("", "claws me");
  setDmgFromMonsterData(*d, ActorData::dataList[actor_giantMantis]);
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::giantLocustBite);
  resetData(d, ItemType::meleeWpnIntr);
  d->meleeAttMsgs = ItemAttMsgs("", "bites me");
  setDmgFromMonsterData(*d, ActorData::dataList[actor_giantLocust]);
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::mummyMaul);
  resetData(d, ItemType::meleeWpnIntr);
  d->meleeAttMsgs = ItemAttMsgs("", "mauls me");
  setDmgFromMonsterData(*d, ActorData::dataList[actor_mummy]);
  d->propAppliedOnMelee = new PropCursed(eng, propTurnsStd);
  d->meleeCausesKnockBack = true;
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::deepOneJavelinAtt);
  resetData(d, ItemType::rangedWpnIntr);
  d->rangedAttMsgs = ItemAttMsgs("", "throws a Javelin at me");
  setDmgFromMonsterData(*d, ActorData::dataList[actor_deepOne]);
  d->rangedSndMsg = "";
  d->rangedMissileClr = clrBrown;
  d->rangedMissileGlyph = '/';
  d->rangedSndVol = SndVol::low;
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::deepOneSpearAtt);
  resetData(d, ItemType::meleeWpnIntr);
  d->meleeAttMsgs = ItemAttMsgs("", "thrusts a spear at me");
  setDmgFromMonsterData(*d, ActorData::dataList[actor_deepOne]);
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::oozeBlackSpewPus);
  resetData(d, ItemType::meleeWpnIntr);
  d->meleeAttMsgs = ItemAttMsgs("", "spews pus on me");
  setDmgFromMonsterData(*d, ActorData::dataList[actor_oozeBlack]);
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::oozeClearSpewPus);
  resetData(d, ItemType::meleeWpnIntr);
  d->meleeAttMsgs = ItemAttMsgs("", "spews pus on me");
  setDmgFromMonsterData(*d, ActorData::dataList[actor_oozeClear]);
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::oozePutridSpewPus);
  resetData(d, ItemType::meleeWpnIntr);
  d->meleeAttMsgs = ItemAttMsgs("", "spews infected pus on me");
  setDmgFromMonsterData(*d, ActorData::dataList[actor_oozePutrid]);
  d->propAppliedOnMelee = new PropInfected(eng, propTurnsStd);
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::oozePoisonSpewPus);
  resetData(d, ItemType::meleeWpnIntr);
  d->meleeAttMsgs = ItemAttMsgs("", "spews poisonous pus on me");
  setDmgFromMonsterData(*d, ActorData::dataList[actor_oozePoison]);
  d->propAppliedOnMelee = new PropPoisoned(eng, propTurnsStd);
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::colourOOSpaceTouch);
  resetData(d, ItemType::meleeWpnIntr);
  d->meleeAttMsgs = ItemAttMsgs("", "touches me");
  setDmgFromMonsterData(*d, ActorData::dataList[actor_colourOOSpace]);
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::chthonianBite);
  resetData(d, ItemType::meleeWpnIntr);
  d->meleeAttMsgs = ItemAttMsgs("", "strikes me with a tentacle");
  d->meleeCausesKnockBack = true;
  setDmgFromMonsterData(*d, ActorData::dataList[actor_chthonian]);
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::huntingHorrorBite);
  resetData(d, ItemType::meleeWpnIntr);
  d->meleeAttMsgs = ItemAttMsgs("", "bites me");
  setDmgFromMonsterData(*d, ActorData::dataList[actor_huntingHorror]);
  d->propAppliedOnMelee = new PropParalyzed(eng, propTurnsStd);
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::armorLeatherJacket);
  resetData(d, ItemType::armor);
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
  resetData(d, ItemType::armor);
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
  resetData(d, ItemType::armor);
  d->name = ItemName("Flak Jacket", "", "a Flak Jacket");
  d->itemWeight = itemWeight_medium;
  d->clr = clrGreen;
  d->spawnStandardMinDLVL = 3;
  d->armorData.absorptionPoints = 3;
  d->armorData.dmgToDurabilityFactor = 0.5;
  d->landOnHardSurfaceSoundMsg = "I hear a thudding sound.";
  addFeatureFoundIn(d, feature_cabinet);
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::armorAsbSuit);
  resetData(d, ItemType::armor);
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
  resetData(d, ItemType::armor);
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
  resetData(d, ItemType::scroll);
  d->spellCastFromScroll = SpellId::mayhem;
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::scrollOfTelep);
  resetData(d, ItemType::scroll);
  d->spellCastFromScroll = SpellId::teleport;
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::scrollOfPestilence);
  resetData(d, ItemType::scroll);
  d->spellCastFromScroll = SpellId::pestilence;
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::scrollOfSlowMon);
  resetData(d, ItemType::scroll);
  d->spellCastFromScroll = SpellId::slowEnemies;
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::scrollOfTerrifyMon);
  resetData(d, ItemType::scroll);
  d->spellCastFromScroll = SpellId::terrifyEnemies;
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::scrollOfParalMon);
  resetData(d, ItemType::scroll);
  d->spellCastFromScroll = SpellId::paralyzeEnemies;
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::scrollOfDetItems);
  resetData(d, ItemType::scroll);
  d->spellCastFromScroll = SpellId::detItems;
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::scrollOfDetTraps);
  resetData(d, ItemType::scroll);
  d->spellCastFromScroll = SpellId::detTraps;
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::scrollOfBless);
  resetData(d, ItemType::scroll);
  d->spellCastFromScroll = SpellId::bless;
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::scrollOfDarkbolt);
  resetData(d, ItemType::scroll);
  d->spellCastFromScroll = SpellId::darkbolt;
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::scrollOfAzaWrath);
  resetData(d, ItemType::scroll);
  d->spellCastFromScroll = SpellId::azathothsWrath;
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::scrollOfOpening);
  resetData(d, ItemType::scroll);
  d->spellCastFromScroll = SpellId::opening;
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::scrollOfSacrLife);
  resetData(d, ItemType::scroll);
  d->spellCastFromScroll = SpellId::sacrLife;
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::scrollOfSacrSpi);
  resetData(d, ItemType::scroll);
  d->spellCastFromScroll = SpellId::sacrSpi;
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::scrollOfDetMon);
  resetData(d, ItemType::scroll);
  d->spellCastFromScroll = SpellId::detMon;
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::scrollOfElemRes);
  resetData(d, ItemType::scroll);
  d->spellCastFromScroll = SpellId::elemRes;
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::potionOfVitality);
  resetData(d, ItemType::potion);
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::potionOfSpirit);
  resetData(d, ItemType::potion);
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::potionOfBlindness);
  resetData(d, ItemType::potion);
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::potionOfFrenzy);
  resetData(d, ItemType::potion);
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::potionOfFortitude);
  resetData(d, ItemType::potion);
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::potionOfParalyze);
  resetData(d, ItemType::potion);
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::potionOfRElec);
  resetData(d, ItemType::potion);
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::potionOfConf);
  resetData(d, ItemType::potion);
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::potionOfPoison);
  resetData(d, ItemType::potion);
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::potionOfInsight);
  resetData(d, ItemType::potion);
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::potionOfClairv);
  resetData(d, ItemType::potion);
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::potionOfRFire);
  resetData(d, ItemType::potion);
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::potionOfAntidote);
  resetData(d, ItemType::potion);
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::potionOfDescent);
  resetData(d, ItemType::potion);
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::deviceSentry);
  resetData(d, ItemType::device);
  d->name = ItemName("Sentry Device", "Sentry Devices", "a Sentry Device");
  d->itemValue = ItemValue::majorTreasure;
  d->isIdentified = false;
  d->clr = clrGray;
  addFeatureFoundIn(d, feature_chest, 10);
  addFeatureFoundIn(d, feature_tomb, 10);
  addFeatureFoundIn(d, feature_cocoon, 10);
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::deviceRepeller);
  resetData(d, ItemType::device);
  d->name = ItemName("Repeller Device", "Repeller Devices",
                     "a Repeller Device");
  d->itemValue = ItemValue::majorTreasure;
  d->isIdentified = false;
  d->clr = clrGray;
  addFeatureFoundIn(d, feature_chest, 10);
  addFeatureFoundIn(d, feature_tomb, 10);
  addFeatureFoundIn(d, feature_cocoon, 10);
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::deviceRejuvenator);
  resetData(d, ItemType::device);
  d->name = ItemName("Rejuvenator Device", "Rejuvenator Devices",
                     "a Rejuvenator Device");
  d->itemValue = ItemValue::majorTreasure;
  d->isIdentified = false;
  d->clr = clrGray;
  addFeatureFoundIn(d, feature_chest, 10);
  addFeatureFoundIn(d, feature_tomb, 10);
  addFeatureFoundIn(d, feature_cocoon, 10);
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::deviceTranslocator);
  resetData(d, ItemType::device);
  d->name = ItemName("Translocator Device", "Translocator Devices",
                     "a Translocator Device");
  d->itemValue = ItemValue::majorTreasure;
  d->isIdentified = false;
  d->clr = clrGray;
  addFeatureFoundIn(d, feature_chest, 10);
  addFeatureFoundIn(d, feature_tomb, 10);
  addFeatureFoundIn(d, feature_cocoon, 10);
  dataList[int(d->id)] = d;

  d = new ItemData(ItemId::electricLantern);
  resetData(d, ItemType::device);
  d->name = ItemName("Electric Lantern", "Electric Lanterns",
                     "an Electric Lantern");
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
  resetData(d, ItemType::general);
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

} //namespace

void init() {initDataList();}

void cleanup() {
  for(size_t i = 1; i < int(ItemId::endOfItemIds); i++) delete dataList[i];
}


void ItemDataHandler::storeToSaveLines(vector<string>& lines) const {
  for(int i = 1; i < int(ItemId::endOfItemIds); i++) {
    lines.push_back(dataList[i]->isIdentified ? "1" : "0");

    if(dataList[i]->isScroll) {
      lines.push_back(dataList[i]->isTried ? "1" : "0");
    }
  }
}

void ItemDataHandler::setupFromSaveLines(vector<string>& lines) {
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
  const PrimaryAttMode attackMode) const {
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
    Map::player->getData().abilityVals.getVal(
      AbilityId::accuracyRanged, true, *(Map::player));

  if(
    (attackMode == PrimaryAttMode::none &&
     d.primaryAttackMode == PrimaryAttMode::melee) ||
    (attackMode == PrimaryAttMode::melee && d.isMeleeWeapon)) {
    const string rollsStr = toStr(d.meleeDmg.first);
    const string sidesStr = toStr(d.meleeDmg.second);
    const int PLUS = dynamic_cast<const Weapon*>(&item)->meleeDmgPlus;
    const string plusStr = PLUS ==  0 ? "" : ((PLUS > 0 ? "+" : "") +
                           toStr(PLUS));
    const int ITEM_SKILL = d.meleeHitChanceMod;
    const int PLAYER_MELEE_SKILL =
      Map::player->getData().abilityVals.getVal(
        AbilityId::accuracyMelee, true, *(Map::player));
    const int TOTAL_SKILL = max(0, min(100, ITEM_SKILL + PLAYER_MELEE_SKILL));
    const string skillStr = toStr(TOTAL_SKILL) + "%";
    return ret + " " + rollsStr + "d" + sidesStr + plusStr + " " + skillStr;
  }

  if(
    (attackMode == PrimaryAttMode::none &&
     d.primaryAttackMode == PrimaryAttMode::ranged) ||
    (attackMode == PrimaryAttMode::ranged && d.isRangedWeapon)) {

    string dmgStr = d.rangedDmgInfoOverride;

    if(dmgStr.empty()) {
      const int MULTIPL = d.isMachineGun == true ?
                          NR_MG_PROJECTILES : 1;
      const string rollsStr = toStr(d.rangedDmg.rolls * MULTIPL);
      const string sidesStr = toStr(d.rangedDmg.sides);
      const int PLUS = d.rangedDmg.plus * MULTIPL;
      const string plusStr = PLUS ==  0 ? "" : ((PLUS > 0 ? "+" : "") +
                             toStr(PLUS));
      dmgStr = rollsStr + "d" + sidesStr + plusStr;
    }
    const int ITEM_SKILL = d.rangedHitChanceMod;
    const int TOTAL_SKILL = max(0, min(100, ITEM_SKILL + PLAYER_RANGED_SKILL));
    const string skillStr = toStr(TOTAL_SKILL) + "%";
    string ammoLoadedStr = "";
    if(d.rangedHasInfiniteAmmo == false) {
      const Weapon* const w = dynamic_cast<const Weapon*>(&item);
      ammoLoadedStr = " " + toStr(w->nrAmmoLoaded) + "/" +
                      toStr(w->ammoCapacity);
    }
    return ret + " " + dmgStr + " " + skillStr + ammoLoadedStr;
  }

  if(
    (attackMode == PrimaryAttMode::none &&
     d.primaryAttackMode == PrimaryAttMode::missile) ||
    (attackMode == PrimaryAttMode::missile && d.isMissileWeapon)) {
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

} //ItemData
