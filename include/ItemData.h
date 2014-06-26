#ifndef DATA_H
#define DATA_H

#include <vector>
#include <string>
#include "CmnData.h"

#include "AbilityValues.h"
#include "Spells.h"
#include "FeatureData.h"
#include "Audio.h"

enum class SndVol;

enum ItemWeight {
  itemWeight_none       = 0,
  itemWeight_extraLight = 1,  //E.g. ammo
  itemWeight_light      = 10, //E.g. dynamite, daggers
  itemWeight_medium     = 50, //E.g. most firearms
  itemWeight_heavy      = 110 //E.g. heavy armor, heavy weapons
};

enum class PrimaryAttMode {none, melee, missile, ranged};

enum class ItemType {
  general,
  meleeWpn,
  meleeWpnIntr,
  rangedWpn,
  rangedWpnIntr,
  missileWeapon,
  ammo,
  ammoClip,
  scroll,
  potion,
  device,
  armor,
  explosive
};

enum class ItemValue {normal, minorTreasure, majorTreasure};

enum class ItemId {
  empty,

  trapezohedron,

  throwingKnife,
  rock,
  ironSpike,
  dagger,
  hatchet,
  club,
  hammer,
  machete,
  axe,
  pitchFork,
  sledgeHammer,
  sawedOff,
  pumpShotgun,
  machineGun,
  incinerator,
  teslaCannon,
  spikeGun,
  shotgunShell,
  drumOfBullets,
  napalmCartridge,
  teslaCanister,
  pistol,
  pistolClip,
  flareGun,
  dynamite,
  flare,
  molotov,

  playerKick,
  playerStomp,
  playerPunch,

  bloatedZombiePunch,
  bloatedZombieSpit,
  ratBite,
  ratBiteDiseased,
  ratThingBite,
  wormMassBite,
  greenSpiderBite,
  whiteSpiderBite,
  redSpiderBite,
  shadowSpiderBite,
  lengSpiderBite,
  ghoulClaw,
  shadowClaw,
  byakheeClaw,
  giantMantisClaw,
  giantLocustBite,
  fireHoundBreath, frostHoundBreath,
  fireHoundBite, frostHoundBite, zuulBite,
  giantBatBite,
  zombieAxe,
  zombieClaw,
  zombieClawDiseased,
  wolfBite,
  ghostClaw,
  phantasmSickle,
  wraithClaw,
  miGoElectricGun,
  polypTentacle,
  mummyMaul,
  deepOneSpearAtt,
  deepOneJavelinAtt,
  oozeBlackSpewPus,
  oozePutridSpewPus,
  oozePoisonSpewPus,
  oozeClearSpewPus,
  colourOOSpaceTouch,
  chthonianBite,
  huntingHorrorBite,
  dustVortexEngulf, fireVortexEngulf, frostVortexEngulf,

  armorLeatherJacket,
  armorIronSuit,
  armorFlackJacket,
  armorAsbSuit,
  armorHeavyCoat,

  scrollPestilence,
  scrollTelep,
  scrollSlowMon,
  scrollTerrifyMon,
  scrollParalMon,
  scrollDetTraps,
  scrollDetItems,
  scrollDetMon,
  scrollBless,
  scrollMayhem,
  scrollDarkbolt,
  scrollAzaWrath,
  scrollOpening,
  scrollSacrLife,
  scrollSacrSpi,
  scrollElemRes,

  potionVitality,
  potionSpirit,
  potionBlindness,
  potionFrenzy,
  potionFortitude,
  potionParalyze,
  potionRElec,
  potionConf,
  potionPoison,
  potionInsight,
  potionClairv,
  potionRFire,
  potionAntidote,
  potionDescent,

  deviceSentry,
  deviceRepeller,
  deviceRejuvenator,
  deviceTranslocator,
  electricLantern,

  medicalBag,

  endOfItemIds
};

struct ArmorData {
  ArmorData() : absorptionPoints(0), dmgToDurabilityFactor(0.0) {}

  int absorptionPoints;
  double dmgToDurabilityFactor;
};

class ItemDataT {
public:
  ItemDataT(const ItemId itemId) :
    id(itemId), propAppliedOnMelee(nullptr), propAppliedOnRanged(nullptr) {}

  ~ItemDataT() {
    if(propAppliedOnMelee)  {delete propAppliedOnMelee;}
    if(propAppliedOnRanged) {delete propAppliedOnRanged;}
  }

  ItemId id;
  ItemValue itemValue;
  ItemWeight itemWeight;
  int spawnStandardMinDLVL;
  int spawnStandardMaxDLVL;
  int maxStackSizeAtSpawn;
  int chanceToIncludeInSpawnList;
  bool isStackable;
  bool isIdentified;
  bool isTried;
  ItemName baseName;
  char glyph;
  SDL_Color clr;
  TileId tile;
  PrimaryAttMode primaryAttackMode;
  bool isExplosive, isScroll, isPotion, isDevice, isEatable;
  bool isArmor, isCloak, isRing, isAmulet;
  bool isIntrinsic, isMeleeWeapon, isRangedWeapon, isMissileWeapon, isShotgun;
  bool isMachineGun, isAmmo, isAmmoClip, isMedicalBag;
  SpellId spellCastFromScroll;
  ArmorData armorData;
  int ammoContainedInClip;
  DiceParam missileDmg;
  int missileHitChanceMod;
  std::pair<int, int> meleeDmg;
  int meleeHitChanceMod;
  ItemAttMsgs meleeAttMsgs;
  Prop* propAppliedOnMelee; //TODO This requires deep copy of items
  DmgType meleeDmgType;
  bool meleeCausesKnockBack;
  bool rangedCausesKnockBack;
  DiceParam rangedDmg;
  std::string rangedDmgInfoOverride;
  int rangedHitChanceMod;
  ItemId rangedAmmoTypeUsed;
  DmgType rangedDmgType;
  bool rangedHasInfiniteAmmo;
  char rangedMissileGlyph;
  TileId rangedMissileTile;
  SDL_Color rangedMissileClr;
  bool rangedMissileLeavesTrail;
  bool rangedMissileLeavesSmoke;
  ItemAttMsgs rangedAttMsgs;
  std::string rangedSndMsg;
  SndVol rangedSndVol;
  bool rangedMakesRicochetSound;
  std::string landOnHardSurfaceSoundMsg;
  SfxId landOnHardSurfaceSfx;
  SfxId rangedAttackSfx;
  SfxId meleeHitSmallSfx;
  SfxId meleeHitMediumSfx;
  SfxId meleeHitHardSfx;
  SfxId meleeMissSfx;
  SfxId reloadSfx;
  Prop* propAppliedOnRanged;
  std::vector<RoomType> nativeRooms;
  std::vector< std::pair<FeatureId, int> > featuresCanBeFoundIn;
};

class Item;

enum class ItemRefType {plain, a, plural};

namespace ItemData {

extern ItemDataT* data[int(ItemId::endOfItemIds)];

void init();
void cleanup();

void storeToSaveLines(std::vector<std::string>& lines);
void setupFromSaveLines(std::vector<std::string>& lines);

std::string getItemRef(const Item& item, const ItemRefType itemRefForm,
                       const bool SKIP_EXTRA_INFO = false);

std::string getItemInterfaceRef(
  const Item& item, const bool ADD_A,
  const PrimaryAttMode attMode = PrimaryAttMode::none);

bool isWeaponStronger(const ItemDataT& data1, const ItemDataT& data2,
                      const bool IS_MELEE);

} //ItemData

#endif
