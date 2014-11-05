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

enum ItemWeight
{
  itemWeight_none       = 0,
  itemWeight_extraLight = 1,  //E.g. ammo
  itemWeight_light      = 10, //E.g. dynamite, daggers
  itemWeight_medium     = 50, //E.g. most firearms
  itemWeight_heavy      = 110 //E.g. heavy armor, heavy weapons
};

enum class MainAttMode {none, melee, thrown, ranged};

enum class ItemType
{
  general,
  meleeWpn,
  meleeWpnIntr,
  rangedWpn,
  rangedWpnIntr,
  throwingWpn,
  ammo,
  ammoClip,
  scroll,
  potion,
  device,
  armor,
  explosive
};

enum class ItemValue {normal, minorTreasure, majorTreasure};

enum class ItemId
{
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

  pharaohStaff,

  sawedOff,
  pumpShotgun,
  machineGun,
  incinerator,
  teslaCannon,
  spikeGun,
  shotgunShell,
  drumOfBullets,
  incineratorCartridge,
  teslaCanister,
  pistol,
  pistolClip,
  flareGun,

  dynamite,
  flare,
  molotov,
  smokeGrenade,

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

  gasMask,
  hideousMask,

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

  deviceBlaster,
  deviceShockwave,
  deviceRejuvenator,
  deviceTranslocator,
  electricLantern,

  medicalBag,

  END
};

class ItemDataT
{
public:
  ItemDataT(const ItemId id);

  ItemDataT() = default;

  ~ItemDataT() {}

  ItemId id;
  ItemValue itemValue;
  ItemWeight itemWeight;
  Range spawnStdRange;
  int maxStackAtSpawn;
  int chanceToIncludeInSpawnList;
  bool isStackable;
  bool isIdentified;
  bool isTried;
  ItemName baseName;
  ItemName baseNameUnid;
  std::vector<std::string> baseDescr;
  char glyph;
  Clr clr;
  TileId tile;
  MainAttMode mainAttMode;
  bool isExplosive, isScroll, isPotion, isDevice;
  bool isArmor, isHeadwear;
  bool isAmmo, isAmmoClip;
  bool isIntrinsic;
  SpellId spellCastFromScroll;
  std::string landOnHardSndMsg;
  SfxId landOnHardSfx;
  std::vector<RoomType> nativeRooms;
  std::vector< std::pair<FeatureId, int> > featuresCanBeFoundIn;

  struct MeleeItemData
  {
    MeleeItemData();
    ~MeleeItemData();

    bool isMeleeWpn;
    std::pair<int, int> dmg;
    int hitChanceMod;
    ItemAttMsgs attMsgs;
    Prop* propApplied;
    DmgType dmgType;
    bool isKnockback;
    SfxId hitSmallSfx;
    SfxId hitMediumSfx;
    SfxId hitHardSfx;
    SfxId missSfx;
  } melee;

  struct RangedItemData
  {
    RangedItemData();
    ~RangedItemData();

    bool isRangedWpn, isThrowingWpn, isMachineGun, isShotgun;
    int ammoContainedInClip;
    DiceParam dmg;
    DiceParam throwDmg;
    int hitChanceMod;
    int throwHitChanceMod;
    bool isKnockback;
    std::string dmgInfoOverride;
    ItemId ammoItemId;
    DmgType dmgType;
    bool hasInfiniteAmmo;
    char missileGlyph;
    TileId missileTile;
    Clr missileClr;
    bool missileLeavesTrail;
    bool missileLeavesSmoke;
    ItemAttMsgs attMsgs;
    std::string sndMsg;
    SndVol sndVol;
    bool makesRicochetSnd;
    SfxId attSfx;
    SfxId reloadSfx;
    Prop* propApplied;
  } ranged;

  struct ArmorItemData
  {
    ArmorItemData();

    int absorptionPoints;
    double dmgToDurabilityFactor;
  } armor;
};

class Item;

namespace ItemData
{

extern ItemDataT* data[int(ItemId::END)];

void init();
void cleanup();

void storeToSaveLines(std::vector<std::string>& lines);
void setupFromSaveLines(std::vector<std::string>& lines);

bool isWpnStronger(const ItemDataT& data1, const ItemDataT& data2,
                   const bool IS_MELEE);

} //ItemData

#endif
