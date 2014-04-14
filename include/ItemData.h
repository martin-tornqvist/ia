#ifndef DATA_H
#define DATA_H

#include <vector>
#include <string>
#include "CommonData.h"

#include "AbilityValues.h"
#include "Spells.h"
#include "FeatureData.h"
#include "Audio.h"

enum class SndVol;

enum ItemWeight {
  itemWeight_none       = 0,
  itemWeight_extraLight = 1,  //Ammo...
  itemWeight_light      = 10, //Dynamite, daggers...
  itemWeight_medium     = 50, //Most firearms...
  itemWeight_heavy      = 110 //Heavy armor, heavy weapons...
};

enum class PrimaryAttackMode {none, melee, missile, ranged};

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
  deepOneSpearAttack,
  deepOneJavelinAttack,
  oozeBlackSpewPus,
  oozePutridSpewPus,
  oozePoisonSpewPus,
  oozeClearSpewPus,
  colourOutOfSpaceTouch,
  chthonianBite,
  huntingHorrorBite,
  dustVortexEngulf, fireVortexEngulf, frostVortexEngulf,

  armorLeatherJacket,
  armorIronSuit,
  armorFlackJacket,
  armorAsbestosSuit,
  armorHeavyCoat,

  scrollOfPestilence,
  scrollOfTelep,
  scrollOfSlowEnemies,
  scrollOfTerrifyEnemies,
  scrollOfParalyzeEnemies,
  scrollOfDetTraps,
  scrollOfDetItems,
  scrollOfDetMon,
  scrollOfBless,
  scrollOfMayhem,
  scrollOfDarkbolt,
  scrollOfAzathothsWrath,
  scrollOfOpening,
  scrollOfSacrLife,
  scrollOfSacrSpi,
  scrollOfElemRes,

  potionOfVitality,
  potionOfSpirit,
  potionOfBlindness,
  potionOfFrenzy,
  potionOfFortitude,
  potionOfParalyze,
  potionOfRElec,
  potionOfConfusion,
  potionOfPoison,
  potionOfInsight,
  potionOfClairv,
  potionOfRFire,
  potionOfAntidote,
  potionOfDescent,

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

//  string overRideAbsorptionPointLabel;
  int absorptionPoints;
  double dmgToDurabilityFactor;
};

class ItemData {
public:
  ItemData(const ItemId itemId) :
    id(itemId), propAppliedOnMelee(NULL), propAppliedOnRanged(NULL) {}

  ~ItemData() {
    if(propAppliedOnMelee  != NULL) delete propAppliedOnMelee;
    if(propAppliedOnRanged != NULL) delete propAppliedOnRanged;
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
private:
  friend class ItemDataHandler;
  friend class PotionNameHandler;
  friend class Potion;
  friend class ScrollNameHandler;
  friend class Scroll;
  ItemName name;
public:
  char glyph;
  SDL_Color clr;
  TileId tile;
  PrimaryAttackMode primaryAttackMode;
  bool isExplosive, isScroll, isPotion, isDevice, isEatable;
  bool isArmor, isCloak, isRing, isAmulet;
  bool isIntrinsic, isMeleeWeapon, isRangedWeapon, isMissileWeapon, isShotgun;
  bool isMachineGun, isAmmo, isAmmoClip, isMedicalBag;
  SpellId spellCastFromScroll;
  ArmorData armorData;
  int ammoContainedInClip;
  DiceParam missileDmg;
  int missileHitChanceMod;
  pair<int, int> meleeDmg;
  int meleeHitChanceMod;
  ItemAttMsgs meleeAttMsgs;
  Prop* propAppliedOnMelee; //TODO This requires deep copy of items
  DmgType meleeDmgType;
  bool meleeCausesKnockBack;
  bool rangedCausesKnockBack;
  DiceParam rangedDmg;
  string rangedDmgInfoOverride;
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
  string rangedSndMsg;
  SndVol rangedSndVol;
  bool rangedMakesRicochetSound;
  string landOnHardSurfaceSoundMsg;
  SfxId landOnHardSurfaceSfx;
  SfxId rangedAttackSfx;
  SfxId meleeHitSmallSfx;
  SfxId meleeHitMediumSfx;
  SfxId meleeHitHardSfx;
  SfxId meleeMissSfx;
  SfxId reloadSfx;
  Prop* propAppliedOnRanged;
  vector<RoomThemeId> nativeRooms;
  vector< pair<FeatureId, int> > featuresCanBeFoundIn;
};

using namespace std;

class Engine;
class Item;
struct ActorData;

enum class ItemRefType {plain, a, plural};

class ItemDataHandler {
public:
  ItemDataHandler(Engine& engine) : eng(engine) {initDataList();}
  ~ItemDataHandler() {
    for(size_t i = 1; i < int(ItemId::endOfItemIds); i++)
      delete dataList[i];
  }

  string getItemRef(const Item& item, const ItemRefType itemRefForm,
                    const bool SKIP_EXTRA_INFO = false) const;

  string getItemInterfaceRef(
    const Item& item, const bool ADD_A,
    const PrimaryAttackMode attackMode = PrimaryAttackMode::none) const;

  ItemData* dataList[int(ItemId::endOfItemIds)];

  bool isWeaponStronger(const ItemData& data1, const ItemData& data2,
                        const bool IS_MELEE);
  void addSaveLines(vector<string>& lines) const;
  void setParamsFromSaveLines(vector<string>& lines);

private:
  void initDataList();
  void setDmgFromMonsterData(ItemData& itemData,
                             const ActorData& actorData) const;

  void resetData(ItemData* const d, ItemType const itemType) const;

  void addFeatureFoundIn(ItemData* const itemData, const FeatureId featureId,
                         const int CHANCE_TO_INCLUDE = 100) const;

  Engine& eng;
};

#endif
