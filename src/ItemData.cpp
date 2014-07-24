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
#include "Map.h"

using namespace std;

namespace ItemData {

ItemDataT* data[int(ItemId::END)];

namespace {

void addFeatureFoundIn(ItemDataT* const itemData, const FeatureId featureId,
                       const int CHANCE_TO_INCLUDE = 100) {
  itemData->featuresCanBeFoundIn.push_back(
    pair<FeatureId, int>(featureId, CHANCE_TO_INCLUDE));
}

//------------------------------- ITEM ARCHETYPES (DEFAULTS)
void resetData(ItemDataT* const d, ItemType const itemType) {
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
      d->tile = TileId::empty;
      d->primaryAttackMode = PrimaryAttMode::none;
      d->isScroll = d->isPotion = d->isEatable = false;
      d->isArmor = d->isCloak = d->isRing = d->isAmulet = false;
      d->isIntrinsic = d->isMeleeWeapon = d->isRangedWeapon = false;
      d->isMissileWeapon = d->isShotgun = d->isMachineGun = false;
      d->isAmmo = d->isAmmoClip = d->isDevice = d->isMedicalBag = false;
      d->spellCastFromScroll = SpellId::END;
      d->ammoContainedInClip = 0;
      d->meleeHitChanceMod = 0;
      d->propAppliedOnMelee = nullptr;
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
      d->rangedMissileTile = TileId::projectileStandardFrontSlash;
      d->rangedMissileClr = clrWhite;
      d->rangedMissileLeavesTrail = false;
      d->rangedMissileLeavesSmoke = false;
      d->rangedSndMsg = "";
      d->rangedSndVol = SndVol::low;
      d->rangedMakesRicochetSound = false;
      d->landOnHardSurfaceSoundMsg = "I hear a thudding sound.";
      d->landOnHardSurfaceSfx = SfxId::END;
      d->rangedAttackSfx = SfxId::END;
      d->meleeHitSmallSfx = SfxId::END;
      d->meleeHitMediumSfx = SfxId::END;
      d->meleeHitHardSfx = SfxId::END;
      d->meleeMissSfx = SfxId::END;
      d->reloadSfx = SfxId::END;
      d->propAppliedOnRanged = nullptr;
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
      d->spawnStandardMaxDLVL = FIRST_CAVERN_LVL - 1;
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
      d->spawnStandardMaxDLVL = FIRST_CAVERN_LVL - 1;
      d->rangedSndVol = SndVol::low;
    } break;

    case ItemType::ammo: {
      resetData(d, ItemType::general);
      d->itemWeight = itemWeight_extraLight;
      d->glyph = '{';
      d->clr = clrWhite;
      d->tile = TileId::ammo;
      d->isAmmo = true;
      d->spawnStandardMaxDLVL = FIRST_CAVERN_LVL - 1;
    } break;

    case ItemType::ammoClip: {
      resetData(d, ItemType::ammo);
      d->itemWeight = itemWeight_light;
      d->isStackable = false;
      d->isAmmoClip = true;
      d->spawnStandardMaxDLVL = FIRST_CAVERN_LVL - 1;
    } break;

    case ItemType::scroll: {
      resetData(d, ItemType::general);
      d->itemValue = ItemValue::minorTreasure;
      d->chanceToIncludeInSpawnList = 40;
      d->itemWeight = itemWeight_none;
      d->isIdentified = false;
      d->glyph = '?';
      d->clr = clrWhite;
      d->tile = TileId::scroll;
      d->isScroll = true;
      d->maxStackSizeAtSpawn = 1;
      d->landOnHardSurfaceSoundMsg = "";
      addFeatureFoundIn(d, FeatureId::chest);
      addFeatureFoundIn(d, FeatureId::tomb);
      addFeatureFoundIn(d, FeatureId::cabinet, 25);
      addFeatureFoundIn(d, FeatureId::cocoon, 25);
      ScrollNameHandling::setFalseScrollName(*d);
    } break;

    case ItemType::potion: {
      resetData(d, ItemType::general);
      d->itemValue = ItemValue::minorTreasure;
      d->chanceToIncludeInSpawnList = 55;
      d->itemWeight = itemWeight_light;
      d->isIdentified = false;
      d->glyph = '!';
      d->tile = TileId::potion;
      d->isPotion = true;
      d->isMissileWeapon = true;
      d->missileHitChanceMod = 15;
      d->missileDmg = DiceParam(1, 3, 0);
      d->maxStackSizeAtSpawn = 2;
      d->landOnHardSurfaceSoundMsg = "";
      addFeatureFoundIn(d, FeatureId::chest);
      addFeatureFoundIn(d, FeatureId::tomb);
      addFeatureFoundIn(d, FeatureId::cabinet, 25);
      addFeatureFoundIn(d, FeatureId::cocoon, 25);
      PotionNameHandling::setClrAndFalseName(d);
    } break;

    case ItemType::device: {
      resetData(d, ItemType::general);
      d->isDevice = true;
      d->chanceToIncludeInSpawnList = 12;
      d->itemWeight = itemWeight_light;
      d->isIdentified = true;
      d->glyph = '~';
      d->tile = TileId::device1;
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
      d->tile = TileId::armor;
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

void setDmgFromMonsterData(ItemDataT& itemData, const ActorDataT& actorData)  {
  itemData.meleeDmg  = pair<int, int>(1, actorData.dmgMelee);
  itemData.rangedDmg = DiceParam(1, actorData.dmgRanged, 0);
}

//------------------------------- LIST OF ITEMS
void initDataList() {
  ItemDataT* d = nullptr;

  d = new ItemDataT(ItemId::trapezohedron);
  resetData(d, ItemType::general);
  d->baseName = ItemName("Shining Trapezohedron", "Shining Trapezohedrons",
                         "The Shining Trapezohedron");
  d->spawnStandardMinDLVL = -1;
  d->spawnStandardMaxDLVL = -1;
  d->isStackable = false;
  d->glyph = '*';
  d->clr = clrRedLgt;
  d->tile = TileId::trapezohedron;
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::sawedOff);
  resetData(d, ItemType::rangedWpn);
  d->baseName = ItemName("Sawed-off Shotgun", "Sawed-off shotguns",
                         "a Sawed-off Shotgun");
  d->itemWeight = itemWeight_medium;
  d->tile = TileId::shotgun;
  d->isShotgun = true;
  d->meleeAttMsgs = ItemAttMsgs("strike", "strikes me with a shotgun");
  d->rangedDmg = DiceParam(8, 3);
  d->rangedAmmoTypeUsed = ItemId::shotgunShell;
  d->rangedAttMsgs = ItemAttMsgs("fire", "fires a shotgun");
  d->rangedSndMsg = "I hear a shotgun blast.";
  d->rangedAttackSfx = SfxId::shotgunSawedOffFire;
  d->rangedMakesRicochetSound = true;
  d->reloadSfx = SfxId::shotgunReload;
  addFeatureFoundIn(d, FeatureId::chest);
  addFeatureFoundIn(d, FeatureId::cabinet);
  addFeatureFoundIn(d, FeatureId::cocoon);
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::pumpShotgun);
  resetData(d, ItemType::rangedWpn);
  d->baseName = ItemName("Pump Shotgun", "Pump shotguns", "a Pump Shotgun");
  d->itemWeight = itemWeight_medium;
  d->tile = TileId::shotgun;
  d->isShotgun = true;
  d->meleeAttMsgs = ItemAttMsgs("strike", "strikes me with a shotgun");
  d->rangedDmg = DiceParam(6, 3);
  d->rangedAmmoTypeUsed = ItemId::shotgunShell;
  d->rangedAttMsgs = ItemAttMsgs("fire", "fires a shotgun");
  d->rangedSndMsg = "I hear a shotgun blast.";
  d->rangedAttackSfx = SfxId::shotgunPumpFire ;
  d->rangedMakesRicochetSound = true;
  d->reloadSfx = SfxId::shotgunReload;
  addFeatureFoundIn(d, FeatureId::chest);
  addFeatureFoundIn(d, FeatureId::cabinet);
  addFeatureFoundIn(d, FeatureId::cocoon);
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::shotgunShell);
  resetData(d, ItemType::ammo);
  d->baseName = ItemName("Shotgun shell", "Shotgun shells", "a shotgun shell");
  d->maxStackSizeAtSpawn = 10;
  addFeatureFoundIn(d, FeatureId::chest);
  addFeatureFoundIn(d, FeatureId::cabinet);
  addFeatureFoundIn(d, FeatureId::cocoon);
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::incinerator);
  resetData(d, ItemType::rangedWpn);
  d->baseName = ItemName("Incinerator", "Incinerators", "an Incinerator");
  d->itemWeight = itemWeight_heavy;
  d->tile = TileId::incinerator;
  d->meleeAttMsgs = ItemAttMsgs("strike", "strikes me with an Incinerator");
  d->rangedDmg = DiceParam(1, 3);
  d->rangedDmgInfoOverride = "*";
  d->rangedAmmoTypeUsed = ItemId::napalmCartridge;
  d->rangedAttMsgs = ItemAttMsgs("fire", "fires an incinerator");
  d->rangedSndMsg = "I hear the blast of a launched missile.";
  d->rangedMissileGlyph = '*';
  d->rangedMissileClr = clrRedLgt;
  d->spawnStandardMinDLVL = 5;
  addFeatureFoundIn(d, FeatureId::chest, 25);
  addFeatureFoundIn(d, FeatureId::cabinet, 25);
  addFeatureFoundIn(d, FeatureId::cocoon, 25);
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::napalmCartridge);
  resetData(d, ItemType::ammoClip);
  d->baseName = ItemName("Napalm Cartridge", "Napalm Cartridges",
                         "a Napalm Cartridge");
  d->itemWeight = itemWeight_light;
  d->ammoContainedInClip = 3;
  d->spawnStandardMinDLVL = 5;
  d->maxStackSizeAtSpawn = 1;
  addFeatureFoundIn(d, FeatureId::chest, 25);
  addFeatureFoundIn(d, FeatureId::cabinet, 25);
  addFeatureFoundIn(d, FeatureId::cocoon, 25);
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::machineGun);
  resetData(d, ItemType::rangedWpn);
  d->baseName = ItemName("Tommy Gun", "Tommy Guns", "a Tommy Gun");
  d->itemWeight = itemWeight_medium;
  d->tile = TileId::tommyGun;
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
  addFeatureFoundIn(d, FeatureId::chest);
  addFeatureFoundIn(d, FeatureId::cabinet);
  addFeatureFoundIn(d, FeatureId::cocoon);
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::drumOfBullets);
  resetData(d, ItemType::ammoClip);
  d->baseName = ItemName("Drum of .45 ACP", "Drums of .45 ACP",
                         "a Drum of .45 ACP");
  d->ammoContainedInClip = 50;
  addFeatureFoundIn(d, FeatureId::chest);
  addFeatureFoundIn(d, FeatureId::cabinet);
  addFeatureFoundIn(d, FeatureId::cocoon);
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::pistol);
  resetData(d, ItemType::rangedWpn);
  d->baseName = ItemName("M1911 Colt", "M1911 Colt", "an M1911 Colt");
  d->itemWeight = itemWeight_light;
  d->tile = TileId::pistol;
  d->rangedDmg = DiceParam(1, 8, 4);
  d->rangedAmmoTypeUsed = ItemId::pistolClip;
  d->meleeAttMsgs = ItemAttMsgs("strike", "strikes me with a pistol");
  d->rangedAttMsgs = ItemAttMsgs("fire", "fires a pistol");
  d->rangedSndMsg = "I hear a pistol being fired.";
  d->rangedAttackSfx = SfxId::pistolFire;
  d->rangedMakesRicochetSound = true;
  d->reloadSfx = SfxId::pistolReload;
  addFeatureFoundIn(d, FeatureId::chest);
  addFeatureFoundIn(d, FeatureId::cabinet);
  addFeatureFoundIn(d, FeatureId::cocoon);
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::flareGun);
  resetData(d, ItemType::rangedWpn);
  d->baseName = ItemName("Flare Gun", "Flare Gun", "a Flare Gun");
  d->itemWeight = itemWeight_light;
  d->tile = TileId::flareGun;
  d->rangedDmg = DiceParam(1, 3, 0);
  d->rangedDmgInfoOverride = "*";
  d->rangedAmmoTypeUsed = ItemId::flare;
  d->meleeAttMsgs = ItemAttMsgs("strike", "strikes me with a flare gun");
  d->rangedAttMsgs = ItemAttMsgs("fire", "fires a flare gun");
  d->rangedSndMsg = "I hear a flare gun being fired.";
  d->propAppliedOnRanged = new PropFlared(PropTurns::standard);
  addFeatureFoundIn(d, FeatureId::chest);
  addFeatureFoundIn(d, FeatureId::cabinet);
  addFeatureFoundIn(d, FeatureId::cocoon);
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::pistolClip);
  resetData(d, ItemType::ammoClip);
  d->baseName = ItemName(".45ACP Colt cartridge", ".45ACP Colt cartridges",
                         "a .45ACP Colt cartridge");
  d->ammoContainedInClip = 7;
  addFeatureFoundIn(d, FeatureId::chest);
  addFeatureFoundIn(d, FeatureId::cabinet);
  addFeatureFoundIn(d, FeatureId::cocoon);
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::teslaCannon);
  resetData(d, ItemType::rangedWpn);
  d->baseName = ItemName("Tesla Cannon", "Tesla Cannons", "a Tesla Cannon");
  d->itemWeight = itemWeight_medium;
  d->tile = TileId::teslaCannon;
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
  addFeatureFoundIn(d, FeatureId::chest, 50);
  addFeatureFoundIn(d, FeatureId::cabinet, 50);
  addFeatureFoundIn(d, FeatureId::cocoon, 50);
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::teslaCanister);
  resetData(d, ItemType::ammoClip);
  d->baseName = ItemName("Nuclear battery", "Nuclear batteries",
                         "a Nuclear battery");
  d->ammoContainedInClip = 30;
  d->spawnStandardMinDLVL = 6;
  addFeatureFoundIn(d, FeatureId::chest, 50);
  addFeatureFoundIn(d, FeatureId::cabinet, 50);
  addFeatureFoundIn(d, FeatureId::cocoon, 50);
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::spikeGun);
  resetData(d, ItemType::rangedWpn);
  d->baseName = ItemName("Spike Gun", "Spike Guns", "a Spike Gun");
  d->itemWeight = itemWeight_medium;
  d->tile = TileId::tommyGun;
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
  addFeatureFoundIn(d, FeatureId::chest, 50);
  addFeatureFoundIn(d, FeatureId::cabinet, 50);
  addFeatureFoundIn(d, FeatureId::cocoon, 50);
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::dynamite);
  resetData(d, ItemType::explosive);
  d->baseName = ItemName("Dynamite", "Sticks of Dynamite",
                         "a Stick of Dynamite");
  d->itemWeight = itemWeight_light;
  d->tile = TileId::dynamite;
  d->clr = clrRedLgt;
  addFeatureFoundIn(d, FeatureId::chest);
  addFeatureFoundIn(d, FeatureId::cabinet);
  addFeatureFoundIn(d, FeatureId::cocoon);
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::flare);
  resetData(d, ItemType::explosive);
  d->baseName = ItemName("Flare", "Flares", "a Flare");
  d->itemWeight = itemWeight_light;
  d->tile = TileId::flare;
  d->clr = clrGray;
  d->isAmmo = true;
  addFeatureFoundIn(d, FeatureId::chest);
  addFeatureFoundIn(d, FeatureId::cabinet);
  addFeatureFoundIn(d, FeatureId::cocoon);
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::molotov);
  resetData(d, ItemType::explosive);
  d->baseName = ItemName("Molotov Cocktail", "Molotov Cocktails",
                         "a Molotov Cocktail");
  d->itemWeight = itemWeight_light;
  d->tile = TileId::molotov;
  d->clr = clrWhite;
  addFeatureFoundIn(d, FeatureId::chest);
  addFeatureFoundIn(d, FeatureId::cabinet);
  addFeatureFoundIn(d, FeatureId::cocoon);
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::throwingKnife);
  resetData(d, ItemType::missileWeapon);
  d->baseName = ItemName("Throwing Knife", "Throwing Knives",
                         "a Throwing Knife");
  d->itemWeight = itemWeight_extraLight;
  d->tile = TileId::dagger;
  d->glyph = '/';
  d->clr = clrWhite;
  d->missileHitChanceMod = 0;
  d->missileDmg = DiceParam(2, 4);
  d->maxStackSizeAtSpawn = 8;
  d->landOnHardSurfaceSoundMsg = "I hear a clanking sound.";
  d->landOnHardSurfaceSfx = SfxId::metalClank;
  d->primaryAttackMode = PrimaryAttMode::missile;
  addFeatureFoundIn(d, FeatureId::chest);
  addFeatureFoundIn(d, FeatureId::cabinet);
  addFeatureFoundIn(d, FeatureId::cocoon);
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::rock);
  resetData(d, ItemType::missileWeapon);
  d->baseName = ItemName("Rock", "Rocks", "a Rock");
  d->itemWeight = itemWeight_extraLight;
  d->tile = TileId::rock;
  d->glyph = '*';
  d->clr = clrGray;
  d->missileHitChanceMod = 10;
  d->missileDmg = DiceParam(1, 3);
  d->maxStackSizeAtSpawn = 6;
  d->primaryAttackMode = PrimaryAttMode::missile;
  addFeatureFoundIn(d, FeatureId::cabinet);
  addFeatureFoundIn(d, FeatureId::cocoon);
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::dagger);
  resetData(d, ItemType::meleeWpn);
  d->baseName = ItemName("Dagger", "Daggers", "a Dagger");
  d->itemWeight = itemWeight_light;
  d->tile = TileId::dagger;
  d->meleeAttMsgs = ItemAttMsgs("strike", "strikes me with a Dagger");
  d->meleeDmg = pair<int, int>(1, 4);
  d->meleeHitChanceMod = 20;
  d->meleeHitMediumSfx = SfxId::hitSharp;
  d->meleeHitHardSfx = SfxId::hitSharp;
  d->meleeMissSfx = SfxId::missLight;
  addFeatureFoundIn(d, FeatureId::chest);
  addFeatureFoundIn(d, FeatureId::cabinet);
  addFeatureFoundIn(d, FeatureId::tomb);
  addFeatureFoundIn(d, FeatureId::cocoon);
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::hatchet);
  resetData(d, ItemType::meleeWpn);
  d->baseName = ItemName("Hatchet", "Hatchets", "a Hatchet");
  d->itemWeight = itemWeight_light;
  d->tile = TileId::axe;
  d->meleeAttMsgs = ItemAttMsgs("strike", "strikes me with a Hatchet");
  d->meleeDmg = pair<int, int>(1, 5);
  d->meleeHitChanceMod = 15;
  d->missileHitChanceMod = -5;
  d->missileDmg = DiceParam(1, 10);
  d->isMissileWeapon = false;
  d->meleeHitMediumSfx = SfxId::hitSharp;
  d->meleeHitHardSfx = SfxId::hitSharp;
  d->meleeMissSfx = SfxId::missLight;
  addFeatureFoundIn(d, FeatureId::chest);
  addFeatureFoundIn(d, FeatureId::cabinet);
  addFeatureFoundIn(d, FeatureId::cocoon);
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::club);
  resetData(d, ItemType::meleeWpn);
  d->baseName = ItemName("Club", "Clubs", "a Club");
  d->spawnStandardMinDLVL = FIRST_CAVERN_LVL;
  d->spawnStandardMaxDLVL = 999;
  d->itemWeight = itemWeight_medium;
  d->tile = TileId::club;
  d->clr = clrBrown;
  d->meleeAttMsgs = ItemAttMsgs("strike", "strikes me with a Club");
  d->meleeDmg = pair<int, int>(2, 3);
  d->meleeHitChanceMod = 10;
  d->meleeMissSfx = SfxId::missMedium;
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::hammer);
  resetData(d, ItemType::meleeWpn);
  d->baseName = ItemName("Hammer", "Hammers", "a Hammer");
  d->itemWeight = itemWeight_medium;
  d->tile = TileId::hammer;
  d->meleeAttMsgs = ItemAttMsgs("strike", "strikes me with a Hammer");
  d->meleeDmg = pair<int, int>(2, 4);
  d->meleeHitChanceMod = 5;
  d->meleeMissSfx = SfxId::missMedium;
  addFeatureFoundIn(d, FeatureId::cabinet);
  addFeatureFoundIn(d, FeatureId::cocoon);
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::machete);
  resetData(d, ItemType::meleeWpn);
  d->baseName = ItemName("Machete", "Machetes", "a Machete");
  d->itemWeight = itemWeight_medium;
  d->tile = TileId::machete;
  d->meleeAttMsgs = ItemAttMsgs("strike", "strikes me with a Machete");
  d->meleeDmg = pair<int, int>(2, 5);
  d->meleeHitChanceMod = 0;
  d->meleeHitSmallSfx = SfxId::hitSharp;
  d->meleeHitMediumSfx = SfxId::hitSharp;
  d->meleeMissSfx = SfxId::missMedium;
  addFeatureFoundIn(d, FeatureId::cabinet);
  addFeatureFoundIn(d, FeatureId::cocoon);
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::axe);
  resetData(d, ItemType::meleeWpn);
  d->baseName = ItemName("Axe", "Axes", "an Axe");
  d->itemWeight = itemWeight_medium;
  d->tile = TileId::axe;
  d->meleeAttMsgs = ItemAttMsgs("strike", "strikes me with an axe");
  d->meleeDmg = pair<int, int>(2, 6);
  d->meleeHitChanceMod = -5;
  d->meleeHitSmallSfx = SfxId::hitSmall;
  d->meleeHitMediumSfx = SfxId::hitMedium;
  d->meleeHitHardSfx = SfxId::hitHard;
  d->meleeMissSfx = SfxId::missMedium;
  addFeatureFoundIn(d, FeatureId::cabinet);
  addFeatureFoundIn(d, FeatureId::tomb);
  addFeatureFoundIn(d, FeatureId::cocoon);
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::pitchFork);
  resetData(d, ItemType::meleeWpn);
  d->baseName = ItemName("Pitchfork", "Pitchforks", "a Pitchfork");
  d->itemWeight = itemWeight_heavy;
  d->tile = TileId::pitchfork;
  d->meleeAttMsgs = ItemAttMsgs("strike", "strikes me with a Pitchfork");
  d->meleeDmg = pair<int, int>(3, 4);
  d->meleeHitChanceMod = -5;
  d->meleeCausesKnockBack = true;
  d->meleeHitSmallSfx = SfxId::hitSharp;
  d->meleeHitMediumSfx = SfxId::hitSharp;
  d->meleeMissSfx = SfxId::missHeavy;
  addFeatureFoundIn(d, FeatureId::cabinet);
  addFeatureFoundIn(d, FeatureId::cocoon);
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::sledgeHammer);
  resetData(d, ItemType::meleeWpn);
  d->baseName = ItemName("Sledgehammer", "Sledgehammers", "a Sledgehammer");
  d->itemWeight = itemWeight_heavy;
  d->tile = TileId::sledgeHammer;
  d->meleeAttMsgs = ItemAttMsgs("strike", "strikes me with a Sledgehammer");
  d->meleeDmg = pair<int, int>(3, 5);
  d->meleeHitChanceMod = -10;
  d->meleeCausesKnockBack = true;
  d->meleeMissSfx = SfxId::missHeavy;
  addFeatureFoundIn(d, FeatureId::cabinet);
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::ironSpike);
  resetData(d, ItemType::missileWeapon);
  d->baseName = ItemName("Iron Spike", "Iron Spikes", "an Iron Spike");
  d->isAmmo = true;
  d->itemWeight = itemWeight_extraLight;
  d->tile = TileId::ironSpike;
  d->isStackable = true;
  d->clr = clrGray;
  d->glyph = '/';
  d->missileHitChanceMod = -5;
  d->missileDmg = DiceParam(1, 3);
  d->maxStackSizeAtSpawn = 12;
  d->landOnHardSurfaceSoundMsg = "I hear a clanking sound.";
  d->landOnHardSurfaceSfx = SfxId::metalClank;
  d->primaryAttackMode = PrimaryAttMode::missile;
  addFeatureFoundIn(d, FeatureId::cabinet);
  addFeatureFoundIn(d, FeatureId::cocoon);
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::playerKick);
  resetData(d, ItemType::meleeWpnIntr);
  d->meleeAttMsgs = ItemAttMsgs("kick", "");
  d->meleeHitChanceMod = 20;
  d->meleeDmg = pair<int, int>(1, 3);
  d->meleeCausesKnockBack = true;
  d->meleeMissSfx = SfxId::missMedium;
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::playerStomp);
  resetData(d, ItemType::meleeWpnIntr);
  d->meleeAttMsgs = ItemAttMsgs("stomp", "");
  d->meleeHitChanceMod = 20;
  d->meleeDmg = pair<int, int>(1, 3);
  d->meleeCausesKnockBack = false;
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::playerPunch);
  resetData(d, ItemType::meleeWpnIntr);
  d->meleeAttMsgs = ItemAttMsgs("punch", "");
  d->meleeHitChanceMod = 25;
  d->meleeDmg = pair<int, int>(1, 2);
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::zombieClaw);
  resetData(d, ItemType::meleeWpnIntr);
  d->meleeAttMsgs = ItemAttMsgs("", "claws me");
  setDmgFromMonsterData(*d, ActorData::data[ActorId::zombie]);
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::zombieClawDiseased);
  resetData(d, ItemType::meleeWpnIntr);
  d->meleeAttMsgs = ItemAttMsgs("", "claws me");
  setDmgFromMonsterData(*d, ActorData::data[ActorId::zombie]);
  d->propAppliedOnMelee = new PropInfected(PropTurns::standard);
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::zombieAxe);
  resetData(d, ItemType::meleeWpnIntr);
  d->meleeAttMsgs = ItemAttMsgs("", "chops me with a rusty axe");
  d->meleeHitSmallSfx = SfxId::hitSmall;
  d->meleeHitMediumSfx = SfxId::hitMedium;
  d->meleeHitHardSfx = SfxId::hitHard;
  setDmgFromMonsterData(*d, ActorData::data[ActorId::zombieAxe]);
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::bloatedZombiePunch);
  resetData(d, ItemType::meleeWpnIntr);
  d->meleeAttMsgs = ItemAttMsgs("", "mauls me");
  setDmgFromMonsterData(*d, ActorData::data[ActorId::bloatedZombie]);
  d->meleeCausesKnockBack = true;
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::bloatedZombieSpit);
  resetData(d, ItemType::rangedWpnIntr);
  d->rangedAttMsgs = ItemAttMsgs("", "spits acid pus at me");
  setDmgFromMonsterData(*d, ActorData::data[ActorId::bloatedZombie]);
  d->rangedSndMsg = "I hear spitting.";
  d->rangedMissileClr = clrGreenLgt;
  d->rangedDmgType = DmgType::acid;
  d->rangedMissileGlyph = '*';
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::ratBite);
  resetData(d, ItemType::meleeWpnIntr);
  d->meleeAttMsgs = ItemAttMsgs("", "bites me");
  setDmgFromMonsterData(*d, ActorData::data[ActorId::rat]);
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::ratBiteDiseased);
  resetData(d, ItemType::meleeWpnIntr);
  d->meleeAttMsgs = ItemAttMsgs("", "bites me");
  setDmgFromMonsterData(*d, ActorData::data[ActorId::rat]);
  d->propAppliedOnMelee = new PropInfected(PropTurns::standard);
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::ratThingBite);
  resetData(d, ItemType::meleeWpnIntr);
  d->meleeAttMsgs = ItemAttMsgs("", "bites me");
  setDmgFromMonsterData(*d, ActorData::data[ActorId::ratThing]);
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::wormMassBite);
  resetData(d, ItemType::meleeWpnIntr);
  d->meleeAttMsgs = ItemAttMsgs("", "bites me");
  setDmgFromMonsterData(*d, ActorData::data[ActorId::wormMass]);
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::wolfBite);
  resetData(d, ItemType::meleeWpnIntr);
  d->meleeAttMsgs = ItemAttMsgs("", "bites me");
  setDmgFromMonsterData(*d, ActorData::data[ActorId::wolf]);
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::greenSpiderBite);
  resetData(d, ItemType::meleeWpnIntr);
  d->meleeAttMsgs = ItemAttMsgs("", "bites me");
  setDmgFromMonsterData(*d, ActorData::data[ActorId::greenSpider]);
  d->propAppliedOnMelee = new PropBlind(PropTurns::specific, 4);
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::whiteSpiderBite);
  resetData(d, ItemType::meleeWpnIntr);
  d->meleeAttMsgs = ItemAttMsgs("", "bites me");
  setDmgFromMonsterData(*d, ActorData::data[ActorId::whiteSpider]);
  d->propAppliedOnMelee = new PropParalyzed(PropTurns::specific, 2);
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::redSpiderBite);
  resetData(d, ItemType::meleeWpnIntr);
  d->meleeAttMsgs = ItemAttMsgs("", "bites me");
  setDmgFromMonsterData(*d, ActorData::data[ActorId::redSpider]);
  d->propAppliedOnMelee = new PropWeakened(PropTurns::standard);
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::shadowSpiderBite);
  resetData(d, ItemType::meleeWpnIntr);
  d->meleeAttMsgs = ItemAttMsgs("", "bites me");
  setDmgFromMonsterData(*d, ActorData::data[ActorId::shadowSpider]);
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::lengSpiderBite);
  resetData(d, ItemType::meleeWpnIntr);
  d->meleeAttMsgs = ItemAttMsgs("", "bites me");
  setDmgFromMonsterData(
    *d, ActorData::data[ActorId::lengSpider]);
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::fireHoundBreath);
  resetData(d, ItemType::rangedWpnIntr);
  d->rangedAttMsgs = ItemAttMsgs("", "breaths fire at me");
  d->rangedSndMsg = "I hear a burst of flames.";
  setDmgFromMonsterData(*d, ActorData::data[ActorId::fireHound]);
  d->propAppliedOnRanged = new PropBurning(PropTurns::standard);
  d->rangedMissileClr = clrRedLgt;
  d->rangedMissileGlyph = '*';
  d->rangedMissileLeavesTrail = true;
  d->rangedMissileLeavesSmoke = true;
  d->rangedDmgType = DmgType::fire;
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::fireHoundBite);
  resetData(d, ItemType::meleeWpnIntr);
  d->meleeAttMsgs = ItemAttMsgs("", "bites me");
  setDmgFromMonsterData(*d, ActorData::data[ActorId::fireHound]);
  d->meleeDmgType = DmgType::fire;
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::frostHoundBreath);
  resetData(d, ItemType::rangedWpnIntr);
  d->rangedAttMsgs = ItemAttMsgs("", "breaths frost at me");
  d->rangedSndMsg = "I hear a chilling sound.";
  setDmgFromMonsterData(*d, ActorData::data[ActorId::frostHound]);
  d->rangedMissileClr = clrBlueLgt;
  d->rangedMissileGlyph = '*';
  d->rangedMissileLeavesTrail = true;
  d->rangedMissileLeavesSmoke = true;
  d->rangedDmgType = DmgType::cold;
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::frostHoundBite);
  resetData(d, ItemType::meleeWpnIntr);
  d->meleeAttMsgs = ItemAttMsgs("", "bites me");
  setDmgFromMonsterData(*d, ActorData::data[ActorId::frostHound]);
  d->meleeDmgType = DmgType::cold;
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::zuulBite);
  resetData(d, ItemType::meleeWpnIntr);
  d->meleeAttMsgs = ItemAttMsgs("", "bites me");
  setDmgFromMonsterData(*d, ActorData::data[ActorId::zuul]);
  d->meleeDmgType = DmgType::physical;
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::dustVortexEngulf);
  resetData(d, ItemType::meleeWpnIntr);
  d->meleeAttMsgs = ItemAttMsgs("", "engulfs me");
  setDmgFromMonsterData(*d, ActorData::data[ActorId::dustVortex]);
  d->propAppliedOnMelee = new PropBlind(PropTurns::standard);
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::fireVortexEngulf);
  resetData(d, ItemType::meleeWpnIntr);
  d->meleeAttMsgs = ItemAttMsgs("", "engulfs me");
  setDmgFromMonsterData(*d, ActorData::data[ActorId::fireVortex]);
  d->propAppliedOnMelee = new PropBurning(PropTurns::standard);
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::frostVortexEngulf);
  resetData(d, ItemType::meleeWpnIntr);
  d->meleeAttMsgs = ItemAttMsgs("", "engulfs me");
  setDmgFromMonsterData(*d, ActorData::data[ActorId::frostVortex]);
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::ghostClaw);
  resetData(d, ItemType::meleeWpnIntr);
  d->meleeAttMsgs = ItemAttMsgs("", "claws me");
  setDmgFromMonsterData(*d, ActorData::data[ActorId::ghost]);
  d->propAppliedOnMelee = new PropTerrified(PropTurns::specific, 4);
  d->meleeDmgType = DmgType::spirit;
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::phantasmSickle);
  resetData(d, ItemType::meleeWpnIntr);
  d->meleeAttMsgs = ItemAttMsgs("", "slices me with a sickle");
  setDmgFromMonsterData(*d, ActorData::data[ActorId::phantasm]);
  d->propAppliedOnMelee = new PropTerrified(PropTurns::specific, 4);
  d->meleeDmgType = DmgType::spirit;
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::wraithClaw);
  resetData(d, ItemType::meleeWpnIntr);
  d->meleeAttMsgs = ItemAttMsgs("", "claws me");
  setDmgFromMonsterData(*d, ActorData::data[ActorId::wraith]);
  d->propAppliedOnMelee = new PropTerrified(PropTurns::specific, 4);
  d->meleeDmgType = DmgType::spirit;
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::giantBatBite);
  resetData(d, ItemType::meleeWpnIntr);
  d->meleeAttMsgs = ItemAttMsgs("", "bites me");
  setDmgFromMonsterData(*d, ActorData::data[ActorId::giantBat]);
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::miGoElectricGun);
  resetData(d, ItemType::rangedWpnIntr);
  d->rangedMissileLeavesTrail = true;
  d->rangedMissileClr = clrYellow;
  d->rangedMissileGlyph = '/';
  d->rangedAttMsgs = ItemAttMsgs("", "fires an electric gun");
  d->rangedDmgType = DmgType::electric;
  d->propAppliedOnRanged = new PropParalyzed(PropTurns::specific, 2);
  d->rangedSndMsg = "I hear a bolt of electricity.";
  setDmgFromMonsterData(*d, ActorData::data[ActorId::miGo]);
  d->rangedSndVol = SndVol::high;
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::polypTentacle);
  resetData(d, ItemType::meleeWpnIntr);
  d->meleeAttMsgs = ItemAttMsgs("", "grips me with a tentacle");
  d->propAppliedOnMelee = new PropParalyzed(PropTurns::specific, 1);
  setDmgFromMonsterData(*d, ActorData::data[ActorId::flyingPolyp]);
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::ghoulClaw);
  resetData(d, ItemType::meleeWpnIntr);
  d->meleeAttMsgs = ItemAttMsgs("", "claws me");
  setDmgFromMonsterData(*d, ActorData::data[ActorId::ghoul]);
  d->propAppliedOnMelee = new PropInfected(PropTurns::standard);
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::shadowClaw);
  resetData(d, ItemType::meleeWpnIntr);
  d->meleeAttMsgs = ItemAttMsgs("", "claws me");
  setDmgFromMonsterData(*d, ActorData::data[ActorId::shadow]);
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::byakheeClaw);
  resetData(d, ItemType::meleeWpnIntr);
  d->meleeAttMsgs = ItemAttMsgs("", "claws me");
  setDmgFromMonsterData(*d, ActorData::data[ActorId::byakhee]);
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::giantMantisClaw);
  resetData(d, ItemType::meleeWpnIntr);
  d->meleeAttMsgs = ItemAttMsgs("", "claws me");
  setDmgFromMonsterData(*d, ActorData::data[ActorId::giantMantis]);
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::giantLocustBite);
  resetData(d, ItemType::meleeWpnIntr);
  d->meleeAttMsgs = ItemAttMsgs("", "bites me");
  setDmgFromMonsterData(*d, ActorData::data[ActorId::locust]);
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::mummyMaul);
  resetData(d, ItemType::meleeWpnIntr);
  d->meleeAttMsgs = ItemAttMsgs("", "mauls me");
  setDmgFromMonsterData(*d, ActorData::data[ActorId::mummy]);
  d->propAppliedOnMelee = new PropCursed(PropTurns::standard);
  d->meleeCausesKnockBack = true;
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::deepOneJavelinAtt);
  resetData(d, ItemType::rangedWpnIntr);
  d->rangedAttMsgs = ItemAttMsgs("", "throws a Javelin at me");
  setDmgFromMonsterData(*d, ActorData::data[ActorId::deepOne]);
  d->rangedSndMsg = "";
  d->rangedMissileClr = clrBrown;
  d->rangedMissileGlyph = '/';
  d->rangedSndVol = SndVol::low;
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::deepOneSpearAtt);
  resetData(d, ItemType::meleeWpnIntr);
  d->meleeAttMsgs = ItemAttMsgs("", "thrusts a spear at me");
  setDmgFromMonsterData(*d, ActorData::data[ActorId::deepOne]);
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::oozeBlackSpewPus);
  resetData(d, ItemType::meleeWpnIntr);
  d->meleeAttMsgs = ItemAttMsgs("", "spews pus on me");
  setDmgFromMonsterData(*d, ActorData::data[ActorId::oozeBlack]);
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::oozeClearSpewPus);
  resetData(d, ItemType::meleeWpnIntr);
  d->meleeAttMsgs = ItemAttMsgs("", "spews pus on me");
  setDmgFromMonsterData(*d, ActorData::data[ActorId::oozeClear]);
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::oozePutridSpewPus);
  resetData(d, ItemType::meleeWpnIntr);
  d->meleeAttMsgs = ItemAttMsgs("", "spews infected pus on me");
  setDmgFromMonsterData(*d, ActorData::data[ActorId::oozePutrid]);
  d->propAppliedOnMelee = new PropInfected(PropTurns::standard);
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::oozePoisonSpewPus);
  resetData(d, ItemType::meleeWpnIntr);
  d->meleeAttMsgs = ItemAttMsgs("", "spews poisonous pus on me");
  setDmgFromMonsterData(*d, ActorData::data[ActorId::oozePoison]);
  d->propAppliedOnMelee = new PropPoisoned(PropTurns::standard);
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::colourOOSpaceTouch);
  resetData(d, ItemType::meleeWpnIntr);
  d->meleeAttMsgs = ItemAttMsgs("", "touches me");
  setDmgFromMonsterData(*d, ActorData::data[ActorId::colourOOSpace]);
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::chthonianBite);
  resetData(d, ItemType::meleeWpnIntr);
  d->meleeAttMsgs = ItemAttMsgs("", "strikes me with a tentacle");
  d->meleeCausesKnockBack = true;
  setDmgFromMonsterData(*d, ActorData::data[ActorId::chthonian]);
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::huntingHorrorBite);
  resetData(d, ItemType::meleeWpnIntr);
  d->meleeAttMsgs = ItemAttMsgs("", "bites me");
  setDmgFromMonsterData(*d, ActorData::data[ActorId::huntingHorror]);
  d->propAppliedOnMelee = new PropParalyzed(PropTurns::standard);
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::armorLeatherJacket);
  resetData(d, ItemType::armor);
  d->baseName = ItemName("Leather Jacket", "", "a Leather Jacket");
  d->itemWeight = itemWeight_light;
  d->clr = clrBrown;
  d->spawnStandardMinDLVL = 1;
  d->armorData.absorptionPoints = 1;
  d->armorData.dmgToDurabilityFactor = 1.0;
  d->landOnHardSurfaceSoundMsg = "";
  addFeatureFoundIn(d, FeatureId::cabinet);
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::armorIronSuit);
  resetData(d, ItemType::armor);
  d->baseName = ItemName("Iron Suit", "", "an Iron Suit");
  d->itemWeight = itemWeight_heavy;
  d->clr = clrWhite;
  d->spawnStandardMinDLVL = 2;
  d->armorData.absorptionPoints = 4;
  d->armorData.dmgToDurabilityFactor = 0.5;
  d->landOnHardSurfaceSoundMsg = "I hear a crashing sound.";
  addFeatureFoundIn(d, FeatureId::cabinet);
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::armorFlackJacket);
  resetData(d, ItemType::armor);
  d->baseName = ItemName("Flak Jacket", "", "a Flak Jacket");
  d->itemWeight = itemWeight_medium;
  d->clr = clrGreen;
  d->spawnStandardMinDLVL = 3;
  d->armorData.absorptionPoints = 3;
  d->armorData.dmgToDurabilityFactor = 0.5;
  d->landOnHardSurfaceSoundMsg = "I hear a thudding sound.";
  addFeatureFoundIn(d, FeatureId::cabinet);
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::armorAsbSuit);
  resetData(d, ItemType::armor);
  d->baseName = ItemName("Asbestos Suit", "", "an Asbestos Suit");
  d->itemWeight = itemWeight_medium;
  d->clr = clrRedLgt;
  d->spawnStandardMinDLVL = 3;
  d->armorData.absorptionPoints = 1;
  d->armorData.dmgToDurabilityFactor = 1.0;
  d->landOnHardSurfaceSoundMsg = "";
  addFeatureFoundIn(d, FeatureId::cabinet);
  addFeatureFoundIn(d, FeatureId::chest);
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::armorHeavyCoat);
  resetData(d, ItemType::armor);
  d->baseName = ItemName("Heavy Coat", "", "a Heavy Coat");
  d->itemWeight = itemWeight_medium;
  d->clr = clrBlueLgt;
  d->spawnStandardMinDLVL = 3;
  d->armorData.absorptionPoints = 1;
  d->armorData.dmgToDurabilityFactor = 1.0;
  d->landOnHardSurfaceSoundMsg = "";
  addFeatureFoundIn(d, FeatureId::cabinet);
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::scrollMayhem);
  resetData(d, ItemType::scroll);
  d->spellCastFromScroll = SpellId::mayhem;
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::scrollTelep);
  resetData(d, ItemType::scroll);
  d->spellCastFromScroll = SpellId::teleport;
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::scrollPestilence);
  resetData(d, ItemType::scroll);
  d->spellCastFromScroll = SpellId::pestilence;
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::scrollSlowMon);
  resetData(d, ItemType::scroll);
  d->spellCastFromScroll = SpellId::slowEnemies;
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::scrollTerrifyMon);
  resetData(d, ItemType::scroll);
  d->spellCastFromScroll = SpellId::terrifyEnemies;
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::scrollParalMon);
  resetData(d, ItemType::scroll);
  d->spellCastFromScroll = SpellId::paralyzeEnemies;
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::scrollDetItems);
  resetData(d, ItemType::scroll);
  d->spellCastFromScroll = SpellId::detItems;
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::scrollDetTraps);
  resetData(d, ItemType::scroll);
  d->spellCastFromScroll = SpellId::detTraps;
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::scrollBless);
  resetData(d, ItemType::scroll);
  d->spellCastFromScroll = SpellId::bless;
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::scrollDarkbolt);
  resetData(d, ItemType::scroll);
  d->spellCastFromScroll = SpellId::darkbolt;
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::scrollAzaWrath);
  resetData(d, ItemType::scroll);
  d->spellCastFromScroll = SpellId::azathothsWrath;
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::scrollOpening);
  resetData(d, ItemType::scroll);
  d->spellCastFromScroll = SpellId::opening;
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::scrollSacrLife);
  resetData(d, ItemType::scroll);
  d->spellCastFromScroll = SpellId::sacrLife;
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::scrollSacrSpi);
  resetData(d, ItemType::scroll);
  d->spellCastFromScroll = SpellId::sacrSpi;
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::scrollDetMon);
  resetData(d, ItemType::scroll);
  d->spellCastFromScroll = SpellId::detMon;
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::scrollElemRes);
  resetData(d, ItemType::scroll);
  d->spellCastFromScroll = SpellId::elemRes;
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::potionVitality);
  resetData(d, ItemType::potion);
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::potionSpirit);
  resetData(d, ItemType::potion);
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::potionBlindness);
  resetData(d, ItemType::potion);
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::potionFrenzy);
  resetData(d, ItemType::potion);
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::potionFortitude);
  resetData(d, ItemType::potion);
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::potionParalyze);
  resetData(d, ItemType::potion);
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::potionRElec);
  resetData(d, ItemType::potion);
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::potionConf);
  resetData(d, ItemType::potion);
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::potionPoison);
  resetData(d, ItemType::potion);
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::potionInsight);
  resetData(d, ItemType::potion);
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::potionClairv);
  resetData(d, ItemType::potion);
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::potionRFire);
  resetData(d, ItemType::potion);
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::potionAntidote);
  resetData(d, ItemType::potion);
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::potionDescent);
  resetData(d, ItemType::potion);
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::deviceSentry);
  resetData(d, ItemType::device);
  d->baseName = ItemName("Sentry Device", "Sentry Devices", "a Sentry Device");
  d->itemValue = ItemValue::majorTreasure;
  d->isIdentified = false;
  d->clr = clrGray;
  addFeatureFoundIn(d, FeatureId::chest, 10);
  addFeatureFoundIn(d, FeatureId::tomb, 10);
  addFeatureFoundIn(d, FeatureId::cocoon, 10);
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::deviceRepeller);
  resetData(d, ItemType::device);
  d->baseName = ItemName("Repeller Device", "Repeller Devices",
                         "a Repeller Device");
  d->itemValue = ItemValue::majorTreasure;
  d->isIdentified = false;
  d->clr = clrGray;
  addFeatureFoundIn(d, FeatureId::chest, 10);
  addFeatureFoundIn(d, FeatureId::tomb, 10);
  addFeatureFoundIn(d, FeatureId::cocoon, 10);
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::deviceRejuvenator);
  resetData(d, ItemType::device);
  d->baseName = ItemName("Rejuvenator Device", "Rejuvenator Devices",
                         "a Rejuvenator Device");
  d->itemValue = ItemValue::majorTreasure;
  d->isIdentified = false;
  d->clr = clrGray;
  addFeatureFoundIn(d, FeatureId::chest, 10);
  addFeatureFoundIn(d, FeatureId::tomb, 10);
  addFeatureFoundIn(d, FeatureId::cocoon, 10);
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::deviceTranslocator);
  resetData(d, ItemType::device);
  d->baseName = ItemName("Translocator Device", "Translocator Devices",
                         "a Translocator Device");
  d->itemValue = ItemValue::majorTreasure;
  d->isIdentified = false;
  d->clr = clrGray;
  addFeatureFoundIn(d, FeatureId::chest, 10);
  addFeatureFoundIn(d, FeatureId::tomb, 10);
  addFeatureFoundIn(d, FeatureId::cocoon, 10);
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::electricLantern);
  resetData(d, ItemType::device);
  d->baseName = ItemName("Electric Lantern", "Electric Lanterns",
                         "an Electric Lantern");
  d->spawnStandardMinDLVL = 1;
  d->spawnStandardMaxDLVL = 10;
  d->chanceToIncludeInSpawnList = 50;
  d->isIdentified = true;
  d->tile = TileId::electricLantern;
  d->clr = clrYellow;
  addFeatureFoundIn(d, FeatureId::chest);
  addFeatureFoundIn(d, FeatureId::cabinet);
  addFeatureFoundIn(d, FeatureId::cocoon);
  data[int(d->id)] = d;

  d = new ItemDataT(ItemId::medicalBag);
  resetData(d, ItemType::general);
  d->isMedicalBag = true;
  d->baseName = ItemName("Medical Bag", "Medical Bags", "a Medical Bag");
  d->itemValue = ItemValue::normal;
  d->itemWeight = itemWeight_medium;
  d->spawnStandardMinDLVL = 1;
  d->spawnStandardMaxDLVL = LAST_ROOM_AND_CORRIDOR_LVL;
  d->isStackable = false;
  d->glyph = '~';
  d->clr = clrBrownDrk;
  d->tile = TileId::medicalBag;
//  d->nativeRooms.resize(0);
  data[int(d->id)] = d;
}

} //namespace

void init() {
  initDataList();
}

void cleanup() {
  for(size_t i = 1; i < int(ItemId::END); ++i) delete data[i];
}


void storeToSaveLines(vector<string>& lines) {
  for(int i = 1; i < int(ItemId::END); ++i) {
    lines.push_back(data[i]->isIdentified ? "1" : "0");

    if(data[i]->isScroll) {
      lines.push_back(data[i]->isTried ? "1" : "0");
    }
  }
}

void setupFromSaveLines(vector<string>& lines) {
  for(int i = 1; i < int(ItemId::END); ++i) {
    data[i]->isIdentified = lines.front() == "0" ? false : true;
    lines.erase(begin(lines));

    if(data[i]->isScroll) {
      data[i]->isTried = lines.front() == "0" ? false : true;
      lines.erase(begin(lines));
    }
  }
}

//TODO Remove this function
bool isWeaponStronger(const ItemDataT& data1, const ItemDataT& data2,
                      const bool IS_MELEE) {
  (void)data1;
  (void)data2;
  (void)IS_MELEE;

  return false;
}

//TODO Remove this function and make getName() function in Item instead
string getItemRef(const Item& item, const ItemRefType itemRefForm,
                  const bool SKIP_EXTRA_INFO) {

  const ItemDataT& d = item.getData();
  string ret = "";

  if(d.isDevice && d.id != ItemId::electricLantern) {
    if(d.isIdentified) {
      ret = itemRefForm == ItemRefType::plain ?
            d.baseName.name : d.baseName.nameA;
      if(!SKIP_EXTRA_INFO) {
        const Condition cond = static_cast<const Device*>(&item)->condition_;
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
    ret += d.baseName.name_plural;
  } else {
    ret = itemRefForm == ItemRefType::plain ?
          d.baseName.name : d.baseName.nameA;
  }

  if(d.isAmmoClip) {
    const ItemAmmoClip* const ammoItem =
      static_cast<const ItemAmmoClip*>(&item);
    return ret + " {" + toStr(ammoItem->ammo) + "}";
  }

  if(d.isMedicalBag) {
    const MedicalBag* const medicalBag =
      static_cast<const MedicalBag*>(&item);
    return ret + " {" + toStr(medicalBag->getNrSupplies()) + "}";
  }

  if(!SKIP_EXTRA_INFO) {
    if(d.isRangedWeapon) {
      string ammoLoadedStr = "";
      if(!d.rangedHasInfiniteAmmo) {
        const Weapon* const w = static_cast<const Weapon*>(&item);
        ammoLoadedStr = " " + toStr(w->nrAmmoLoaded) + "/" +
                        toStr(w->ammoCapacity);
      }
      return ret + ammoLoadedStr;
    }

    if((d.isScroll || d.isPotion) && d.isTried && !d.isIdentified) {
      return ret + " {tried}";
    }
  }

  return ret;
}

//TODO Remove this function and make getName() function in Item instead
string getItemInterfaceRef(const Item& item, const bool ADD_A,
                           const PrimaryAttMode attackMode) {
  const ItemDataT& d = item.getData();

  if(d.isDevice && d.id != ItemId::electricLantern) {
    if(d.isIdentified) {
      string ret = ADD_A ? d.baseName.nameA : d.baseName.name;
      const Condition cond = static_cast<const Device*>(&item)->condition_;
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
    ret = toStr(item.nrItems) + " " + d.baseName.name_plural;
  } else {
    ret = (ADD_A ? d.baseName.nameA : d.baseName.name);
  }

  const int PLAYER_RANGED_SKILL =
    Map::player->getData().abilityVals.getVal(
      AbilityId::ranged, true, *(Map::player));

  if(
    (attackMode == PrimaryAttMode::none &&
     d.primaryAttackMode == PrimaryAttMode::melee) ||
    (attackMode == PrimaryAttMode::melee && d.isMeleeWeapon)) {
    const string rollsStr = toStr(d.meleeDmg.first);
    const string sidesStr = toStr(d.meleeDmg.second);
    const int PLUS = static_cast<const Weapon*>(&item)->meleeDmgPlus;
    const string plusStr = PLUS ==  0 ? "" : ((PLUS > 0 ? "+" : "") +
                           toStr(PLUS));
    const int ITEM_SKILL = d.meleeHitChanceMod;
    const int PLAYER_MELEE_SKILL = Map::player->getData().abilityVals.getVal(
                                     AbilityId::melee, true, *(Map::player));
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
      const int MULTIPL = d.isMachineGun ?
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
    if(!d.rangedHasInfiniteAmmo) {
      const Weapon* const w = static_cast<const Weapon*>(&item);
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
    const MedicalBag* const medicalBag = static_cast<const MedicalBag*>(&item);
    return ret + " {" + toStr(medicalBag->getNrSupplies()) + "}";
  }

  if(d.isAmmoClip) {
    const ItemAmmoClip* const clip = static_cast<const ItemAmmoClip*>(&item);
    return ret + " {" + toStr(clip->ammo) + "}";
  }

  if(d.isArmor) {
    const string armorDataLine =
      static_cast<const Armor*>(&item)->getArmorDataLine(true);
    return armorDataLine.empty() ? ret : ret + " " + armorDataLine;
  }

  if((d.isScroll || d.isPotion) && d.isTried && !d.isIdentified) {
    return ret + " {tried}";
  }

  return ret;
}

} //ItemData
