#ifndef ITEM_DATA_H
#define ITEM_DATA_H

#include <vector>
#include <string>
#include "CommonData.h"

#include "AbilityValues.h"
#include "Spells.h"
#include "FeatureData.h"
#include "AudioIds.h"

enum ItemWeight_t {
  itemWeight_none       = 0,
  itemWeight_extraLight = 1,  //Ammo...
  itemWeight_light      = 10, //Dynamite, daggers...
  itemWeight_medium     = 50, //Most firearms...
  itemWeight_heavy      = 120 //Heavy armor, heavy weapons...
};

enum PrimaryAttackMode_t {
  primaryAttackMode_none,
  primaryAttackMode_melee,
  primaryAttackMode_missile,
  primaryAttackMode_ranged
};

enum ItemDataArchetypes_t {
  itemData_general,
  itemData_meleeWpn,
  itemData_meleeWpnIntr,
  itemData_rangedWpn,
  itemData_rangedWpnIntr,
  itemData_missileWeapon,
  itemData_ammo,
  itemData_ammoClip,
  itemData_scroll,
  itemData_potion,
  itemData_device,
  itemData_armor,
  itemData_explosive
};

enum ItemValue_t {
  itemValue_normal,
  itemValue_minorTreasure,
  itemValue_majorTreasure
};

enum ItemId_t {
  item_empty,

  item_trapezohedron,

  item_throwingKnife,
  item_rock,
  item_ironSpike,
  item_dagger,
  item_hatchet,
  item_club,
  item_hammer,
  item_machete,
  item_axe,
  item_pitchFork,
  item_sledgeHammer,
  item_sawedOff,
  item_pumpShotgun,
  item_machineGun,
  item_incinerator,
  item_teslaCannon,
  item_spikeGun,
  item_shotgunShell,
  item_drumOfBullets,
  item_napalmCartridge,
  item_teslaCanister,
  item_pistol,
  item_pistolClip,
  item_flareGun,
  item_dynamite,
  item_flare,
  item_molotov,

  item_playerKick,
  item_playerStomp,
  item_playerPunch,

  item_bloatedZombiePunch,
  item_bloatedZombieSpit,
  item_ratBite,
  item_ratBiteDiseased,
  item_ratThingBite,
  item_wormMassBite,
  item_greenSpiderBite,
  item_whiteSpiderBite,
  item_redSpiderBite,
  item_shadowSpiderBite,
  item_lengSpiderBite,
  item_ghoulClaw,
  item_shadowClaw,
  item_byakheeClaw,
  item_giantMantisClaw,
  item_giantLocustBite,
  item_fireHoundBreath, item_frostHoundBreath, //TODO Make them spells instead?
  item_fireHoundBite, item_frostHoundBite, item_zuulBite,
  item_giantBatBite,
  item_zombieAxe,
  item_zombieClaw,
  item_zombieClawDiseased,
  item_wolfBite,
  item_ghostClaw,
  item_phantasmSickle,
  item_wraithClaw,
  item_miGoElectricGun,
  item_polypTentacle,
  item_mummyMaul,
  item_deepOneSpearAttack,
  item_deepOneJavelinAttack,
  item_oozeBlackSpewPus,
  item_oozePutridSpewPus,
  item_oozePoisonSpewPus,
  item_oozeClearSpewPus,
  item_colourOutOfSpaceTouch,
  item_huntingHorrorBite,
  item_dustVortexEngulf, item_fireVortexEngulf, item_frostVortexEngulf,

  item_armorLeatherJacket,
  item_armorIronSuit,
  item_armorFlackJacket,
  item_armorAsbestosSuit,
  item_armorHeavyCoat,

  item_scrollOfPestilence,
  item_scrollOfTeleportation,
  item_scrollOfEnfeebleEnemies,
  item_scrollOfDetectTraps,
  item_scrollOfDetectItems,
  item_scrollOfBlessing,
  item_scrollOfMayhem,
  item_scrollOfClairvoyance,
  item_scrollOfDarkbolt,
  item_scrollOfAzathothsWrath,
  item_scrollOfOpening,
  item_scrollOfSacrificeLife,
  item_scrollOfSacrificeSpirit,

  item_thaumaturgicAlteration,

  item_potionOfHealing,
  item_potionOfSpirit,
  item_potionOfBlindness,
  item_potionOfFrenzy,
  item_potionOfFortitude,
  item_potionOfParalyzation,
  item_potionOfRElec,
  item_potionOfConfusion,
  item_potionOfPoison,
  item_potionOfInsight,
  item_potionOfRFire,
  item_potionOfAntidote,
  item_potionOfDescent,

  item_deviceSentry,
  item_deviceRepeller,
  item_deviceRejuvenator,
  item_deviceTranslocator,
  item_deviceElectricLantern,

  item_medicalBag,

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
  ItemData(const ItemId_t itemId) :
    id(itemId), propAppliedOnMelee(NULL), propAppliedOnRanged(NULL) {}

  ~ItemData() {
    if(propAppliedOnMelee  != NULL) delete propAppliedOnMelee;
    if(propAppliedOnRanged != NULL) delete propAppliedOnRanged;
  }

  ItemId_t id;
  ItemValue_t itemValue;
  ItemWeight_t itemWeight;
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
  SDL_Color color;
  Tile_t tile;
  PrimaryAttackMode_t primaryAttackMode;
  bool isExplosive, isScroll, isPotion, isDevice, isEatable;
  bool isArmor, isCloak, isRing, isAmulet;
  bool isIntrinsic, isMeleeWeapon, isRangedWeapon, isMissileWeapon, isShotgun;
  bool isMachineGun, isAmmo, isAmmoClip, isMedicalBag;
  Spell_t spellCastFromScroll;
  ArmorData armorData;
  int ammoContainedInClip;
  DiceParam missileDmg;
  int missileHitChanceMod;
  pair<int, int> meleeDmg;
  int meleeHitChanceMod;
  ItemAttackMessages meleeAttackMessages;
  Prop* propAppliedOnMelee; //TODO This requires deep copy of items
  DmgTypes_t meleeDmgType;
  bool meleeCausesKnockBack;
  bool rangedCausesKnockBack;
  DiceParam rangedDmg;
  string rangedDmgLabelOverRide;
  int rangedHitChanceMod;
  ItemId_t rangedAmmoTypeUsed;
  DmgTypes_t rangedDmgType;
  bool rangedHasInfiniteAmmo;
  char rangedMissileGlyph;
  Tile_t rangedMissileTile;
  SDL_Color rangedMissileColor;
  bool rangedMissileLeavesTrail;
  bool rangedMissileLeavesSmoke;
  ItemAttackMessages rangedAttackMessages;
  string rangedSoundMessage;
  bool rangedSoundIsLoud;
  bool rangedMakesRicochetSound;
  string landOnHardSurfaceSoundMsg;
  Sfx_t landOnHardSurfaceSfx;
  Sfx_t rangedAttackSfx;
  Sfx_t meleeAttackSfx;
  Sfx_t reloadSfx;
  Prop* propAppliedOnRanged;
  vector<RoomTheme_t> nativeRooms;
  vector< pair<Feature_t, int> > featuresCanBeFoundIn;
private:
};

using namespace std;

class Engine;
class Item;
struct ActorData;

enum ItemRef_t {itemRef_plain, itemRef_a, itemRef_plural};

class ItemDataHandler {
public:
  ItemDataHandler(Engine& engine) : eng(engine) {initDataList();}
  ~ItemDataHandler() {
    for(unsigned int i = 1; i < endOfItemIds; i++)
      delete dataList[i];
  }

  string getItemRef(const Item& item, const ItemRef_t itemRefForm,
                    const bool SKIP_EXTRA_INFO = false) const;

  string getItemInterfaceRef(
    const Item& item, const bool ADD_A,
    const PrimaryAttackMode_t attackMode = primaryAttackMode_none) const;

  ItemData* dataList[endOfItemIds];

  bool isWeaponStronger(const ItemData& data1, const ItemData& data2,
                        const bool IS_MELEE);
  void addSaveLines(vector<string>& lines) const;
  void setParamsFromSaveLines(vector<string>& lines);

private:
  void initDataList();
  void setDmgFromMonsterData(ItemData& itemData,
                             const ActorData& actorData) const;

  void resetData(ItemData* const d,
                 ItemDataArchetypes_t const archetype) const;

  void addFeatureFoundIn(ItemData* const itemData, const Feature_t featureId,
                         const int CHANCE_TO_INCLUDE = 100) const;

  Engine& eng;
};

#endif
