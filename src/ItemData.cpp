#include "ItemData.h"

#include <iostream>
#include <climits>

#include "Init.h"
#include "Colors.h"
#include "Item.h"
#include "CmnData.h"
#include "ActorData.h"
#include "ItemScroll.h"
#include "ItemPotion.h"
#include "ActorPlayer.h"
#include "Sound.h"
#include "ItemDevice.h"
#include "Map.h"

using namespace std;

ItemDataT::ItemDataT(const ItemId id_) :
    id                              (id_),
    type                            (ItemType::general),
    value                           (ItemValue::normal),
    weight                          (ItemWeight::none),
    allowSpawn                      (true),
    spawnStdRange                   (Range(1, INT_MAX)),
    maxStackAtSpawn                 (1),
    chanceToIncludeInFloorSpawnList (100),
    isStackable                     (true),
    isIdentified                    (true),
    isTried                         (false),
    baseName                        (),
    glyph                           ('X'),
    clr                             (clrWhite),
    tile                            (TileId::empty),
    mainAttMode                     (MainAttMode::none),
    spellCastFromScroll             (SpellId::END),
    landOnHardSndMsg                ("I hear a thudding sound."),
    landOnHardSfx                   (),
    shockWhileInBackpack            (0),
    shockWhileEquipped              (0),
    melee                           (ItemMeleeData()),
    ranged                          (ItemRangedData()),
    armor                           (ItemArmorData())
{
    for (int i = 0; i < int(AbilityId::END); ++i)
    {
        abilityModsWhileEquipped[i] = 0;
    }

    baseDescr.clear();
    nativeRooms.clear();
    containerSpawnRules.clear();
}

ItemDataT::ItemMeleeData::ItemMeleeData() :
    isMeleeWpn                      (false),
    dmg                             (pair<int, int>(0, 0)),
    hitChanceMod                    (0),
    attMsgs                         (ItemAttMsgs()),
    propApplied                     (nullptr),
    dmgType                         (DmgType::physical),
    knocksBack                      (false),
    hitSmallSfx                     (SfxId::END),
    hitMediumSfx                    (SfxId::END),
    hitHardSfx                      (SfxId::END),
    missSfx                         (SfxId::END) {}

ItemDataT::ItemMeleeData::~ItemMeleeData()
{
    if (propApplied) {delete propApplied;}
}

ItemDataT::ItemRangedData::ItemRangedData() :
    isRangedWpn                     (false),
    isThrowingWpn                   (false),
    isMachineGun                    (false),
    isShotgun                       (false),
    maxNrAmmoInClip                 (0),
    dmg                             (DiceParam()),
    throwDmg                        (DiceParam()),
    hitChanceMod                    (0),
    throwHitChanceMod               (0),
    effectiveRange                  (3),
    knocksBack                      (false),
    dmgInfoOverride                 (""),
    ammoItemId                      (ItemId::END),
    dmgType                         (DmgType::physical),
    hasInfiniteAmmo                 (false),
    missileGlyph                    ('/'),
    missileTile                     (TileId::projectileStdFrontSlash),
    missileClr                      (clrWhite),
    missileLeavesTrail              (false),
    missileLeavesSmoke              (false),
    attMsgs                         (ItemAttMsgs()),
    sndMsg                          (""),
    sndVol                          (SndVol::low),
    makesRicochetSnd                (false),
    attSfx                          (SfxId::END),
    reloadSfx                       (SfxId::END),
    propApplied                     (nullptr) {}

ItemDataT::ItemRangedData::~ItemRangedData()
{
    if (propApplied) {delete propApplied;}
}

ItemDataT::ItemArmorData::ItemArmorData() :
    armorPoints             (0),
    dmgToDurabilityFactor   (0.0) {}

namespace ItemData
{

ItemDataT* data[int(ItemId::END)];

namespace
{

void addFeatureFoundIn(ItemDataT& data, const FeatureId featureId,
                       const int CHANCE_TO_INCL = 100)
{
    data.containerSpawnRules.push_back({featureId, CHANCE_TO_INCL});
}

//------------------------------- ITEM ARCHETYPES (DEFAULTS)
void resetData(ItemDataT& d, ItemType const itemType)
{
    switch (itemType)
    {
    case ItemType::general:
    {
        d = ItemDataT(d.id);
    } break;

    case ItemType::meleeWpn:
    {
        resetData(d, ItemType::general);
        d.type = ItemType::meleeWpn;
        d.isStackable = false;
        d.weight = ItemWeight::medium;
        d.glyph = '(';
        d.clr = clrWhite;
        d.mainAttMode = MainAttMode::melee;
        d.melee.isMeleeWpn = true;
        d.melee.missSfx = SfxId::missMedium;
        d.melee.hitSmallSfx = SfxId::hitSmall;
        d.melee.hitMediumSfx = SfxId::hitMedium;
        d.melee.hitHardSfx = SfxId::hitHard;
    } break;

    case ItemType::meleeWpnIntr:
    {
        resetData(d, ItemType::meleeWpn);
        d.type = ItemType::meleeWpnIntr;
        d.spawnStdRange = Range(-1, -1);
        d.chanceToIncludeInFloorSpawnList = 0;
        d.allowSpawn = false;
        d.melee.hitSmallSfx = SfxId::hitSmall;
        d.melee.hitMediumSfx = SfxId::hitMedium;
        d.melee.hitHardSfx = SfxId::hitHard;
        d.melee.missSfx = SfxId::missMedium;
    } break;

    case ItemType::rangedWpn:
    {
        resetData(d, ItemType::general);
        d.type = ItemType::rangedWpn;
        d.isStackable = false;
        d.weight = ItemWeight::medium;
        d.glyph = '}';
        d.clr = clrWhite;
        d.melee.isMeleeWpn = true;
        d.melee.dmg = pair<int, int>(1, 6);
        d.mainAttMode = MainAttMode::ranged;
        d.ranged.isRangedWpn = true;
        d.ranged.missileGlyph = '/';
        d.ranged.missileClr = clrWhite;
        d.spawnStdRange.upper = DLVL_LAST_MID_GAME;
        d.melee.hitSmallSfx = SfxId::hitSmall;
        d.melee.hitMediumSfx = SfxId::hitMedium;
        d.melee.hitHardSfx = SfxId::hitHard;
        d.melee.missSfx = SfxId::missMedium;
        d.ranged.sndVol = SndVol::high;
    } break;

    case ItemType::rangedWpnIntr:
    {
        resetData(d, ItemType::rangedWpn);
        d.type = ItemType::rangedWpnIntr;
        d.ranged.hasInfiniteAmmo = true;
        d.spawnStdRange = Range(-1, -1);
        d.chanceToIncludeInFloorSpawnList = 0;
        d.allowSpawn = false;
        d.melee.isMeleeWpn = false;
        d.ranged.missileGlyph = '*';
        d.ranged.sndVol = SndVol::low;
    } break;

    case ItemType::throwingWpn:
    {
        resetData(d, ItemType::general);
        d.type = ItemType::throwingWpn;
        d.weight = ItemWeight::extraLight;
        d.isStackable = true;
        d.ranged.isThrowingWpn = true;
        d.spawnStdRange.upper = DLVL_LAST_MID_GAME;
        d.ranged.sndVol = SndVol::low;
    } break;

    case ItemType::ammo:
    {
        resetData(d, ItemType::general);
        d.type = ItemType::ammo;
        d.weight = ItemWeight::extraLight;
        d.glyph = '{';
        d.clr = clrWhite;
        d.tile = TileId::ammo;
        d.spawnStdRange.upper = DLVL_LAST_MID_GAME;
    } break;

    case ItemType::ammoClip:
    {
        resetData(d, ItemType::ammo);
        d.type = ItemType::ammoClip;
        d.weight = ItemWeight::light;
        d.isStackable = false;
        d.spawnStdRange.upper = DLVL_LAST_MID_GAME;
    } break;

    case ItemType::scroll:
    {
        resetData(d, ItemType::general);
        d.type = ItemType::scroll;
        d.baseDescr =
        {
            "A short transcription of an eldritch incantation. There is a strange aura "
            "about it, as if some power was imbued in the paper itself.",
            "It should be possible to pronounce it correctly, but the purpose is unclear."
        };
        d.value = ItemValue::minorTreasure;
        d.chanceToIncludeInFloorSpawnList = 40;
        d.weight = ItemWeight::none;
        d.isIdentified = false;
        d.glyph = '?';
        d.clr = clrWhite;
        d.tile = TileId::scroll;
        d.maxStackAtSpawn = 1;
        d.landOnHardSndMsg = "";
        addFeatureFoundIn(d, FeatureId::chest);
        addFeatureFoundIn(d, FeatureId::tomb);
        addFeatureFoundIn(d, FeatureId::cabinet, 25);
        addFeatureFoundIn(d, FeatureId::cocoon, 25);
    } break;

    case ItemType::potion:
    {
        resetData(d, ItemType::general);
        d.type = ItemType::potion;
        d.baseDescr =
        {
            "A small glass bottle containing a mysterious concoction."
        };
        d.value = ItemValue::minorTreasure;
        d.chanceToIncludeInFloorSpawnList = 55;
        d.weight = ItemWeight::light;
        d.isIdentified = false;
        d.glyph = '!';
        d.tile = TileId::potion;
        d.ranged.isThrowingWpn = true;
        d.ranged.throwHitChanceMod = 15;
        d.ranged.throwDmg = DiceParam(1, 3, 0);
        d.maxStackAtSpawn = 2;
        d.landOnHardSndMsg = "";
        addFeatureFoundIn(d, FeatureId::chest);
        addFeatureFoundIn(d, FeatureId::tomb);
        addFeatureFoundIn(d, FeatureId::cabinet, 25);
        addFeatureFoundIn(d, FeatureId::cocoon, 25);
    } break;

    case ItemType::device:
    {
        resetData(d, ItemType::general);
        d.type = ItemType::device;
        d.baseNameUnId = {"Strange Device", "Strange Devices", "a Strange Device"};
        d.baseDescr =
        {
            "A small piece of machinery. It could not possibly have been designed by a "
            "human mind. Even for its small size, it seems incredibly complex. There is "
            "no hope of understanding the purpose or function of it through normal means."
        };
        d.chanceToIncludeInFloorSpawnList = 12;
        d.weight = ItemWeight::light;
        d.isIdentified = false;
        d.glyph = '~';
        d.tile = TileId::device1;
        d.isStackable = false;
        d.landOnHardSndMsg = "I hear a clanking sound.";
        d.landOnHardSfx = SfxId::metalClank;
        addFeatureFoundIn(d, FeatureId::chest, 10);
    } break;

    case ItemType::armor:
    {
        resetData(d, ItemType::general);
        d.type = ItemType::armor;
        d.weight = ItemWeight::heavy;
        d.glyph = '[';
        d.tile = TileId::armor;
        d.isStackable = false;
    } break;

    case ItemType::headWear:
    {
        resetData(d, ItemType::general);
        d.type = ItemType::headWear;
        d.glyph = '[';
        d.isStackable = false;
    } break;

    case ItemType::amulet:
    {
        resetData(d, ItemType::general);
        d.type = ItemType::amulet;
        d.value = ItemValue::majorTreasure;
        d.tile = TileId::amulet;
        d.glyph = '\"';
        d.weight = ItemWeight::light;
        d.isIdentified = false;
        d.isStackable = false;
        d.chanceToIncludeInFloorSpawnList = 2;
        addFeatureFoundIn(d, FeatureId::tomb, 16);
        addFeatureFoundIn(d, FeatureId::chest, 5);
    } break;

    case ItemType::ring:
    {
        resetData(d, ItemType::general);
        d.type = ItemType::ring;
        d.value = ItemValue::majorTreasure;
        d.tile = TileId::ring;
        d.glyph = '=';
        d.weight = ItemWeight::extraLight;
        d.isIdentified = false;
        d.isStackable = false;
        d.chanceToIncludeInFloorSpawnList = 2;
        addFeatureFoundIn(d, FeatureId::tomb, 16);
        addFeatureFoundIn(d, FeatureId::chest, 5);
    } break;

    case ItemType::explosive:
    {
        resetData(d, ItemType::general);
        d.type = ItemType::explosive;
        d.weight = ItemWeight::light;
        d.glyph = '-';
        d.maxStackAtSpawn = 2;
        d.landOnHardSndMsg = "";
    } break;

    default: {} break;
    }
}

void setDmgFromMonId(ItemDataT& itemData, const ActorId id)
{
    const auto& actorData     = ActorData::data[int(id)];
    itemData.melee.dmg        = pair<int, int>(1, actorData.dmgMelee);
    itemData.ranged.dmg       = DiceParam(1, actorData.dmgRanged, 0);
    itemData.ranged.throwDmg  = DiceParam(1, actorData.dmgRanged, 0);
}

//------------------------------- LIST OF ITEMS
void initDataList()
{
    ItemDataT* d = nullptr;

    d = new ItemDataT(ItemId::trapezohedron);
    resetData(*d, ItemType::general);
    d->baseName =
    {
        "Shining Trapezohedron", "Shining Trapezohedrons", "The Shining Trapezohedron"
    };
    d->spawnStdRange = Range(-1, -1);
    d->chanceToIncludeInFloorSpawnList = 0;
    d->allowSpawn = false;
    d->isStackable = false;
    d->glyph = '*';
    d->clr = clrRedLgt;
    d->tile = TileId::trapezohedron;
    data[int(d->id)] = d;

    d = new ItemDataT(ItemId::sawedOff);
    resetData(*d, ItemType::rangedWpn);
    d->baseName =
    {
        "Sawed-off Shotgun", "Sawed-off shotguns", "a Sawed-off Shotgun"
    };
    d->baseDescr =
    {
        "Compared to a standard shotgun, the sawed - off has a shorter effective range. "
        "At close range it is more devastating however. It holds two barrels, and needs "
        "to be reloaded after both are discharged"
    };
    d->weight = ItemWeight::medium;
    d->tile = TileId::shotgun;
    d->ranged.isShotgun = true;
    d->melee.attMsgs = {"strike", "strikes me with a shotgun"};
    d->ranged.dmg = DiceParam(8, 3);
    d->ranged.effectiveRange = 3;
    d->ranged.ammoItemId = ItemId::shotgunShell;
    d->ranged.attMsgs = {"fire", "fires a shotgun"};
    d->ranged.sndMsg = "I hear a shotgun blast.";
    d->ranged.attSfx = SfxId::shotgunSawedOffFire;
    d->ranged.makesRicochetSnd = true;
    d->ranged.reloadSfx = SfxId::shotgunReload;
    addFeatureFoundIn(*d, FeatureId::chest);
    addFeatureFoundIn(*d, FeatureId::cabinet);
    addFeatureFoundIn(*d, FeatureId::cocoon);
    data[int(d->id)] = d;

    d = new ItemDataT(ItemId::pumpShotgun);
    resetData(*d, ItemType::rangedWpn);
    d->baseName = {"Pump Shotgun", "Pump shotguns", "a Pump Shotgun"};
    d->baseDescr =
    {
        "A pump - action shotgun has a handgrip that can be pumped back and forth in "
        "order to eject a spent round of ammunition and to chamber a fresh one. It has "
        "a single barrel above a tube magazine into which shells are inserted. The "
        "magazine has a capacity of 8 shells."
    };
    d->weight = ItemWeight::medium;
    d->tile = TileId::shotgun;
    d->ranged.isShotgun = true;
    d->melee.attMsgs = {"strike", "strikes me with a shotgun"};
    d->ranged.dmg = DiceParam(6, 3);
    d->ranged.effectiveRange = 5;
    d->ranged.ammoItemId = ItemId::shotgunShell;
    d->ranged.attMsgs = {"fire", "fires a shotgun"};
    d->ranged.sndMsg = "I hear a shotgun blast.";
    d->ranged.attSfx = SfxId::shotgunPumpFire ;
    d->ranged.makesRicochetSnd = true;
    d->ranged.reloadSfx = SfxId::shotgunReload;
    addFeatureFoundIn(*d, FeatureId::chest);
    addFeatureFoundIn(*d, FeatureId::cabinet);
    addFeatureFoundIn(*d, FeatureId::cocoon);
    data[int(d->id)] = d;

    d = new ItemDataT(ItemId::shotgunShell);
    resetData(*d, ItemType::ammo);
    d->baseName = {"Shotgun shell", "Shotgun shells", "a shotgun shell"};
    d->baseDescr =
    {
        "A cartridge designed to be fired from a shotgun."
    };
    d->maxStackAtSpawn = 10;
    addFeatureFoundIn(*d, FeatureId::chest);
    addFeatureFoundIn(*d, FeatureId::cabinet);
    addFeatureFoundIn(*d, FeatureId::cocoon);
    data[int(d->id)] = d;

    d = new ItemDataT(ItemId::incinerator);
    resetData(*d, ItemType::rangedWpn);
    d->baseName = {"Incinerator", "Incinerators", "an Incinerator"};
    d->baseDescr =
    {
        "This hellish, experimental weapon launches an explosive fireball. Best ued "
        "with extreme caution."
    };
    d->weight = ItemWeight::heavy;
    d->tile = TileId::incinerator;
    d->melee.attMsgs = {"strike", "strikes me with an Incinerator"};
    d->ranged.dmg = DiceParam(1, 3);
    d->ranged.effectiveRange = 8;
    d->ranged.dmgInfoOverride = "* ";
    d->ranged.ammoItemId = ItemId::incineratorAmmo;
    d->ranged.attMsgs = {"fire", "fires an incinerator"};
    d->ranged.sndMsg = "I hear the blast of a launched missile.";
    d->ranged.missileGlyph = '*';
    d->ranged.missileClr = clrRedLgt;
    d->spawnStdRange.lower = 5;
    addFeatureFoundIn(*d, FeatureId::chest, 25);
    addFeatureFoundIn(*d, FeatureId::cabinet, 25);
    addFeatureFoundIn(*d, FeatureId::cocoon, 25);
    data[int(d->id)] = d;

    d = new ItemDataT(ItemId::incineratorAmmo);
    resetData(*d, ItemType::ammoClip);
    d->baseName =
    {
        "Incinerator Cartridge", "Incinerator Cartridges", "an Incinerator Cartridge"
    };
    d->baseDescr =
    {
        "Ammunition designed for Incinerators."
    };
    d->weight = ItemWeight::light;
    d->ranged.maxNrAmmoInClip = 5;
    d->spawnStdRange.lower = 5;
    d->maxStackAtSpawn = 1;
    addFeatureFoundIn(*d, FeatureId::chest, 25);
    addFeatureFoundIn(*d, FeatureId::cabinet, 25);
    addFeatureFoundIn(*d, FeatureId::cocoon, 25);
    data[int(d->id)] = d;

    d = new ItemDataT(ItemId::machineGun);
    resetData(*d, ItemType::rangedWpn);
    d->baseName = {"Tommy Gun", "Tommy Guns", "a Tommy Gun"};
    d->baseDescr =
    {
        "\"Tommy Gun\" is a nickname for the Thompson submachine gun - an automatic "
        "firearm with a drum magazine and verical foregrip. It fires .45 ACP "
        "ammunition. The drum magazine has a capacity of 50 rounds."
    };
    d->weight = ItemWeight::medium;
    d->tile = TileId::tommyGun;
    d->melee.attMsgs = {"strike", "strikes me with a Tommy Gun"};
    d->ranged.isMachineGun = true;
    d->ranged.dmg = DiceParam(2, 2, 2);
    d->ranged.hitChanceMod = -10;
    d->ranged.effectiveRange = 8;
    d->ranged.ammoItemId = ItemId::drumOfBullets;
    d->ranged.attMsgs = {"fire", "fires a Tommy Gun"};
    d->ranged.sndMsg = "I hear the burst of a machine gun.";
    d->ranged.attSfx = SfxId::machineGunFire;
    d->ranged.makesRicochetSnd = true;
    d->ranged.reloadSfx = SfxId::machineGunReload;
    addFeatureFoundIn(*d, FeatureId::chest);
    addFeatureFoundIn(*d, FeatureId::cabinet);
    addFeatureFoundIn(*d, FeatureId::cocoon);
    data[int(d->id)] = d;

    d = new ItemDataT(ItemId::drumOfBullets);
    resetData(*d, ItemType::ammoClip);
    d->baseName = {"Drum of .45 ACP", "Drums of .45 ACP", "a Drum of .45 ACP"};
    d->baseDescr =
    {
        "Ammunition used by Tommy Guns."
    };
    d->ranged.maxNrAmmoInClip = 50;
    addFeatureFoundIn(*d, FeatureId::chest);
    addFeatureFoundIn(*d, FeatureId::cabinet);
    addFeatureFoundIn(*d, FeatureId::cocoon);
    data[int(d->id)] = d;

    d = new ItemDataT(ItemId::pistol);
    resetData(*d, ItemType::rangedWpn);
    d->baseName = {"M1911 Colt", "M1911 Colt", "an M1911 Colt"};
    d->baseDescr =
    {
        "A semi-automatic, magazine-fed pistol chambered for the .45 ACP cartridge."
    };
    d->weight = ItemWeight::light;
    d->tile = TileId::pistol;
    d->ranged.dmg = DiceParam(1, 8, 4);
    d->ranged.effectiveRange = 6;
    d->ranged.ammoItemId = ItemId::pistolClip;
    d->melee.attMsgs = {"strike", "strikes me with a pistol"};
    d->ranged.attMsgs = {"fire", "fires a pistol"};
    d->ranged.sndMsg = "I hear a pistol being fired.";
    d->ranged.attSfx = SfxId::pistolFire;
    d->ranged.makesRicochetSnd = true;
    d->ranged.reloadSfx = SfxId::pistolReload;
    addFeatureFoundIn(*d, FeatureId::chest);
    addFeatureFoundIn(*d, FeatureId::cabinet);
    addFeatureFoundIn(*d, FeatureId::cocoon);
    data[int(d->id)] = d;

    d = new ItemDataT(ItemId::pistolClip);
    resetData(*d, ItemType::ammoClip);
    d->baseName = {".45ACP Colt cartridge", ".45ACP Colt cartridges",
                   "a .45ACP Colt cartridge"
                  };
    d->baseDescr =
    {
        "Ammunition used by Colt pistols."
    };
    d->ranged.maxNrAmmoInClip = 7;
    addFeatureFoundIn(*d, FeatureId::chest);
    addFeatureFoundIn(*d, FeatureId::cabinet);
    addFeatureFoundIn(*d, FeatureId::cocoon);
    data[int(d->id)] = d;

    d = new ItemDataT(ItemId::miGoGun);
    resetData(*d, ItemType::rangedWpn);
    d->baseName =
    {
        "Mi-go Electric Gun", "Mi-go Electric Gun", "a Mi-go Electric Gun"
    };
    d->baseDescr =
    {
        "A weapon created by the Mi-go. It fires devastating bolts of electricity.",
        "If there is no ammunition to fuel the weapon, it can draw power from the "
        "essence of the wielder (press [f] while no ammo loaded)."
    };
    d->spawnStdRange = Range(-1, -1);
    d->weight = ItemWeight::medium;
    d->tile = TileId::miGoGun;
    d->clr = clrYellow;
    d->ranged.dmg = DiceParam(3, 6, 0);
    d->ranged.hitChanceMod = 5;
    d->ranged.effectiveRange = 4;
    d->ranged.propApplied = new PropParalyzed(PropTurns::specific, 2);
    d->ranged.dmgType = DmgType::electric;
    d->ranged.ammoItemId = ItemId::miGoGunAmmo;
    d->ranged.hasInfiniteAmmo = false;
    d->ranged.missileLeavesTrail = true;
    d->ranged.missileClr = clrYellow;
    d->melee.attMsgs = {"strike", "strikes me with a Mi-go Electric Gun"};
    d->ranged.attMsgs = {"fire", "fires a Mi-go Electric Gun"};
    d->ranged.sndMsg = "I hear a bolt of electricity.";
    d->ranged.attSfx = SfxId::miGoGunFire;
    d->ranged.reloadSfx = SfxId::machineGunReload;
    d->ranged.makesRicochetSnd = false;
    data[int(d->id)] = d;

    d = new ItemDataT(ItemId::miGoGunAmmo);
    resetData(*d, ItemType::ammoClip);
    d->baseName =
    {
        "Mi-go Electric Cell", "Mi-go Electric Cells", "a Mi-go Electric Cell"
    };
    d->baseDescr =
    {
        "Ammunition for the Mi-go Electric gun."
    };
    d->clr = clrYellow;
    d->spawnStdRange = Range(-1, -1);
    d->ranged.maxNrAmmoInClip = 20;
    data[int(d->id)] = d;

    d = new ItemDataT(ItemId::flareGun);
    resetData(*d, ItemType::rangedWpn);
    d->baseName = {"Flare Gun", "Flare Gun", "a Flare Gun"};
    d->baseDescr =
    {
        "Launches flares. Not designed to function as a weapon."
    };
    d->weight = ItemWeight::light;
    d->tile = TileId::flareGun;
    d->ranged.dmg = DiceParam(1, 3, 0);
    d->ranged.effectiveRange = 3;
    d->ranged.dmgInfoOverride = "*";
    d->ranged.ammoItemId = ItemId::flare;
    d->melee.attMsgs = {"strike", "strikes me with a flare gun"};
    d->ranged.attMsgs = {"fire", "fires a flare gun"};
    d->ranged.sndMsg = "I hear a flare gun being fired.";
    d->ranged.propApplied = new PropFlared(PropTurns::std);
    addFeatureFoundIn(*d, FeatureId::chest);
    addFeatureFoundIn(*d, FeatureId::cabinet);
    addFeatureFoundIn(*d, FeatureId::cocoon);
    data[int(d->id)] = d;

    d = new ItemDataT(ItemId::spikeGun);
    resetData(*d, ItemType::rangedWpn);
    d->baseName = {"Spike Gun", "Spike Guns", "a Spike Gun"};
    d->baseDescr =
    {
        "A very strange and crude weapon capable of launching iron spikes with enough "
        "force to pierce flesh (or even rock). It seems almost to be deliberately "
        "designed for cruelty, rather than pure stopping power."
    };
    d->weight = ItemWeight::medium;
    d->tile = TileId::tommyGun;
    d->clr = clrBlueLgt;
    d->melee.attMsgs = {"strike", "strikes me with a Spike Gun"};
    d->ranged.isMachineGun = false;
    d->ranged.hitChanceMod = 0;
    d->ranged.dmg = DiceParam(1, 7, 0);
    d->ranged.effectiveRange = 4;
    d->ranged.dmgType = DmgType::physical;
    d->ranged.knocksBack = true;
    d->ranged.ammoItemId = ItemId::ironSpike;
    d->ranged.attMsgs = {"fire", "fires a Spike Gun"};
    d->ranged.sndMsg = "I hear a very crude weapon being fired.";
    d->ranged.makesRicochetSnd = true;
    d->ranged.missileGlyph = '/';
    d->ranged.missileClr = clrGray;
    d->spawnStdRange.lower = 4;
    d->ranged.attSfx = SfxId::spikeGun;
    d->ranged.sndVol = SndVol::low;
    addFeatureFoundIn(*d, FeatureId::chest, 50);
    addFeatureFoundIn(*d, FeatureId::cabinet, 50);
    addFeatureFoundIn(*d, FeatureId::cocoon, 50);
    data[int(d->id)] = d;

    d = new ItemDataT(ItemId::dynamite);
    resetData(*d, ItemType::explosive);
    d->baseName = {"Dynamite", "Sticks of Dynamite", "a Stick of Dynamite"};
    d->baseDescr =
    {
        "An explosive material based on nitroglycerin. The name comes from the ancient "
        "Greek word for \"power\"."
    };
    d->weight = ItemWeight::light;
    d->tile = TileId::dynamite;
    d->clr = clrRedLgt;
    addFeatureFoundIn(*d, FeatureId::chest);
    addFeatureFoundIn(*d, FeatureId::cabinet);
    addFeatureFoundIn(*d, FeatureId::cocoon);
    data[int(d->id)] = d;

    d = new ItemDataT(ItemId::flare);
    resetData(*d, ItemType::explosive);
    d->baseName = {"Flare", "Flares", "a Flare"};
    d->baseDescr =
    {
        "A type of pyrotechnic that produces a brilliant light or intense heat without "
        "an explosion."
    };
    d->weight = ItemWeight::light;
    d->tile = TileId::flare;
    d->clr = clrGray;
    addFeatureFoundIn(*d, FeatureId::chest);
    addFeatureFoundIn(*d, FeatureId::cabinet);
    addFeatureFoundIn(*d, FeatureId::cocoon);
    data[int(d->id)] = d;

    d = new ItemDataT(ItemId::molotov);
    resetData(*d, ItemType::explosive);
    d->baseName = {"Molotov Cocktail", "Molotov Cocktails", "a Molotov Cocktail"};
    d->baseDescr =
    {
        "An improvised incendiary weapon made of a glass bottle containing flammable "
        "liquid and some cloth for ignition. In action, the cloth is lit and the bottle "
        "hurled at a target, causing an immediate fireball followed by a raging fire."
    };
    d->weight = ItemWeight::light;
    d->tile = TileId::molotov;
    d->clr = clrWhite;
    addFeatureFoundIn(*d, FeatureId::chest);
    addFeatureFoundIn(*d, FeatureId::cabinet);
    addFeatureFoundIn(*d, FeatureId::cocoon);
    data[int(d->id)] = d;

    d = new ItemDataT(ItemId::smokeGrenade);
    resetData(*d, ItemType::explosive);
    d->baseName = {"Smoke Grenade", "Smoke Grenades", "a Smoke Grenade"};
    d->baseDescr =
    {
        "A sheet steel cylinder with emission holes releasing smoke when the grenade is "
        "ignited. Their primary use is to create smoke screens for concealment. "
        "The fumes produced can harm the eyes, throat and lungs  - so it is "
        "recommended to wear a protective mask."
    };
    d->weight = ItemWeight::light;
    d->tile = TileId::flare;
    d->clr = clrGreen;
    addFeatureFoundIn(*d, FeatureId::chest);
    addFeatureFoundIn(*d, FeatureId::cabinet);
    data[int(d->id)] = d;

    d = new ItemDataT(ItemId::thrKnife);
    resetData(*d, ItemType::throwingWpn);
    d->baseName = {"Throwing Knife", "Throwing Knives", "a Throwing Knife"};
    d->baseDescr =
    {
        "A knife specially designed and weighted so that it can be thrown effectively."
    };
    d->weight = ItemWeight::extraLight;
    d->tile = TileId::dagger;
    d->glyph = '/';
    d->clr = clrWhite;
    d->ranged.throwHitChanceMod = 0;
    d->ranged.throwDmg = DiceParam(2, 4);
    d->ranged.effectiveRange = 5;
    d->maxStackAtSpawn = 8;
    d->landOnHardSndMsg = "I hear a clanking sound.";
    d->landOnHardSfx = SfxId::metalClank;
    d->mainAttMode = MainAttMode::thrown;
    addFeatureFoundIn(*d, FeatureId::chest);
    addFeatureFoundIn(*d, FeatureId::cabinet);
    addFeatureFoundIn(*d, FeatureId::cocoon);
    data[int(d->id)] = d;

    d = new ItemDataT(ItemId::rock);
    resetData(*d, ItemType::throwingWpn);
    d->baseName = {"Rock", "Rocks", "a Rock"};
    d->baseDescr =
    {
        "Although not a very impressive weapon, with skill they can be used with some "
        "result."
    };
    d->weight = ItemWeight::extraLight;
    d->tile = TileId::rock;
    d->glyph = '*';
    d->clr = clrGray;
    d->ranged.throwHitChanceMod = 10;
    d->ranged.throwDmg = DiceParam(1, 3);
    d->ranged.effectiveRange = 4;
    d->maxStackAtSpawn = 6;
    d->mainAttMode = MainAttMode::thrown;
    addFeatureFoundIn(*d, FeatureId::cabinet);
    addFeatureFoundIn(*d, FeatureId::cocoon);
    data[int(d->id)] = d;

    d = new ItemDataT(ItemId::dagger);
    resetData(*d, ItemType::meleeWpn);
    d->baseName = {"Dagger", "Daggers", "a Dagger"};
    d->baseDescr =
    {
        "Commonly associated with deception, stealth, and treachery. Many "
        "assassinations have been carried out with the use of a dagger.",

        "Attacking an unaware opponent with a dagger does 300% damage (instead of the "
        "normal 150% damage from stealth attacks).",

        "Melee attacks with daggers are silent."
    };
    d->weight = ItemWeight::light;
    d->tile = TileId::dagger;
    d->melee.attMsgs = {"stab", "stabs me with a Dagger"};
    d->melee.dmg = pair<int, int>(1, 4);
    d->melee.hitChanceMod = 20;
    d->melee.hitMediumSfx = SfxId::hitSharp;
    d->melee.hitHardSfx = SfxId::hitSharp;
    d->melee.missSfx = SfxId::missLight;
    addFeatureFoundIn(*d, FeatureId::chest);
    addFeatureFoundIn(*d, FeatureId::cabinet);
    addFeatureFoundIn(*d, FeatureId::tomb);
    addFeatureFoundIn(*d, FeatureId::cocoon);
    data[int(d->id)] = d;

    d = new ItemDataT(ItemId::hatchet);
    resetData(*d, ItemType::meleeWpn);
    d->baseName = {"Hatchet", "Hatchets", "a Hatchet"};
    d->baseDescr =
    {
        "A small axe with a short handle. Hatchets are reliable weapons - they are easy "
        "to use, and cause decent damage for their low weight. "
        /*TODO: "They can also serve well as thrown weapons."*/,

        "Melee attacks with hatchets are silent."
    };
    d->weight = ItemWeight::light;
    d->tile = TileId::axe;
    d->melee.attMsgs = {"strike", "strikes me with a Hatchet"};
    d->melee.dmg = pair<int, int>(1, 5);
    d->melee.hitChanceMod = 15;
    d->ranged.throwHitChanceMod = -5;
    d->ranged.throwDmg = DiceParam(1, 10);
    d->ranged.isThrowingWpn = false;
    d->melee.hitMediumSfx = SfxId::hitSharp;
    d->melee.hitHardSfx = SfxId::hitSharp;
    d->melee.missSfx = SfxId::missLight;
    addFeatureFoundIn(*d, FeatureId::chest);
    addFeatureFoundIn(*d, FeatureId::cabinet);
    addFeatureFoundIn(*d, FeatureId::cocoon);
    data[int(d->id)] = d;

    d = new ItemDataT(ItemId::club);
    resetData(*d, ItemType::meleeWpn);
    d->baseName = {"Club", "Clubs", "a Club"};
    d->baseDescr =
    {
        "Wielded since prehistoric times.",

        "Melee attacks with clubs are noisy."
    };
    d->spawnStdRange = Range(DLVL_FIRST_LATE_GAME, INT_MAX);
    d->weight = ItemWeight::medium;
    d->tile = TileId::club;
    d->clr = clrBrown;
    d->melee.attMsgs = {"strike", "strikes me with a Club"};
    d->melee.dmg = pair<int, int>(2, 3);
    d->melee.hitChanceMod = 10;
    d->melee.missSfx = SfxId::missMedium;
    data[int(d->id)] = d;

    d = new ItemDataT(ItemId::hammer);
    resetData(*d, ItemType::meleeWpn);
    d->baseName = {"Hammer", "Hammers", "a Hammer"};
    d->baseDescr =
    {
        "Typically used for construction, but can be quite devastating when wielded as "
        "a weapon.",

        "Melee attacks with hammers are noisy."
    };
    d->weight = ItemWeight::medium;
    d->tile = TileId::hammer;
    d->melee.attMsgs = {"strike", "strikes me with a Hammer"};
    d->melee.dmg = pair<int, int>(2, 4);
    d->melee.hitChanceMod = 5;
    d->melee.missSfx = SfxId::missMedium;
    addFeatureFoundIn(*d, FeatureId::cabinet);
    addFeatureFoundIn(*d, FeatureId::cocoon);
    data[int(d->id)] = d;

    d = new ItemDataT(ItemId::machete);
    resetData(*d, ItemType::meleeWpn);
    d->baseName = {"Machete", "Machetes", "a Machete"};
    d->baseDescr =
    {
        "A large cleaver-like knife. It serves well both as a cutting tool and weapon.",

        "Melee attacks with machetes are noisy."
    };
    d->weight = ItemWeight::medium;
    d->tile = TileId::machete;
    d->melee.attMsgs = {"strike", "strikes me with a Machete"};
    d->melee.dmg = pair<int, int>(2, 5);
    d->melee.hitChanceMod = 0;
    d->melee.hitSmallSfx = SfxId::hitSharp;
    d->melee.hitMediumSfx = SfxId::hitSharp;
    d->melee.missSfx = SfxId::missMedium;
    addFeatureFoundIn(*d, FeatureId::cabinet);
    addFeatureFoundIn(*d, FeatureId::cocoon);
    data[int(d->id)] = d;

    d = new ItemDataT(ItemId::axe);
    resetData(*d, ItemType::meleeWpn);
    d->baseName = {"Axe", "Axes", "an Axe"};
    d->baseDescr =
    {
        "A tool intended for felling trees, splitting timber, etc. Used as a weapon it "
        "can deliver devastating blows, although it requires some skill to use "
        "effectively. Also effective for breaching wooden doors.",

        "Melee attacks with axes are noisy."
    };
    d->weight = ItemWeight::medium;
    d->tile = TileId::axe;
    d->melee.attMsgs = {"strike", "strikes me with an axe"};
    d->melee.dmg = pair<int, int>(2, 6);
    d->melee.hitChanceMod = -5;
    d->melee.hitSmallSfx = SfxId::hitSmall;
    d->melee.hitMediumSfx = SfxId::hitMedium;
    d->melee.hitHardSfx = SfxId::hitHard;
    d->melee.missSfx = SfxId::missMedium;
    addFeatureFoundIn(*d, FeatureId::cabinet);
    addFeatureFoundIn(*d, FeatureId::tomb);
    addFeatureFoundIn(*d, FeatureId::cocoon);
    data[int(d->id)] = d;

    d = new ItemDataT(ItemId::pitchFork);
    resetData(*d, ItemType::meleeWpn);
    d->baseName = {"Pitchfork", "Pitchforks", "a Pitchfork"};
    d->baseDescr =
    {
        "A long staff with a forked, four-pronged end. Victims can be pushed away when "
        "stabbed, to keep them at bay."
    };
    d->weight = ItemWeight::heavy;
    d->tile = TileId::pitchfork;
    d->melee.attMsgs = {"strike", "strikes me with a Pitchfork"};
    d->melee.dmg = pair<int, int>(3, 4);
    d->melee.hitChanceMod = -5;
    d->melee.knocksBack = true;
    d->melee.hitSmallSfx = SfxId::hitSharp;
    d->melee.hitMediumSfx = SfxId::hitSharp;
    d->melee.missSfx = SfxId::missHeavy;
    addFeatureFoundIn(*d, FeatureId::cabinet);
    addFeatureFoundIn(*d, FeatureId::cocoon);
    data[int(d->id)] = d;

    d = new ItemDataT(ItemId::sledgeHammer);
    resetData(*d, ItemType::meleeWpn);
    d->baseName = {"Sledgehammer", "Sledgehammers", "a Sledgehammer"};
    d->baseDescr =
    {
        "Often used in destruction work for breaking through walls. It can deal a great "
        "amount of damage, although it is cumbersome to carry, and it requires some "
        "skill to use effectively."
    };
    d->weight = ItemWeight::heavy;
    d->tile = TileId::sledgeHammer;
    d->melee.attMsgs = {"strike", "strikes me with a Sledgehammer"};
    d->melee.dmg = pair<int, int>(3, 5);
    d->melee.hitChanceMod = -10;
    d->melee.knocksBack = true;
    d->melee.missSfx = SfxId::missHeavy;
    addFeatureFoundIn(*d, FeatureId::cabinet);
    data[int(d->id)] = d;

    d = new ItemDataT(ItemId::pharaohStaff);
    resetData(*d, ItemType::meleeWpn);
    d->baseName =
    {
        "Staff of the Pharaohs", "Staff of the Pharaohs", "the Staff of the Pharaohs"
    };
    d->baseDescr =
    {
        "Once wielded by long-forgotten kings in ancient times, this powerful artifact "
        "grants the power to call up a loyal servant from the dead."
    };
    d->clr = clrMagenta;
    d->weight = ItemWeight::medium;
    d->tile = TileId::pharaohStaff;
    d->melee.attMsgs = {"strike", "strikes me with the Staff of the Pharaohs"};
    d->melee.dmg = pair<int, int>(2, 4);
    d->melee.hitChanceMod = 0;
    d->melee.missSfx = SfxId::missMedium;
    d->chanceToIncludeInFloorSpawnList = 1;
    d->value = ItemValue::majorTreasure;
    addFeatureFoundIn(*d, FeatureId::tomb, 20);
    data[int(d->id)] = d;

    d = new ItemDataT(ItemId::ironSpike);
    resetData(*d, ItemType::throwingWpn);
    d->baseName = {"Iron Spike", "Iron Spikes", "an Iron Spike"};
    d->baseDescr =
    {
        "Can be useful for wedging things closed."
        /*TODO: or prying things open."*/
    };
    d->weight = ItemWeight::extraLight;
    d->tile = TileId::ironSpike;
    d->isStackable = true;
    d->clr = clrGray;
    d->glyph = '/';
    d->ranged.throwHitChanceMod = -5;
    d->ranged.throwDmg = DiceParam(1, 3);
    d->ranged.effectiveRange = 3;
    d->maxStackAtSpawn = 12;
    d->landOnHardSndMsg = "I hear a clanking sound.";
    d->landOnHardSfx = SfxId::metalClank;
    d->mainAttMode = MainAttMode::thrown;
    addFeatureFoundIn(*d, FeatureId::cabinet);
    addFeatureFoundIn(*d, FeatureId::cocoon);
    data[int(d->id)] = d;

    d = new ItemDataT(ItemId::playerKick);
    resetData(*d, ItemType::meleeWpnIntr);
    d->melee.attMsgs = {"kick", ""};
    d->melee.hitChanceMod = 20;
    d->melee.dmg = pair<int, int>(1, 3);
    d->melee.knocksBack = true;
    d->melee.missSfx = SfxId::missMedium;
    data[int(d->id)] = d;

    d = new ItemDataT(ItemId::playerStomp);
    resetData(*d, ItemType::meleeWpnIntr);
    d->melee.attMsgs = {"stomp", ""};
    d->melee.hitChanceMod = 20;
    d->melee.dmg = pair<int, int>(1, 3);
    d->melee.knocksBack = false;
    data[int(d->id)] = d;

    d = new ItemDataT(ItemId::playerPunch);
    resetData(*d, ItemType::meleeWpnIntr);
    d->melee.attMsgs = {"punch", ""};
    d->melee.hitChanceMod = 25;
    d->melee.dmg = pair<int, int>(1, 2);
    data[int(d->id)] = d;

    d = new ItemDataT(ItemId::zombieClaw);
    resetData(*d, ItemType::meleeWpnIntr);
    d->melee.attMsgs = {"", "claws me"};
    setDmgFromMonId(*d, ActorId::zombie);
    data[int(d->id)] = d;

    d = new ItemDataT(ItemId::zombieClawDiseased);
    resetData(*d, ItemType::meleeWpnIntr);
    d->melee.attMsgs = {"", "claws me"};
    setDmgFromMonId(*d, ActorId::zombie);
    d->melee.propApplied = new PropInfected(PropTurns::std);
    data[int(d->id)] = d;

    d = new ItemDataT(ItemId::zombieAxe);
    resetData(*d, ItemType::meleeWpnIntr);
    d->melee.attMsgs = {"", "chops me with a rusty axe"};
    d->melee.hitSmallSfx = SfxId::hitSmall;
    d->melee.hitMediumSfx = SfxId::hitMedium;
    d->melee.hitHardSfx = SfxId::hitHard;
    setDmgFromMonId(*d, ActorId::zombieAxe);
    data[int(d->id)] = d;

    d = new ItemDataT(ItemId::bloatedZombiePunch);
    resetData(*d, ItemType::meleeWpnIntr);
    d->melee.attMsgs = {"", "mauls me"};
    setDmgFromMonId(*d, ActorId::bloatedZombie);
    d->melee.knocksBack = true;
    data[int(d->id)] = d;

    d = new ItemDataT(ItemId::bloatedZombieSpit);
    resetData(*d, ItemType::rangedWpnIntr);
    d->ranged.attMsgs = {"", "spits acid pus at me"};
    setDmgFromMonId(*d, ActorId::bloatedZombie);
    d->ranged.sndMsg = "I hear spitting.";
    d->ranged.missileClr = clrGreenLgt;
    d->ranged.dmgType = DmgType::acid;
    d->ranged.missileGlyph = '*';
    data[int(d->id)] = d;

    d = new ItemDataT(ItemId::ratBite);
    resetData(*d, ItemType::meleeWpnIntr);
    d->melee.attMsgs = {"", "bites me"};
    setDmgFromMonId(*d, ActorId::rat);
    data[int(d->id)] = d;

    d = new ItemDataT(ItemId::ratBiteDiseased);
    resetData(*d, ItemType::meleeWpnIntr);
    d->melee.attMsgs = {"", "bites me"};
    setDmgFromMonId(*d, ActorId::rat);
    d->melee.propApplied = new PropInfected(PropTurns::std);
    data[int(d->id)] = d;

    d = new ItemDataT(ItemId::ratThingBite);
    resetData(*d, ItemType::meleeWpnIntr);
    d->melee.attMsgs = {"", "bites me"};
    setDmgFromMonId(*d, ActorId::ratThing);
    data[int(d->id)] = d;

    d = new ItemDataT(ItemId::brownJenkinBite);
    resetData(*d, ItemType::meleeWpnIntr);
    d->melee.attMsgs = {"", "bites me"};
    setDmgFromMonId(*d, ActorId::brownJenkin);
    data[int(d->id)] = d;

    d = new ItemDataT(ItemId::wormMassBite);
    resetData(*d, ItemType::meleeWpnIntr);
    d->melee.attMsgs = {"", "bites me"};
    setDmgFromMonId(*d, ActorId::wormMass);
    data[int(d->id)] = d;

    d = new ItemDataT(ItemId::wolfBite);
    resetData(*d, ItemType::meleeWpnIntr);
    d->melee.attMsgs = {"", "bites me"};
    setDmgFromMonId(*d, ActorId::wolf);
    data[int(d->id)] = d;

    d = new ItemDataT(ItemId::greenSpiderBite);
    resetData(*d, ItemType::meleeWpnIntr);
    d->melee.attMsgs = {"", "bites me"};
    setDmgFromMonId(*d, ActorId::greenSpider);
    d->melee.propApplied = new PropBlind(PropTurns::specific, 4);
    data[int(d->id)] = d;

    d = new ItemDataT(ItemId::whiteSpiderBite);
    resetData(*d, ItemType::meleeWpnIntr);
    d->melee.attMsgs = {"", "bites me"};
    setDmgFromMonId(*d, ActorId::whiteSpider);
    d->melee.propApplied = new PropParalyzed(PropTurns::specific, 2);
    data[int(d->id)] = d;

    d = new ItemDataT(ItemId::redSpiderBite);
    resetData(*d, ItemType::meleeWpnIntr);
    d->melee.attMsgs = {"", "bites me"};
    setDmgFromMonId(*d, ActorId::redSpider);
    d->melee.propApplied = new PropWeakened(PropTurns::std);
    data[int(d->id)] = d;

    d = new ItemDataT(ItemId::shadowSpiderBite);
    resetData(*d, ItemType::meleeWpnIntr);
    d->melee.attMsgs = {"", "bites me"};
    setDmgFromMonId(*d, ActorId::shadowSpider);
    data[int(d->id)] = d;

    d = new ItemDataT(ItemId::lengSpiderBite);
    resetData(*d, ItemType::meleeWpnIntr);
    d->melee.attMsgs = {"", "bites me"};
    setDmgFromMonId(*d, ActorId::lengSpider);
    data[int(d->id)] = d;

    d = new ItemDataT(ItemId::fireHoundBreath);
    resetData(*d, ItemType::rangedWpnIntr);
    d->ranged.attMsgs = {"", "breaths fire at me"};
    d->ranged.sndMsg = "I hear a burst of flames.";
    setDmgFromMonId(*d, ActorId::fireHound);
    d->ranged.propApplied = new PropBurning(PropTurns::std);
    d->ranged.missileClr = clrRedLgt;
    d->ranged.missileGlyph = '*';
    d->ranged.missileLeavesTrail = true;
    d->ranged.missileLeavesSmoke = true;
    d->ranged.dmgType = DmgType::fire;
    data[int(d->id)] = d;

    d = new ItemDataT(ItemId::fireHoundBite);
    resetData(*d, ItemType::meleeWpnIntr);
    d->melee.attMsgs = {"", "bites me"};
    setDmgFromMonId(*d, ActorId::fireHound);
    d->melee.dmgType = DmgType::fire;
    data[int(d->id)] = d;

    d = new ItemDataT(ItemId::frostHoundBreath);
    resetData(*d, ItemType::rangedWpnIntr);
    d->ranged.attMsgs = {"", "breaths frost at me"};
    d->ranged.sndMsg = "I hear a chilling sound.";
    setDmgFromMonId(*d, ActorId::frostHound);
    d->ranged.missileClr = clrBlueLgt;
    d->ranged.missileGlyph = '*';
    d->ranged.missileLeavesTrail = true;
    d->ranged.missileLeavesSmoke = true;
    d->ranged.dmgType = DmgType::cold;
    data[int(d->id)] = d;

    d = new ItemDataT(ItemId::frostHoundBite);
    resetData(*d, ItemType::meleeWpnIntr);
    d->melee.attMsgs = {"", "bites me"};
    setDmgFromMonId(*d, ActorId::frostHound);
    d->melee.dmgType = DmgType::cold;
    data[int(d->id)] = d;

    d = new ItemDataT(ItemId::zuulBite);
    resetData(*d, ItemType::meleeWpnIntr);
    d->melee.attMsgs = {"", "bites me"};
    setDmgFromMonId(*d, ActorId::zuul);
    d->melee.dmgType = DmgType::physical;
    data[int(d->id)] = d;

    d = new ItemDataT(ItemId::dustVortexEngulf);
    resetData(*d, ItemType::meleeWpnIntr);
    d->melee.attMsgs = {"", "engulfs me"};
    setDmgFromMonId(*d, ActorId::dustVortex);
    d->melee.propApplied = new PropBlind(PropTurns::std);
    data[int(d->id)] = d;

    d = new ItemDataT(ItemId::fireVortexEngulf);
    resetData(*d, ItemType::meleeWpnIntr);
    d->melee.attMsgs = {"", "engulfs me"};
    setDmgFromMonId(*d, ActorId::fireVortex);
    d->melee.propApplied = new PropBurning(PropTurns::std);
    data[int(d->id)] = d;

    d = new ItemDataT(ItemId::frostVortexEngulf);
    resetData(*d, ItemType::meleeWpnIntr);
    d->melee.attMsgs = {"", "engulfs me"};
    setDmgFromMonId(*d, ActorId::frostVortex);
    data[int(d->id)] = d;

    d = new ItemDataT(ItemId::ghostClaw);
    resetData(*d, ItemType::meleeWpnIntr);
    d->melee.attMsgs = {"", "claws me"};
    setDmgFromMonId(*d, ActorId::ghost);
    d->melee.propApplied = new PropTerrified(PropTurns::specific, 4);
    d->melee.dmgType = DmgType::spirit;
    data[int(d->id)] = d;

    d = new ItemDataT(ItemId::phantasmSickle);
    resetData(*d, ItemType::meleeWpnIntr);
    d->melee.attMsgs = {"", "slices me with a sickle"};
    setDmgFromMonId(*d, ActorId::phantasm);
    d->melee.propApplied = new PropTerrified(PropTurns::specific, 4);
    d->melee.dmgType = DmgType::spirit;
    data[int(d->id)] = d;

    d = new ItemDataT(ItemId::wraithClaw);
    resetData(*d, ItemType::meleeWpnIntr);
    d->melee.attMsgs = {"", "claws me"};
    setDmgFromMonId(*d, ActorId::wraith);
    d->melee.propApplied = new PropTerrified(PropTurns::specific, 4);
    d->melee.dmgType = DmgType::spirit;
    data[int(d->id)] = d;

    d = new ItemDataT(ItemId::giantBatBite);
    resetData(*d, ItemType::meleeWpnIntr);
    d->melee.attMsgs = {"", "bites me"};
    setDmgFromMonId(*d, ActorId::giantBat);
    data[int(d->id)] = d;

    d = new ItemDataT(ItemId::polypTentacle);
    resetData(*d, ItemType::meleeWpnIntr);
    d->melee.attMsgs = {"", "grips me with a tentacle"};
    d->melee.propApplied = new PropParalyzed(PropTurns::specific, 1);
    setDmgFromMonId(*d, ActorId::flyingPolyp);
    data[int(d->id)] = d;

    d = new ItemDataT(ItemId::greaterPolypTentacle);
    resetData(*d, ItemType::meleeWpnIntr);
    d->melee.attMsgs = {"", "grips me with a tentacle"};
    d->melee.propApplied = new PropParalyzed(PropTurns::specific, 1);
    setDmgFromMonId(*d, ActorId::greaterPolyp);
    data[int(d->id)] = d;

    d = new ItemDataT(ItemId::ghoulClaw);
    resetData(*d, ItemType::meleeWpnIntr);
    d->melee.attMsgs = {"", "claws me"};
    setDmgFromMonId(*d, ActorId::ghoul);
    d->melee.propApplied = new PropInfected(PropTurns::std);
    data[int(d->id)] = d;

    d = new ItemDataT(ItemId::shadowClaw);
    resetData(*d, ItemType::meleeWpnIntr);
    d->melee.attMsgs = {"", "claws me"};
    setDmgFromMonId(*d, ActorId::shadow);
    data[int(d->id)] = d;

    d = new ItemDataT(ItemId::byakheeClaw);
    resetData(*d, ItemType::meleeWpnIntr);
    d->melee.attMsgs = {"", "claws me"};
    setDmgFromMonId(*d, ActorId::byakhee);
    data[int(d->id)] = d;

    d = new ItemDataT(ItemId::giantMantisClaw);
    resetData(*d, ItemType::meleeWpnIntr);
    d->melee.attMsgs = {"", "claws me"};
    setDmgFromMonId(*d, ActorId::giantMantis);
    data[int(d->id)] = d;

    d = new ItemDataT(ItemId::giantLocustBite);
    resetData(*d, ItemType::meleeWpnIntr);
    d->melee.attMsgs = {"", "bites me"};
    setDmgFromMonId(*d, ActorId::locust);
    data[int(d->id)] = d;

    d = new ItemDataT(ItemId::mummyMaul);
    resetData(*d, ItemType::meleeWpnIntr);
    d->melee.attMsgs = {"", "mauls me"};
    setDmgFromMonId(*d, ActorId::mummy);
    d->melee.propApplied = new PropCursed(PropTurns::std);
    d->melee.knocksBack = true;
    data[int(d->id)] = d;

    d = new ItemDataT(ItemId::crocHeadMummySpear);
    resetData(*d, ItemType::meleeWpnIntr);
    d->melee.attMsgs = {"", "hits me with a spear"};
    setDmgFromMonId(*d, ActorId::crocHeadMummy);
    data[int(d->id)] = d;

    d = new ItemDataT(ItemId::deepOneJavelinAtt);
    resetData(*d, ItemType::rangedWpnIntr);
    d->ranged.attMsgs = {"", "throws a javelin at me"};
    setDmgFromMonId(*d, ActorId::deepOne);
    d->ranged.sndMsg = "";
    d->ranged.missileClr = clrBrown;
    d->ranged.missileGlyph = '/';
    d->ranged.sndVol = SndVol::low;
    data[int(d->id)] = d;

    d = new ItemDataT(ItemId::deepOneSpearAtt);
    resetData(*d, ItemType::meleeWpnIntr);
    d->melee.attMsgs = {"", "hits me with a spear"};
    setDmgFromMonId(*d, ActorId::deepOne);
    data[int(d->id)] = d;

    d = new ItemDataT(ItemId::apeMaul);
    resetData(*d, ItemType::meleeWpnIntr);
    d->melee.attMsgs = {"", "mauls me"};
    setDmgFromMonId(*d, ActorId::ape);
    data[int(d->id)] = d;

    d = new ItemDataT(ItemId::oozeBlackSpewPus);
    resetData(*d, ItemType::meleeWpnIntr);
    d->melee.attMsgs = {"", "spews pus on me"};
    setDmgFromMonId(*d, ActorId::oozeBlack);
    data[int(d->id)] = d;

    d = new ItemDataT(ItemId::oozeClearSpewPus);
    resetData(*d, ItemType::meleeWpnIntr);
    d->melee.attMsgs = {"", "spews pus on me"};
    setDmgFromMonId(*d, ActorId::oozeClear);
    data[int(d->id)] = d;

    d = new ItemDataT(ItemId::oozePutridSpewPus);
    resetData(*d, ItemType::meleeWpnIntr);
    d->melee.attMsgs = {"", "spews infected pus on me"};
    setDmgFromMonId(*d, ActorId::oozePutrid);
    d->melee.propApplied = new PropInfected(PropTurns::std);
    data[int(d->id)] = d;

    d = new ItemDataT(ItemId::oozePoisonSpewPus);
    resetData(*d, ItemType::meleeWpnIntr);
    d->melee.attMsgs = {"", "spews poisonous pus on me"};
    setDmgFromMonId(*d, ActorId::oozePoison);
    d->melee.propApplied = new PropPoisoned(PropTurns::std);
    data[int(d->id)] = d;

    d = new ItemDataT(ItemId::colourOOSpaceTouch);
    resetData(*d, ItemType::meleeWpnIntr);
    d->melee.attMsgs = {"", "touches me"};
    setDmgFromMonId(*d, ActorId::colourOOSpace);
    data[int(d->id)] = d;

    d = new ItemDataT(ItemId::chthonianBite);
    resetData(*d, ItemType::meleeWpnIntr);
    d->melee.attMsgs = {"", "strikes me with a tentacle"};
    d->melee.knocksBack = true;
    setDmgFromMonId(*d, ActorId::chthonian);
    data[int(d->id)] = d;

    d = new ItemDataT(ItemId::huntingHorrorBite);
    resetData(*d, ItemType::meleeWpnIntr);
    d->melee.attMsgs = {"", "bites me"};
    setDmgFromMonId(*d, ActorId::huntingHorror);
    d->melee.propApplied = new PropPoisoned(PropTurns::std);
    data[int(d->id)] = d;

    d = new ItemDataT(ItemId::moldSpores);
    resetData(*d, ItemType::meleeWpnIntr);
    d->melee.attMsgs = {"", "releases spores at me"};
    setDmgFromMonId(*d, ActorId::mold);
    d->melee.propApplied = new PropPoisoned(PropTurns::specific, POISON_DMG_N_TURN * 2);
    data[int(d->id)] = d;

    d = new ItemDataT(ItemId::miGoSting);
    resetData(*d, ItemType::meleeWpnIntr);
    d->melee.attMsgs = {"", "stings me"};
    setDmgFromMonId(*d, ActorId::miGo);
    d->melee.propApplied = new PropPoisoned(PropTurns::specific, POISON_DMG_N_TURN * 2);
    data[int(d->id)] = d;

    d = new ItemDataT(ItemId::miGoCommanderSting);
    resetData(*d, ItemType::meleeWpnIntr);
    d->melee.attMsgs = {"", "stings me"};
    setDmgFromMonId(*d, ActorId::miGoCommander);
    d->melee.propApplied = new PropPoisoned(PropTurns::specific, POISON_DMG_N_TURN * 2);
    data[int(d->id)] = d;

    d = new ItemDataT(ItemId::theDarkOneClaw);
    resetData(*d, ItemType::meleeWpnIntr);
    d->melee.attMsgs = {"", "claws me"};
    d->melee.propApplied = new PropTerrified(PropTurns::std);
    setDmgFromMonId(*d, ActorId::theDarkOne);
    data[int(d->id)] = d;

    d = new ItemDataT(ItemId::armorLeatherJacket);
    resetData(*d, ItemType::armor);
    d->baseName = {"Leather Jacket", "", "a Leather Jacket"};
    d->baseDescr =
    {
        "It offers some protection."
    };
    d->weight = ItemWeight::light;
    d->clr = clrBrown;
    d->spawnStdRange.lower = 1;
    d->armor.armorPoints = 1;
    d->armor.dmgToDurabilityFactor = 1.0;
    d->landOnHardSndMsg = "";
    addFeatureFoundIn(*d, FeatureId::cabinet);
    data[int(d->id)] = d;

    d = new ItemDataT(ItemId::armorIronSuit);
    resetData(*d, ItemType::armor);
    d->baseName = {"Iron Suit", "", "an Iron Suit"};
    d->baseDescr =
    {
        "A crude armour constructed from metal plates, bolts, and leather straps.",

        "It can absorb a high amount of damage, but it makes sneaking and dodging very "
        "difficult. Also, due to the narrow slit of the helmet, aiming is slightly "
        "more difficult, and it is harder to detect sneaking enemies and hidden objects."
    };
    d->abilityModsWhileEquipped[int(AbilityId::stealth)]    = -50;
    d->abilityModsWhileEquipped[int(AbilityId::dodgeAtt)]   = -50;
    d->abilityModsWhileEquipped[int(AbilityId::dodgeTrap)]  = -50;
    d->abilityModsWhileEquipped[int(AbilityId::melee)]      = -10;
    d->abilityModsWhileEquipped[int(AbilityId::ranged)]     = -10;
    d->abilityModsWhileEquipped[int(AbilityId::searching)]  = -6;
    d->weight = ItemWeight::heavy;
    d->clr = clrWhite;
    d->spawnStdRange.lower = 2;
    d->armor.armorPoints = 5;
    d->armor.dmgToDurabilityFactor = 0.3;
    d->landOnHardSndMsg = "I hear a crashing sound.";
    addFeatureFoundIn(*d, FeatureId::cabinet);
    data[int(d->id)] = d;

    d = new ItemDataT(ItemId::armorFlackJacket);
    resetData(*d, ItemType::armor);
    d->baseName = {"Flak Jacket", "", "a Flak Jacket"};
    d->baseDescr =
    {
        "An armour consisting of steel plates sewn into a waistcoat. It offers very good "
        "protection for its weight. Sneaking and dodging is slightly more difficult."
    };
    d->abilityModsWhileEquipped[int(AbilityId::stealth)]    = -20;
    d->abilityModsWhileEquipped[int(AbilityId::dodgeAtt)]   = -20;
    d->abilityModsWhileEquipped[int(AbilityId::dodgeTrap)]  = -20;
    d->weight = ItemWeight::medium;
    d->clr = clrGreen;
    d->spawnStdRange.lower = 3;
    d->armor.armorPoints = 3;
    d->armor.dmgToDurabilityFactor = 0.5;
    d->landOnHardSndMsg = "I hear a thudding sound.";
    addFeatureFoundIn(*d, FeatureId::cabinet);
    data[int(d->id)] = d;

    d = new ItemDataT(ItemId::armorAsbSuit);
    resetData(*d, ItemType::armor);
    d->baseName = {"Asbestos Suit", "", "an Asbestos Suit"};
    d->baseDescr =
    {
        "A one piece overall of asbestos fabric, including a hood, furnace mask, gloves "
        "and shoes. It protects the wearer against fire, acid and electricity, and also "
        "against smoke, fumes and gas.",

        "It is a bit bulky, so sneaking and dodging is slightly more difficult. Also, "
        "because of the hood and mask, aiming and detecting hidden enemies and objects "
        "is somewhat harder."
    };
    d->abilityModsWhileEquipped[int(AbilityId::stealth)]    = -20;
    d->abilityModsWhileEquipped[int(AbilityId::dodgeAtt)]   = -20;
    d->abilityModsWhileEquipped[int(AbilityId::dodgeTrap)]  = -20;
    d->abilityModsWhileEquipped[int(AbilityId::melee)]      = -10;
    d->abilityModsWhileEquipped[int(AbilityId::ranged)]     = -10;
    d->abilityModsWhileEquipped[int(AbilityId::searching)]  = -6;
    d->weight = ItemWeight::medium;
    d->clr = clrRedLgt;
    d->spawnStdRange.lower = 3;
    d->armor.armorPoints = 1;
    d->armor.dmgToDurabilityFactor = 1.0;
    d->landOnHardSndMsg = "";
    addFeatureFoundIn(*d, FeatureId::cabinet);
    addFeatureFoundIn(*d, FeatureId::chest);
    data[int(d->id)] = d;

    d = new ItemDataT(ItemId::armorHeavyCoat);
    resetData(*d, ItemType::armor);
    d->baseName = {"Heavy Coat", "", "a Heavy Coat"};
    d->baseDescr =
    {
        "A very thick coat. It gives more protection against physical harm than a "
        "leather jacket, and it also protects against cold.",

        "Sneaking and dodging is slightly more difficult due to its higher weight."
    };
    d->abilityModsWhileEquipped[int(AbilityId::stealth)]    = -15;
    d->abilityModsWhileEquipped[int(AbilityId::dodgeAtt)]   = -15;
    d->abilityModsWhileEquipped[int(AbilityId::dodgeTrap)]  = -15;
    d->weight = ItemWeight::medium;
    d->clr = clrBlueLgt;
    d->spawnStdRange.lower = 3;
    d->armor.armorPoints = 2;
    d->armor.dmgToDurabilityFactor = 1.0;
    d->landOnHardSndMsg = "";
    addFeatureFoundIn(*d, FeatureId::cabinet);
    data[int(d->id)] = d;

    d = new ItemDataT(ItemId::armorMiGo);
    resetData(*d, ItemType::armor);
    d->baseName = {"Mi-go Bio-armor", "", "a Mi-go Bio-armor"};
    d->baseDescr =
    {
        "A self-repairing biological armor created by the Mi-go.",

        "It is very disturbing for a human to wear (+15% shock while worn)."
    };
    d->spawnStdRange = Range(-1, -1);
    d->weight = ItemWeight::medium;
    d->shockWhileEquipped = 15;
    d->clr = clrMagenta;
    d->tile = TileId::miGoArmor;
    d->armor.armorPoints = 2;
    d->armor.dmgToDurabilityFactor = 1.5;
    d->landOnHardSndMsg = "";
    data[int(d->id)] = d;

    d = new ItemDataT(ItemId::gasMask);
    resetData(*d, ItemType::headWear);
    d->baseName = {"Gas Mask", "", "a Gas Mask"};
    d->baseDescr =
    {
        "Protects the eyes, throat and lungs from smoke and fumes. It has a limited "
        "useful lifespan that is related to the absorbent capacity of the filter.",

        "Due to the small eye windows, aiming is slightly more difficult, and it is "
        "harder to detect sneaking enemies and hidden objects."
    };
    d->abilityModsWhileEquipped[int(AbilityId::melee)]      = -10;
    d->abilityModsWhileEquipped[int(AbilityId::ranged)]     = -10;
    d->abilityModsWhileEquipped[int(AbilityId::searching)]  = -6;
    d->isStackable = false;
    d->clr = clrBrown;
    d->tile = TileId::mask;
    d->glyph = '[';
    d->spawnStdRange = Range(1, DLVL_LAST_EARLY_GAME);
    d->chanceToIncludeInFloorSpawnList = 50;
    d->weight = ItemWeight::light;
    d->landOnHardSndMsg = "";
    data[int(d->id)] = d;

    d = new ItemDataT(ItemId::hideousMask);
    resetData(*d, ItemType::headWear);
    d->baseName = {"Hideous Mask", "", "The Hideous Mask"};
    d->baseDescr =
    {
        "[TODO]"
    };
    d->isStackable = false;
    d->clr = clrMagenta;
    d->tile = TileId::mask;
    d->glyph = '[';
    d->spawnStdRange = Range(-1, -1);
    d->weight = ItemWeight::light;
    d->landOnHardSndMsg = "";
    d->chanceToIncludeInFloorSpawnList = 1;
    d->value = ItemValue::majorTreasure;
    addFeatureFoundIn(*d, FeatureId::tomb, 8);
    data[int(d->id)] = d;

    d = new ItemDataT(ItemId::scrollMayhem);
    resetData(*d, ItemType::scroll);
    d->spellCastFromScroll = SpellId::mayhem;
    data[int(d->id)] = d;

    d = new ItemDataT(ItemId::scrollTelep);
    resetData(*d, ItemType::scroll);
    d->spellCastFromScroll = SpellId::teleport;
    data[int(d->id)] = d;

    d = new ItemDataT(ItemId::scrollPest);
    resetData(*d, ItemType::scroll);
    d->spellCastFromScroll = SpellId::pest;
    data[int(d->id)] = d;

    d = new ItemDataT(ItemId::scrollSlowMon);
    resetData(*d, ItemType::scroll);
    d->spellCastFromScroll = SpellId::slowMon;
    data[int(d->id)] = d;

    d = new ItemDataT(ItemId::scrollTerrifyMon);
    resetData(*d, ItemType::scroll);
    d->spellCastFromScroll = SpellId::terrifyMon;
    data[int(d->id)] = d;

    d = new ItemDataT(ItemId::scrollParalMon);
    resetData(*d, ItemType::scroll);
    d->spellCastFromScroll = SpellId::paralyzeMon;
    data[int(d->id)] = d;

    d = new ItemDataT(ItemId::scrollDetItems);
    resetData(*d, ItemType::scroll);
    d->spellCastFromScroll = SpellId::detItems;
    data[int(d->id)] = d;

    d = new ItemDataT(ItemId::scrollDetTraps);
    resetData(*d, ItemType::scroll);
    d->spellCastFromScroll = SpellId::detTraps;
    data[int(d->id)] = d;

    d = new ItemDataT(ItemId::scrollBless);
    resetData(*d, ItemType::scroll);
    d->spellCastFromScroll = SpellId::bless;
    data[int(d->id)] = d;

    d = new ItemDataT(ItemId::scrollDarkbolt);
    resetData(*d, ItemType::scroll);
    d->spellCastFromScroll = SpellId::darkbolt;
    data[int(d->id)] = d;

    d = new ItemDataT(ItemId::scrollAzaWrath);
    resetData(*d, ItemType::scroll);
    d->spellCastFromScroll = SpellId::azaWrath;
    data[int(d->id)] = d;

    d = new ItemDataT(ItemId::scrollOpening);
    resetData(*d, ItemType::scroll);
    d->spellCastFromScroll = SpellId::opening;
    data[int(d->id)] = d;

    d = new ItemDataT(ItemId::scrollSacrLife);
    resetData(*d, ItemType::scroll);
    d->spellCastFromScroll = SpellId::sacrLife;
    data[int(d->id)] = d;

    d = new ItemDataT(ItemId::scrollSacrSpi);
    resetData(*d, ItemType::scroll);
    d->spellCastFromScroll = SpellId::sacrSpi;
    data[int(d->id)] = d;

    d = new ItemDataT(ItemId::scrollDetMon);
    resetData(*d, ItemType::scroll);
    d->spellCastFromScroll = SpellId::detMon;
    data[int(d->id)] = d;

    d = new ItemDataT(ItemId::scrollElemRes);
    resetData(*d, ItemType::scroll);
    d->spellCastFromScroll = SpellId::elemRes;
    data[int(d->id)] = d;

    d = new ItemDataT(ItemId::scrollSummonMon);
    resetData(*d, ItemType::scroll);
    d->spellCastFromScroll = SpellId::summonMon;
    data[int(d->id)] = d;

    d = new ItemDataT(ItemId::scrollLight);
    resetData(*d, ItemType::scroll);
    d->spellCastFromScroll = SpellId::light;
    data[int(d->id)] = d;

    d = new ItemDataT(ItemId::potionVitality);
    resetData(*d, ItemType::potion);
    data[int(d->id)] = d;

    d = new ItemDataT(ItemId::potionSpirit);
    resetData(*d, ItemType::potion);
    data[int(d->id)] = d;

    d = new ItemDataT(ItemId::potionBlindness);
    resetData(*d, ItemType::potion);
    data[int(d->id)] = d;

    d = new ItemDataT(ItemId::potionFrenzy);
    resetData(*d, ItemType::potion);
    data[int(d->id)] = d;

    d = new ItemDataT(ItemId::potionFortitude);
    resetData(*d, ItemType::potion);
    data[int(d->id)] = d;

    d = new ItemDataT(ItemId::potionParalyze);
    resetData(*d, ItemType::potion);
    data[int(d->id)] = d;

    d = new ItemDataT(ItemId::potionRElec);
    resetData(*d, ItemType::potion);
    data[int(d->id)] = d;

    d = new ItemDataT(ItemId::potionConf);
    resetData(*d, ItemType::potion);
    data[int(d->id)] = d;

    d = new ItemDataT(ItemId::potionPoison);
    resetData(*d, ItemType::potion);
    data[int(d->id)] = d;

    d = new ItemDataT(ItemId::potionInsight);
    resetData(*d, ItemType::potion);
    data[int(d->id)] = d;

    d = new ItemDataT(ItemId::potionClairv);
    resetData(*d, ItemType::potion);
    data[int(d->id)] = d;

    d = new ItemDataT(ItemId::potionRFire);
    resetData(*d, ItemType::potion);
    data[int(d->id)] = d;

    d = new ItemDataT(ItemId::potionAntidote);
    resetData(*d, ItemType::potion);
    data[int(d->id)] = d;

    d = new ItemDataT(ItemId::potionDescent);
    resetData(*d, ItemType::potion);
    data[int(d->id)] = d;

    d = new ItemDataT(ItemId::deviceBlaster);
    resetData(*d, ItemType::device);
    d->baseName = {"Blaster Device", "Blaster Devices", "a Blaster Device"};
    d->value = ItemValue::minorTreasure;
    d->clr = clrGray;
    addFeatureFoundIn(*d, FeatureId::chest, 10);
    addFeatureFoundIn(*d, FeatureId::tomb, 10);
    addFeatureFoundIn(*d, FeatureId::cocoon, 10);
    data[int(d->id)] = d;

    d = new ItemDataT(ItemId::deviceShockwave);
    resetData(*d, ItemType::device);
    d->baseName =
    {
        "Shock Wave Device", "Shock Wave Devices", "a Shock Wave Device"
    };
    d->value = ItemValue::minorTreasure;
    d->clr = clrGray;
    addFeatureFoundIn(*d, FeatureId::chest, 10);
    addFeatureFoundIn(*d, FeatureId::tomb, 10);
    addFeatureFoundIn(*d, FeatureId::cocoon, 10);
    data[int(d->id)] = d;

    d = new ItemDataT(ItemId::deviceRejuvenator);
    resetData(*d, ItemType::device);
    d->baseName =
    {
        "Rejuvenator Device", "Rejuvenator Devices", "a Rejuvenator Device"
    };
    d->value = ItemValue::minorTreasure;
    d->clr = clrGray;
    addFeatureFoundIn(*d, FeatureId::chest, 10);
    addFeatureFoundIn(*d, FeatureId::tomb, 10);
    addFeatureFoundIn(*d, FeatureId::cocoon, 10);
    data[int(d->id)] = d;

    d = new ItemDataT(ItemId::deviceTranslocator);
    resetData(*d, ItemType::device);
    d->baseName =
    {
        "Translocator Device", "Translocator Devices", "a Translocator Device"
    };
    d->value = ItemValue::minorTreasure;
    d->clr = clrGray;
    addFeatureFoundIn(*d, FeatureId::chest, 10);
    addFeatureFoundIn(*d, FeatureId::tomb, 10);
    addFeatureFoundIn(*d, FeatureId::cocoon, 10);
    data[int(d->id)] = d;

    d = new ItemDataT(ItemId::deviceSentryDrone);
    resetData(*d, ItemType::device);
    d->baseName =
    {
        "Sentry Drone Device", "Sentry Drone Devices", "a Sentry Drone Device"
    };
    d->value = ItemValue::minorTreasure;
    d->clr = clrGray;
    addFeatureFoundIn(*d, FeatureId::chest, 10);
    addFeatureFoundIn(*d, FeatureId::tomb, 10);
    addFeatureFoundIn(*d, FeatureId::cocoon, 10);
    data[int(d->id)] = d;

    d = new ItemDataT(ItemId::electricLantern);
    resetData(*d, ItemType::device);
    d->baseName = {"Electric Lantern", "Electric Lanterns", "an Electric Lantern"};
    d->baseDescr =
    {
        "A portable light source. It is somewhat unreliable as it tends to flicker and "
        "malfunction often."
    };
    d->spawnStdRange = Range(1, DLVL_LAST_MID_GAME);
    d->spawnStdRange = Range(1, 10);
    d->chanceToIncludeInFloorSpawnList = 50;
    d->isIdentified = true;
    d->tile = TileId::electricLantern;
    d->clr = clrYellow;
    addFeatureFoundIn(*d, FeatureId::chest);
    addFeatureFoundIn(*d, FeatureId::cabinet);
    addFeatureFoundIn(*d, FeatureId::cocoon);
    data[int(d->id)] = d;

    d = new ItemDataT(ItemId::medicalBag);
    resetData(*d, ItemType::general);
    d->baseName = {"Medical Bag", "Medical Bags", "a Medical Bag"};
    d->baseDescr =
    {
        "A portable bag of medical supplies."
    };
    d->weight = ItemWeight::medium;
    d->spawnStdRange = Range(1, DLVL_LAST_MID_GAME);
    d->isStackable = false;
    d->glyph = '~';
    d->clr = clrBrownDrk;
    d->tile = TileId::medicalBag;
    data[int(d->id)] = d;

    d = new ItemDataT(ItemId::starAmulet);
    resetData(*d, ItemType::amulet);
    d->baseName = {"Star Amulet", "", "a Star Amulet"};
    d->baseNameUnId = d->baseName;
    d->clr = clrWhite;
    data[int(d->id)] = d;

    d = new ItemDataT(ItemId::skullAmulet);
    resetData(*d, ItemType::amulet);
    d->baseName = {"Skull Amulet", "", "a Skull Amulet"};
    d->baseNameUnId = d->baseName;
    d->clr = clrWhite;
    data[int(d->id)] = d;

    d = new ItemDataT(ItemId::spiderAmulet);
    resetData(*d, ItemType::amulet);
    d->baseName = {"Spider Amulet", "", "a Spider Amulet"};
    d->baseNameUnId = d->baseName;
    d->clr = clrWhite;
    data[int(d->id)] = d;

    d = new ItemDataT(ItemId::eyeAmulet);
    resetData(*d, ItemType::amulet);
    d->baseName = {"Eye Amulet", "", "an Eye Amulet"};
    d->baseNameUnId = d->baseName;
    d->clr = clrWhite;
    data[int(d->id)] = d;

    d = new ItemDataT(ItemId::moonAmulet);
    resetData(*d, ItemType::amulet);
    d->baseName = {"Moon Amulet", "", "a Moon Amulet"};
    d->baseNameUnId = d->baseName;
    d->clr = clrWhite;
    data[int(d->id)] = d;

    d = new ItemDataT(ItemId::batAmulet);
    resetData(*d, ItemType::amulet);
    d->baseName = {"Bat Amulet", "", "a Bat Amulet"};
    d->baseNameUnId = d->baseName;
    d->clr = clrWhite;
    data[int(d->id)] = d;

    d = new ItemDataT(ItemId::scarabAmulet);
    resetData(*d, ItemType::amulet);
    d->baseName = {"Scarab Amulet", "", "a Scarab Amulet"};
    d->baseNameUnId = d->baseName;
    d->clr = clrWhite;
    data[int(d->id)] = d;

    d = new ItemDataT(ItemId::daggerAmulet);
    resetData(*d, ItemType::amulet);
    d->baseName = {"Dagger Amulet", "", "a Dagger Amulet"};
    d->baseNameUnId = d->baseName;
    d->clr = clrWhite;
    data[int(d->id)] = d;

    d = new ItemDataT(ItemId::goldenRing);
    resetData(*d, ItemType::ring);
    d->baseName = {"Golden Ring", "", "a Golden Ring"};
    d->baseNameUnId = d->baseName;
    d->clr = clrYellow;
    data[int(d->id)] = d;

    d = new ItemDataT(ItemId::silverRing);
    resetData(*d, ItemType::ring);
    d->baseName = {"Silver Ring", "", "a Silver Ring"};
    d->baseNameUnId = d->baseName;
    d->clr = clrWhite;
    data[int(d->id)] = d;

    d = new ItemDataT(ItemId::carnelianRing);
    resetData(*d, ItemType::ring);
    d->baseName = {"Carnelian Ring", "", "a Carnelian Ring"};
    d->baseNameUnId = d->baseName;
    d->clr = clrRedLgt;
    data[int(d->id)] = d;

    d = new ItemDataT(ItemId::garnetRing);
    resetData(*d, ItemType::ring);
    d->baseName = {"Garnet Ring", "", "a Garnet Ring"};
    d->baseNameUnId = d->baseName;
    d->clr = clrRedLgt;
    data[int(d->id)] = d;

    d = new ItemDataT(ItemId::ironRing);
    resetData(*d, ItemType::ring);
    d->baseName = {"Iron Ring", "", "an Iron Ring"};
    d->baseNameUnId = d->baseName;
    d->clr = clrGray;
    data[int(d->id)] = d;

    d = new ItemDataT(ItemId::jadeRing);
    resetData(*d, ItemType::ring);
    d->baseName = {"Jade Ring", "", "a Jade Ring"};
    d->baseNameUnId = d->baseName;
    d->clr = clrGreenLgt;
    data[int(d->id)] = d;

    d = new ItemDataT(ItemId::moonstoneRing);
    resetData(*d, ItemType::ring);
    d->baseName = {"Moonstone Ring", "", "a Moonstone Ring"};
    d->baseNameUnId = d->baseName;
    d->clr = clrBlueLgt;
    data[int(d->id)] = d;

    d = new ItemDataT(ItemId::obsidianRing);
    resetData(*d, ItemType::ring);
    d->baseName = {"Obsidian Ring", "", "an Obsidian Ring"};
    d->baseNameUnId = d->baseName;
    d->clr = clrGray;
    data[int(d->id)] = d;

    d = new ItemDataT(ItemId::onyxRing);
    resetData(*d, ItemType::ring);
    d->baseName = {"Onyx Ring", "", "an Onyx Ring"};
    d->baseNameUnId = d->baseName;
    d->clr = clrGray;
    data[int(d->id)] = d;

    d = new ItemDataT(ItemId::topazRing);
    resetData(*d, ItemType::ring);
    d->baseName = {"Topaz Ring", "", "a Topaz Ring"};
    d->baseNameUnId = d->baseName;
    d->clr = clrBlueLgt;
    data[int(d->id)] = d;

    d = new ItemDataT(ItemId::emeraldRing);
    resetData(*d, ItemType::ring);
    d->baseName = {"Emerald Ring", "", "an Emerald Ring"};
    d->baseNameUnId = d->baseName;
    d->clr = clrGreenLgt;
    data[int(d->id)] = d;
}

} //namespace

void init()
{
    TRACE_FUNC_BEGIN;
    initDataList();
    TRACE_FUNC_END;
}

void cleanup()
{
    TRACE_FUNC_BEGIN;
    for (size_t i = 0; i < int(ItemId::END); ++i) {delete data[i];}
    TRACE_FUNC_END;
}


void storeToSaveLines(vector<string>& lines)
{
    for (int i = 0; i < int(ItemId::END); ++i)
    {
        lines.push_back(data[i]->isIdentified ? "1" : "0");
        lines.push_back(data[i]->allowSpawn   ? "1" : "0");

        if (
            data[i]->type == ItemType::scroll ||
            data[i]->type == ItemType::potion)
        {
            lines.push_back(data[i]->isTried ? "1" : "0");
        }
    }
}

void setupFromSaveLines(vector<string>& lines)
{
    for (int i = 0; i < int(ItemId::END); ++i)
    {
        data[i]->isIdentified = lines.front() == "0" ? false : true;
        lines.erase(begin(lines));

        data[i]->allowSpawn = lines.front()   == "0" ? false : true;
        lines.erase(begin(lines));

        if (
            data[i]->type == ItemType::scroll ||
            data[i]->type == ItemType::potion)
        {
            data[i]->isTried = lines.front() == "0" ? false : true;
            lines.erase(begin(lines));
        }
    }
}

//TODO: Remove this function
bool isWpnStronger(const ItemDataT& data1, const ItemDataT& data2,
                   const bool IS_MELEE)
{
    (void)data1;
    (void)data2;
    (void)IS_MELEE;

    return false;
}

} //ItemData
