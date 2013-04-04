#ifndef ITEM_DEFINITION_H
#define ITEM_DEFINITION_H

#include "Sound.h"
#include "AbilityValues.h"
#include "StatusEffects.h"
#include "Audio.h"
#include "RoomTheme.h"
#include "Feature.h"

enum ItemWeight_t {
  itemWeight_none = 0,
  itemWeight_extraLight = 1, //Ammo...
  itemWeight_light = 10, //Dynamite, daggers...
  itemWeight_medium = 60, //Most firearms...
  itemWeight_heavy = 140 //Heavy armor, heavy weapons...
};

enum PrimaryAttackMode_t {
  primaryAttackMode_none,
  primaryAttackMode_melee,
  primaryAttackMode_missile,
  primaryAttackMode_ranged
};

enum ItemDefArchetypes_t {
  itemDef_general,
  itemDef_meleeWpn,
  itemDef_meleeWpnIntr,
  itemDef_rangedWpn,
  itemDef_rangedWpnIntr,
  itemDef_missileWeapon,
  itemDef_ammo,
  itemDef_ammoClip,
  itemDef_scroll,
  itemDef_potion,
  itemDef_device,
  itemDef_armor,
  itemDef_explosive
};

enum ItemId_t {
  item_empty,

  item_trapezohedron,

  item_throwingKnife,
  item_rock,
  item_ironSpike,
  item_crowbar,
  item_lockpick,
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
  item_teslaCanon,
  item_spikeGun,
  item_shotgunShell,
  item_drumOfBullets,
  item_incineratorShell,
  item_teslaCanister,
  item_pistol,
  item_pistolClip,
  item_flareGun,
  item_dynamite,
  item_flare,
  item_molotov,

  item_playerKick,
  item_playerStomp,

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
  item_hellHoundFireBreath,
  item_hellHoundBite,
  item_giantBatBite,
  item_zombieAxe,
  item_zombieClaw,
  item_zombieClawDiseased,
  item_wolfBite,
  item_ghostClaw,
  item_phantasmSickle,
  item_wraithClaw,
  item_miGoElectricGun,
  item_mummyMaul,
  item_deepOneSpearAttack,
  item_deepOneJavelinAttack,
  item_oozeGraySpewPus,
  item_oozePutridSpewPus,
  item_oozePoisonSpewPus,
  item_oozeClearSpewPus,
  item_huntingHorrorBite,
  item_dustVortexEngulf,
  item_fireVortexEngulf,

  item_armorLeatherJacket,
  item_armorIronSuit,
  item_armorFlackJacket,

  item_scrollOfPestilence,
  item_scrollOfTeleportation,
  item_scrollOfDescent,
  item_scrollOfConfuseEnemies,
  item_scrollOfParalyzeEnemies,
  item_scrollOfSlowEnemies,
  item_scrollOfDetectTraps,
  item_scrollOfDetectItems,
  item_scrollOfIdentify,
  item_scrollOfBlessing,
  item_scrollOfMayhem,
  item_scrollOfClairvoyance,
  item_scrollOfAzathothsBlast,

  item_thaumaturgicAlteration,

  item_potionOfHealing,
  item_potionOfBlindness,
  item_potionOfCorruption,
  item_potionOfTheCobra,
//  item_potionOfStealth,
  item_potionOfFortitude,
  item_potionOfToughness,
  item_potionOfParalyzation,
  item_potionOfConfusion,
  item_potionOfSorcery,
//  item_potionOfForgetfulness,

  item_deviceSentry,
  item_deviceRepeller,
  item_deviceRejuvenator,
  item_deviceTranslocator,
//  item_deviceSpellReflector,
  item_deviceElectricLantern,

  endOfItemIds
};

struct ArmorData {
  ArmorData() :
    overRideAbsorptionPointLabel(""), protectsAgainstStatusBurning(false) {
    for(unsigned int i = 0; i < endOfDamageTypes; i++) {
      absorptionPoints[i] = 0;
      damageToDurabilityFactors[i] = 0.0;
    }
  }

  string overRideAbsorptionPointLabel;
  bool protectsAgainstStatusBurning;
  int absorptionPoints[endOfDamageTypes];
  double damageToDurabilityFactors[endOfDamageTypes];
  int chanceToDeflectTouchAttacks;
};

class ItemDefinition {
public:
  ItemDefinition(const ItemId_t itemId) :
    id(itemId), meleeStatusEffect(NULL), rangedStatusEffect(NULL) {
  }

  ~ItemDefinition() {
    if(meleeStatusEffect != NULL)
      delete meleeStatusEffect;
    if(rangedStatusEffect != NULL)
      delete rangedStatusEffect;
  }

  ItemId_t id;
  ItemWeight_t itemWeight;
  int spawnStandardMinDLVL;
  int spawnStandardMaxDLVL;
  int maxStackSizeAtSpawn;
  int chanceToIncludeInSpawnList;
  bool isStackable;
  bool isIdentified;
  bool isTried;
private:
  friend class ItemData;
  friend class PotionNameHandler;
  friend class Potion;
  friend class ScrollNameHandler;
  friend class Scroll;
  ItemName name;
public:
  char glyph;
  sf::Color color;
  Tile_t tile;
  PrimaryAttackMode_t primaryAttackMode;
  bool isExplosive;
  bool isReadable;
  bool isScroll;
  bool isScrollLearned;
  bool isScrollLearnable;
  bool isQuaffable;
  bool isDevice;
  bool isEatable;
  bool isArmor;
  ArmorData armorData;
  bool isCloak;
  bool isRing;
  bool isAmulet;
  bool isIntrinsic;
  bool isMeleeWeapon;
  bool isRangedWeapon;
  bool isMissileWeapon;
  bool isShotgun;
  bool isMachineGun;
  bool isAmmo;
  bool isAmmoClip;
  int ammoContainedInClip;
  DiceParam missileDmg;
  int missileBaseAttackSkill;
  pair<int, int> meleeDmg;
  int meleeBaseAttackSkill;
  Abilities_t meleeAbilityUsed;
  ItemAttackMessages meleeAttackMessages;
  StatusEffect* meleeStatusEffect; //TODO This requires deep copy of items
  DamageTypes_t meleeDamageType;
  bool meleeCausesKnockBack;
  bool rangedCausesKnockBack;
  DiceParam rangedDmg;
  string rangedDmgLabelOverRide;
  int rangedBaseAttackSkill;
  Abilities_t rangedAbilityUsed;
  ItemId_t rangedAmmoTypeUsed;
  DamageTypes_t rangedDamageType;
  bool rangedHasInfiniteAmmo;
  char rangedMissileGlyph;
  Tile_t rangedMissileTile;
  sf::Color rangedMissileColor;
  bool rangedMissileLeavesTrail;
  bool rangedMissileLeavesSmoke;
  ItemAttackMessages rangedAttackMessages;
  string rangedSoundMessage;
  bool rangedSoundIsLoud;
  string landOnHardSurfaceSoundMessage;
  Audio_t rangedAudio;
  Audio_t meleeAudio;
  Audio_t reloadAudio;
  StatusEffect* rangedStatusEffect;
  int castFromMemoryChance;
  int spellTurnsPerPercentCooldown;
  vector<RoomTheme_t> nativeRooms;
  vector<Feature_t> featuresCanBeFoundIn;

private:
};

#endif
