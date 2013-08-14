#ifndef ITEM_DEFINITION_H
#define ITEM_DEFINITION_H

#include "Sound.h"
#include "AbilityValues.h"
#include "StatusEffects.h"
#include "Audio.h"
#include "RoomTheme.h"
#include "Feature.h"
#include "Spells.h"

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
//  item_crowbar,
//  item_lockpick,
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
  item_oozeBlackSpewPus,
  item_oozePutridSpewPus,
  item_oozePoisonSpewPus,
  item_oozeClearSpewPus,
  item_colourOutOfSpaceTouch,
  item_huntingHorrorBite,
  item_dustVortexEngulf,
  item_fireVortexEngulf,

  item_armorLeatherJacket,
  item_armorIronSuit,
  item_armorFlackJacket,

  item_scrollOfPestilence,
  item_scrollOfTeleportation,
  item_scrollOfDescent,
  item_scrollOfEnfeebleEnemies,
  item_scrollOfDetectTraps,
  item_scrollOfDetectItems,
  item_scrollOfIdentify,
  item_scrollOfBlessing,
  item_scrollOfMayhem,
  item_scrollOfClairvoyance,
  item_scrollOfAzathothsBlast,
  item_scrollOfOpening,

  item_thaumaturgicAlteration,

  item_potionOfHealing,
  item_potionOfBlindness,
//  item_potionOfCorruption,
  item_potionOfTheCobra,
//  item_potionOfStealth,
  item_potionOfFortitude,
  item_potionOfToughness,
  item_potionOfParalyzation,
  item_potionOfConfusion,
//  item_potionOfSorcery,
  item_potionOfPoison,
  item_potionOfKnowledge,
//  item_potionOfForgetfulness,

  item_deviceSentry,
  item_deviceRepeller,
  item_deviceRejuvenator,
  item_deviceTranslocator,
//  item_deviceSpellReflector,
  item_deviceElectricLantern,

  item_medicalBag,

  endOfItemIds
};

struct ArmorData {
  ArmorData() :
    overRideAbsorptionPointLabel(""), protectsAgainstStatusBurning(false) {
    for(unsigned int i = 0; i < endOfDmgTypes; i++) {
      absorptionPoints[i] = 0;
      damageToDurabilityFactors[i] = 0.0;
    }
  }

  string overRideAbsorptionPointLabel;
  bool protectsAgainstStatusBurning;
  int absorptionPoints[endOfDmgTypes];
  double damageToDurabilityFactors[endOfDmgTypes];
  int chanceToDeflectTouchAttacks;
};

class ItemDef {
public:
  ItemDef(const ItemId_t itemId) :
    id(itemId), meleeStatusEffect(NULL), rangedStatusEffect(NULL) {
  }

  ~ItemDef() {
    if(meleeStatusEffect != NULL)
      delete meleeStatusEffect;
    if(rangedStatusEffect != NULL)
      delete rangedStatusEffect;
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
  friend class ItemData;
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
  bool isExplosive, isReadable, isScroll;
  bool isQuaffable, isDevice, isEatable, isArmor, isCloak, isRing, isAmulet;
  bool isIntrinsic, isMeleeWeapon, isRangedWeapon, isMissileWeapon, isShotgun;
  bool isMachineGun, isAmmo, isAmmoClip, isMedicalBag;
  Spells_t spellCastFromScroll;
  ArmorData armorData;
  int ammoContainedInClip;
  DiceParam missileDmg;
  int missileHitChanceMod;
  pair<int, int> meleeDmg;
  int meleeHitChanceMod;
  Abilities_t meleeAbilityUsed;
  ItemAttackMessages meleeAttackMessages;
  StatusEffect* meleeStatusEffect; //TODO This requires deep copy of items
  DmgTypes_t meleeDmgType;
  bool meleeCausesKnockBack;
  bool rangedCausesKnockBack;
  DiceParam rangedDmg;
  string rangedDmgLabelOverRide;
  int rangedHitChanceMod;
  Abilities_t rangedAbilityUsed;
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
  string landOnHardSurfaceSoundMessage;
  Audio_t rangedAudio;
  Audio_t meleeAudio;
  Audio_t reloadAudio;
  StatusEffect* rangedStatusEffect;
  vector<RoomTheme_t> nativeRooms;
  vector< pair<Feature_t, int> > featuresCanBeFoundIn;
private:
};

#endif
