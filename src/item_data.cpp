#include "item_data.hpp"

#include <iostream>
#include <climits>

#include "init.hpp"
#include "colors.hpp"
#include "item.hpp"
#include "actor_data.hpp"
#include "actor_player.hpp"
#include "sound.hpp"
#include "item_device.hpp"
#include "map.hpp"
#include "saving.hpp"
#include "game_time.hpp"

ItemDataT::ItemDataT() :
    id                                  (ItemId::END),
    type                                (ItemType::general),
    has_std_activate                    (false),
    is_prio_in_backpack_list            (false),
    value                               (ItemValue::normal),
    weight                              (ItemWeight::none),
    allow_spawn                         (true),
    spawn_std_range                     (Range(1, dlvl_last)),
    max_stack_at_spawn                  (1),
    chance_to_incl_in_spawn_list        (100),
    is_stackable                        (true),
    is_identified                       (true),
    is_tried                            (false),
    xp_on_identify                      (0),
    base_name                           (),
    glyph                               ('X'),
    clr                                 (clr_white),
    tile                                (TileId::empty),
    main_att_mode                       (AttMode::none),
    spell_cast_from_scroll              (SpellId::END),
    land_on_hard_snd_msg                ("I hear a thudding sound."),
    land_on_hard_sfx                    (SfxId::END),
    is_ins_raied_while_carried          (false),
    is_ins_raied_while_equiped          (false),
    melee                               (ItemMeleeData()),
    ranged                              (ItemRangedData()),
    armor                               (ItemArmorData())
{
    for (size_t i = 0; i < size_t(AbilityId::END); ++i)
    {
        ability_mods_while_equipped[i] = 0;
    }

    base_descr.clear();
    native_rooms.clear();
    native_containers.clear();
}

ItemDataT::ItemMeleeData::ItemMeleeData() :
    is_melee_wpn                        (false),
    dmg                                 (),
    hit_chance_mod                      (0),
    att_msgs                            (ItemAttMsgs()),
    prop_applied                        (nullptr),
    dmg_type                            (DmgType::physical),
    knocks_back                         (false),
    hit_small_sfx                       (SfxId::END),
    hit_medium_sfx                      (SfxId::END),
    hit_hard_sfx                        (SfxId::END),
    miss_sfx                            (SfxId::END) {}

ItemDataT::ItemMeleeData::~ItemMeleeData()
{
    if (prop_applied)
    {
        delete prop_applied;
    }
}

ItemDataT::ItemRangedData::ItemRangedData() :
    is_ranged_wpn                       (false),
    is_throwable_wpn                    (false),
    is_machine_gun                      (false),
    is_shotgun                          (false),
    max_ammo                            (0),
    dmg                                 (),
    throw_dmg                           (),
    hit_chance_mod                      (0),
    throw_hit_chance_mod                (0),
    effective_range                     (3),
    knocks_back                         (false),
    dmg_info_override                   (""),
    ammo_item_id                        (ItemId::END),
    dmg_type                            (DmgType::physical),
    has_infinite_ammo                   (false),
    projectile_glyph                    ('/'),
    projectile_tile                     (TileId::projectile_std_front_slash),
    projectile_clr                      (clr_white),
    projectile_leaves_trail             (false),
    projectile_leaves_smoke             (false),
    att_msgs                            (ItemAttMsgs()),
    snd_msg                             (""),
    snd_vol                             (SndVol::low),
    makes_ricochet_snd                  (false),
    att_sfx                             (SfxId::END),
    reload_sfx                          (SfxId::END),
    prop_applied                        (nullptr) {}

ItemDataT::ItemRangedData::~ItemRangedData()
{
    if (prop_applied)
    {
        delete prop_applied;
    }
}

ItemDataT::ItemArmorData::ItemArmorData() :
    armor_points                (0),
    dmg_to_durability_factor    (0.0) {}

namespace item_data
{

ItemDataT data[(size_t)ItemId::END];

namespace
{

void mod_spawn_chance(ItemDataT& data, const double factor)
{
    TRACE << "Name: " << data.base_name.names[(size_t)ItemRefType::plain] << std::endl;
    TRACE << "Chance before: " << data.chance_to_incl_in_spawn_list << std::endl;

    data.chance_to_incl_in_spawn_list =
        (int)((double)data.chance_to_incl_in_spawn_list * factor);

    TRACE << "Chance after: " << data.chance_to_incl_in_spawn_list << std::endl;
}

// -----------------------------------------------------------------------------
// Item archetypes (defaults)
// -----------------------------------------------------------------------------
void reset_data(ItemDataT& d, ItemType const item_type)
{
    switch (item_type)
    {
    case ItemType::general:
        d = ItemDataT();
        break;

    case ItemType::melee_wpn:
        reset_data(d, ItemType::general);
        d.type = ItemType::melee_wpn;
        d.is_stackable = false;
        d.weight = ItemWeight::medium;
        d.glyph = ')';
        d.clr = clr_white;
        d.main_att_mode = AttMode::melee;
        d.melee.is_melee_wpn = true;
        d.melee.miss_sfx = SfxId::miss_medium;
        d.melee.hit_small_sfx = SfxId::hit_small;
        d.melee.hit_medium_sfx = SfxId::hit_medium;
        d.melee.hit_hard_sfx = SfxId::hit_hard;
        d.ranged.is_throwable_wpn = true;
        d.land_on_hard_snd_msg = "I hear a clanking sound.";
        d.land_on_hard_sfx = SfxId::metal_clank;
        break;

    case ItemType::melee_wpn_intr:
        reset_data(d, ItemType::melee_wpn);
        d.type = ItemType::melee_wpn_intr;
        d.spawn_std_range = Range(-1, -1);
        d.chance_to_incl_in_spawn_list = 0;
        d.allow_spawn = false;
        d.melee.hit_small_sfx = SfxId::hit_small;
        d.melee.hit_medium_sfx = SfxId::hit_medium;
        d.melee.hit_hard_sfx = SfxId::hit_hard;
        d.melee.miss_sfx = SfxId::END;
        d.ranged.is_throwable_wpn = false;
        break;

    case ItemType::ranged_wpn:
        reset_data(d, ItemType::general);
        d.type = ItemType::ranged_wpn;
        d.is_stackable = false;
        d.weight = ItemWeight::medium;
        d.glyph = '}';
        d.clr = clr_white;
        d.melee.is_melee_wpn = true;
        d.melee.dmg = DiceParam(1, 4);
        d.main_att_mode = AttMode::ranged;
        d.ranged.is_ranged_wpn = true;
        d.ranged.projectile_glyph = '/';
        d.ranged.projectile_clr = clr_white;
        d.spawn_std_range.max = dlvl_last_mid_game;
        d.melee.hit_small_sfx = SfxId::hit_small;
        d.melee.hit_medium_sfx = SfxId::hit_medium;
        d.melee.hit_hard_sfx = SfxId::hit_hard;
        d.melee.miss_sfx = SfxId::miss_medium;
        d.ranged.snd_vol = SndVol::high;
        break;

    case ItemType::ranged_wpn_intr:
        reset_data(d, ItemType::ranged_wpn);
        d.type = ItemType::ranged_wpn_intr;
        d.ranged.has_infinite_ammo = true;
        d.spawn_std_range = Range(-1, -1);
        d.chance_to_incl_in_spawn_list = 0;
        d.allow_spawn = false;
        d.melee.is_melee_wpn = false;
        d.ranged.projectile_glyph = '*';
        d.ranged.snd_vol = SndVol::low;
        break;

    case ItemType::throwing_wpn:
        reset_data(d, ItemType::general);
        d.type = ItemType::throwing_wpn;
        d.weight = ItemWeight::extra_light;
        d.is_stackable = true;
        d.spawn_std_range.max = dlvl_last_mid_game;
        d.ranged.snd_vol = SndVol::low;
        d.ranged.is_throwable_wpn = true;
        break;

    case ItemType::ammo:
        reset_data(d, ItemType::general);
        d.type = ItemType::ammo;
        d.weight = ItemWeight::extra_light;
        d.glyph = '{';
        d.clr = clr_white;
        d.tile = TileId::ammo;
        d.spawn_std_range.max = dlvl_last_mid_game;
        break;

    case ItemType::ammo_mag:
        reset_data(d, ItemType::ammo);
        d.type = ItemType::ammo_mag;
        d.weight = ItemWeight::light;
        d.is_stackable = false;
        d.spawn_std_range.max = dlvl_last_mid_game;
        break;

    case ItemType::scroll:
        reset_data(d, ItemType::general);
        d.type = ItemType::scroll;
        d.has_std_activate = true;
        d.base_descr =
        {
            "A short transcription of an eldritch incantation. There is a "
            "strange aura about it, as if some power was imbued in the paper "
            "itself.",
            "It should be possible to pronounce it correctly, but the purpose "
            "is unclear."
        };
        d.value = ItemValue::minor_treasure;
        d.chance_to_incl_in_spawn_list = 40;
        d.weight = ItemWeight::none;
        d.is_identified = false;
        d.xp_on_identify = 10;
        d.glyph = '?';
        d.clr = clr_white;
        d.tile = TileId::scroll;
        d.max_stack_at_spawn = 1;
        d.land_on_hard_snd_msg = "";
        d.native_containers.push_back(FeatureId::chest);
        d.native_containers.push_back(FeatureId::tomb);
        d.native_containers.push_back(FeatureId::cabinet);
        d.native_containers.push_back(FeatureId::cocoon);
        break;

    case ItemType::potion:
        reset_data(d, ItemType::general);
        d.type = ItemType::potion;
        d.has_std_activate = true;
        d.base_descr =
        {
            "A small glass bottle containing a mysterious concoction."
        };
        d.value = ItemValue::minor_treasure;
        d.chance_to_incl_in_spawn_list = 55;
        d.weight = ItemWeight::light;
        d.is_identified = false;
        d.xp_on_identify = 10;
        d.glyph = '!';
        d.tile = TileId::potion;
        d.ranged.throw_hit_chance_mod = 15;
        d.ranged.throw_dmg = DiceParam(1, 3, 0);
        d.max_stack_at_spawn = 2;
        d.land_on_hard_snd_msg = "";
        d.ranged.is_throwable_wpn = true;
        d.native_containers.push_back(FeatureId::chest);
        d.native_containers.push_back(FeatureId::tomb);
        d.native_containers.push_back(FeatureId::cabinet);
        d.native_containers.push_back(FeatureId::cocoon);
        break;

    case ItemType::device:
        reset_data(d, ItemType::general);
        d.type = ItemType::device;
        d.value = ItemValue::minor_treasure;
        d.is_ins_raied_while_carried = true;
        d.has_std_activate = true;
        d.base_name_un_id =
        {
            "Strange Device",
            "Strange Devices",
            "a Strange Device"
        };
        d.base_descr =
        {
            "A small piece of machinery. It could not possibly have been "
            "designed by a human mind. Even for its small size, it seems "
            "incredibly complex. There is no hope of understanding the purpose "
            "or function of it through normal means."
        };
        d.weight = ItemWeight::light;
        d.is_identified = false;
        d.xp_on_identify = 15;
        d.glyph = '%';
        d.tile = TileId::device1;
        d.is_stackable = false;
        d.land_on_hard_snd_msg = "I hear a clanking sound.";
        d.land_on_hard_sfx = SfxId::metal_clank;
        d.chance_to_incl_in_spawn_list = 20;
        d.native_containers.push_back(FeatureId::chest);
        d.native_containers.push_back(FeatureId::cocoon);
        break;

    case ItemType::rod:
        reset_data(d, ItemType::general);
        d.type = ItemType::rod;
        d.value = ItemValue::major_treasure;
        d.is_ins_raied_while_carried = true;
        d.has_std_activate = true;
        d.base_descr =
        {
            "A peculiar metallic device of cylindrical shape. The only detail "
            "is a single button on the side."
        };
        d.chance_to_incl_in_spawn_list = 9;
        d.weight = ItemWeight::light;
        d.is_identified = false;
        d.xp_on_identify = 15;
        d.glyph = '%';
        d.tile = TileId::rod;
        d.is_stackable = false;
        d.land_on_hard_snd_msg = "I hear a clanking sound.";
        d.land_on_hard_sfx = SfxId::metal_clank;
        d.chance_to_incl_in_spawn_list = 4;
        d.native_containers.push_back(FeatureId::chest);
        d.native_containers.push_back(FeatureId::cocoon);
        break;

    case ItemType::armor:
        reset_data(d, ItemType::general);
        d.type = ItemType::armor;
        d.weight = ItemWeight::heavy;
        d.glyph = '[';
        d.tile = TileId::armor;
        d.is_stackable = false;
        break;

    case ItemType::head_wear:
        reset_data(d, ItemType::general);
        d.type = ItemType::head_wear;
        d.glyph = '[';
        d.is_stackable = false;
        break;

    case ItemType::amulet:
        reset_data(d, ItemType::general);
        d.type = ItemType::amulet;
        d.value = ItemValue::major_treasure;
        d.is_ins_raied_while_carried = true;
        d.tile = TileId::amulet;
        d.glyph = '\"';
        d.weight = ItemWeight::light;
        d.is_identified = false;
        d.xp_on_identify = 15;
        d.is_stackable = false;
        d.chance_to_incl_in_spawn_list = 1;
        d.native_containers.push_back(FeatureId::tomb);
        d.native_containers.push_back(FeatureId::chest);
        break;

    case ItemType::explosive:
        reset_data(d, ItemType::general);
        d.type = ItemType::explosive;
        d.has_std_activate = true;
        d.weight = ItemWeight::light;
        d.glyph = '-';
        d.max_stack_at_spawn = 3;
        d.land_on_hard_snd_msg = "";
        break;

    default:
        break;
    }
}

void set_dmg_from_mon_id(ItemDataT& item_data, const ActorId id)
{
    const auto& actor_data = actor_data::data[(size_t)id];

    item_data.melee.dmg = DiceParam(1, actor_data.dmg_melee);

    item_data.ranged.dmg = DiceParam(1, actor_data.dmg_ranged);

    item_data.ranged.throw_dmg = DiceParam(1, actor_data.dmg_ranged);
}

// -----------------------------------------------------------------------------
// Item list
// -----------------------------------------------------------------------------
void init_data_list()
{
    ItemDataT d;

    reset_data(d, ItemType::general);
    d.id = ItemId::trapez;;
    d.base_name =
    {
        "Shining Trapezohedron", "Shining Trapezohedrons",
        "The Shining Trapezohedron"
    };
    d.spawn_std_range = Range(-1, -1);
    d.chance_to_incl_in_spawn_list = 0;
    d.allow_spawn = false;
    d.is_stackable = false;
    d.glyph = '*';
    d.clr = clr_red_lgt;
    d.tile = TileId::trapez;
    data[(size_t)d.id] = d;

    reset_data(d, ItemType::ranged_wpn);
    d.id = ItemId::sawed_off;
    d.base_name =
    {
        "Sawed-off Shotgun", "Sawed-off shotguns", "a Sawed-off Shotgun"
    };
    d.base_descr =
    {
        "Compared to a standard shotgun, the sawed - off has a shorter "
        "effective range. At close range it is more devastating however. "
        "It holds two barrels, and needs to be reloaded after both are "
        "discharged"
    };
    d.weight = ItemWeight::medium;
    d.tile = TileId::shotgun;
    d.ranged.is_shotgun = true;
    d.melee.att_msgs = {"strike", "strikes me with a shotgun"};
    d.ranged.max_ammo = 2;
    d.ranged.dmg = DiceParam(8, 3);
    d.ranged.hit_chance_mod = -5;
    d.ranged.effective_range = 3;
    d.ranged.ammo_item_id = ItemId::shotgun_shell;
    d.ranged.att_msgs = {"fire", "fires a shotgun"};
    d.ranged.snd_msg = "I hear a shotgun blast.";
    d.ranged.att_sfx = SfxId::shotgun_sawed_off_fire;
    d.ranged.makes_ricochet_snd = true;
    d.ranged.reload_sfx = SfxId::shotgun_reload;
    d.spawn_std_range.min = 2;
    d.native_containers.push_back(FeatureId::chest);
    d.native_containers.push_back(FeatureId::cabinet);
    d.native_containers.push_back(FeatureId::cocoon);
    data[(size_t)d.id] = d;

    reset_data(d, ItemType::ranged_wpn);
    d.id = ItemId::pump_shotgun;
    d.base_name = {"Pump Shotgun", "Pump shotguns", "a Pump Shotgun"};
    d.base_descr =
    {
        "A pump - action shotgun has a handgrip that can be pumped back and "
        "forth in order to eject a spent round of ammunition and to chamber a "
        "fresh one. It has a single barrel above a tube magazine into which "
        "shells are inserted. The magazine has a capacity of 8 shells."
    };
    d.weight = ItemWeight::medium;
    d.tile = TileId::shotgun;
    d.ranged.is_shotgun = true;
    d.melee.att_msgs = {"strike", "strikes me with a shotgun"};
    d.ranged.max_ammo = 8;
    d.ranged.dmg = DiceParam(6, 3);
    d.ranged.hit_chance_mod = 5;
    d.ranged.effective_range = 5;
    d.ranged.ammo_item_id = ItemId::shotgun_shell;
    d.ranged.att_msgs = {"fire", "fires a shotgun"};
    d.ranged.snd_msg = "I hear a shotgun blast.";
    d.ranged.att_sfx = SfxId::shotgun_pump_fire ;
    d.ranged.makes_ricochet_snd = true;
    d.ranged.reload_sfx = SfxId::shotgun_reload;
    d.spawn_std_range.min = 2;
    d.native_containers.push_back(FeatureId::chest);
    d.native_containers.push_back(FeatureId::cabinet);
    d.native_containers.push_back(FeatureId::cocoon);
    data[(size_t)d.id] = d;

    reset_data(d, ItemType::ammo);
    d.id = ItemId::shotgun_shell;
    d.base_name = {"Shotgun shell", "Shotgun shells", "a shotgun shell"};
    d.base_descr =
    {
        "A cartridge designed to be fired from a shotgun."
    };
    d.max_stack_at_spawn = 10;
    d.native_containers.push_back(FeatureId::chest);
    d.native_containers.push_back(FeatureId::cabinet);
    d.native_containers.push_back(FeatureId::cocoon);
    data[(size_t)d.id] = d;

    reset_data(d, ItemType::ranged_wpn);
    d.id = ItemId::incinerator;
    d.base_name = {"Incinerator", "Incinerators", "an Incinerator"};
    d.base_descr =
    {
        "This hellish, experimental weapon launches an explosive fireball. "
        "Best used with extreme caution."
    };
    d.weight = ItemWeight((ItemWeight::medium + ItemWeight::heavy) / 2);
    d.tile = TileId::incinerator;
    d.melee.att_msgs = {"strike", "strikes me with an Incinerator"};
    d.ranged.max_ammo = 5;
    d.ranged.dmg = DiceParam(1, 3);
    d.ranged.effective_range = 8;
    d.ranged.dmg_info_override = "* ";
    d.ranged.ammo_item_id = ItemId::incinerator_ammo;
    d.ranged.att_msgs = {"fire", "fires an incinerator"};
    d.ranged.snd_msg = "I hear the blast of a launched missile.";
    d.ranged.projectile_glyph = '*';
    d.ranged.projectile_clr = clr_red_lgt;
    d.ranged.reload_sfx = SfxId::machine_gun_reload;
    d.spawn_std_range.min = dlvl_first_mid_game;
    d.native_containers.push_back(FeatureId::chest);
    d.native_containers.push_back(FeatureId::cabinet);
    d.native_containers.push_back(FeatureId::cocoon);
    data[(size_t)d.id] = d;

    reset_data(d, ItemType::ammo_mag);
    d.id = ItemId::incinerator_ammo;
    d.base_name =
    {
        "Incinerator Cartridge", "Incinerator Cartridges",
        "an Incinerator Cartridge"
    };
    d.base_descr =
    {
        "Ammunition designed for Incinerators."
    };
    d.weight = ItemWeight::light;
    d.spawn_std_range.min = 5;
    d.max_stack_at_spawn = 1;
    d.ranged.max_ammo = data[(size_t)ItemId::incinerator].ranged.max_ammo;
    d.chance_to_incl_in_spawn_list = 25;
    d.native_containers.push_back(FeatureId::chest);
    d.native_containers.push_back(FeatureId::cabinet);
    d.native_containers.push_back(FeatureId::cocoon);
    data[(size_t)d.id] = d;

    reset_data(d, ItemType::ranged_wpn);
    d.id = ItemId::machine_gun;
    d.base_name = {"Tommy Gun", "Tommy Guns", "a Tommy Gun"};
    d.base_descr =
    {
        "\"Tommy Gun\" is a nickname for the Thompson submachine gun - an "
        "automatic firearm with a drum magazine and verical foregrip. It fires "
        ".45 ACP ammunition. The drum magazine has a capacity of 50 rounds."
    };
    d.weight = ItemWeight::medium;
    d.tile = TileId::tommy_gun;
    d.melee.att_msgs = {"strike", "strikes me with a Tommy Gun"};
    d.ranged.is_machine_gun = true;
    d.ranged.max_ammo = 50;
    d.ranged.dmg = DiceParam(2, 2, 2);
    d.ranged.hit_chance_mod = -10;
    d.ranged.effective_range = 8;
    d.ranged.ammo_item_id = ItemId::drum_of_bullets;
    d.ranged.att_msgs = {"fire", "fires a Tommy Gun"};
    d.ranged.snd_msg = "I hear the burst of a machine gun.";
    d.ranged.att_sfx = SfxId::machine_gun_fire;
    d.ranged.makes_ricochet_snd = true;
    d.ranged.reload_sfx = SfxId::machine_gun_reload;
    d.spawn_std_range.min = 2;
    d.native_containers.push_back(FeatureId::chest);
    d.native_containers.push_back(FeatureId::cabinet);
    d.native_containers.push_back(FeatureId::cocoon);
    data[(size_t)d.id] = d;

    reset_data(d, ItemType::ammo_mag);
    d.id = ItemId::drum_of_bullets;
    d.base_name = {"Drum of .45 ACP", "Drums of .45 ACP", "a Drum of .45 ACP"};
    d.base_descr =
    {
        "Ammunition used by Tommy Guns."
    };
    d.ranged.max_ammo = data[(size_t)ItemId::machine_gun].ranged.max_ammo;
    d.native_containers.push_back(FeatureId::chest);
    d.native_containers.push_back(FeatureId::cabinet);
    d.native_containers.push_back(FeatureId::cocoon);
    data[(size_t)d.id] = d;

    reset_data(d, ItemType::ranged_wpn);
    d.id = ItemId::pistol;
    d.base_name = {"M1911 Colt", "M1911 Colt", "an M1911 Colt"};
    d.base_descr =
    {
        "A semi-automatic, magazine-fed pistol chambered for the .45 ACP "
        "cartridge."
    };
    d.weight = ItemWeight((ItemWeight::light + ItemWeight::medium) / 2);
    d.tile = TileId::pistol;
    d.ranged.max_ammo = 7;
    d.ranged.dmg = DiceParam(1, 8, 4);
    d.ranged.effective_range = 6;
    d.ranged.ammo_item_id = ItemId::pistol_mag;
    d.melee.att_msgs = {"strike", "strikes me with a pistol"};
    d.ranged.att_msgs = {"fire", "fires a pistol"};
    d.ranged.snd_msg = "I hear a pistol being fired.";
    d.ranged.att_sfx = SfxId::pistol_fire;
    d.ranged.makes_ricochet_snd = true;
    d.ranged.reload_sfx = SfxId::pistol_reload;
    d.native_containers.push_back(FeatureId::chest);
    d.native_containers.push_back(FeatureId::cabinet);
    d.native_containers.push_back(FeatureId::cocoon);
    data[(size_t)d.id] = d;

    reset_data(d, ItemType::ammo_mag);
    d.id = ItemId::pistol_mag;
    d.base_name =
    {
        ".45ACP Colt cartridge", ".45ACP Colt cartridges",
        "a .45ACP Colt cartridge"
    };
    d.base_descr =
    {
        "Ammunition used by Colt pistols."
    };
    d.ranged.max_ammo = data[(size_t)ItemId::pistol].ranged.max_ammo;
    d.native_containers.push_back(FeatureId::chest);
    d.native_containers.push_back(FeatureId::cabinet);
    d.native_containers.push_back(FeatureId::cocoon);
    data[(size_t)d.id] = d;

    reset_data(d, ItemType::ranged_wpn);
    d.id = ItemId::mi_go_gun;
    d.base_name =
    {
        "Electric Gun", "Electric Gun", "an Electric Gun"
    };
    d.base_descr =
    {
        "A weapon created by the Mi-go. It fires devastating bolts of "
        "electricity.",

        "If there is no ammunition to fuel the weapon, it can draw power from "
        "the essence of the wielder (press [f] while no ammo loaded)."
    };
    d.spawn_std_range = Range(-1, -1);
    d.weight = ItemWeight::medium;
    d.tile = TileId::mi_go_gun;
    d.clr = clr_yellow;
    d.ranged.max_ammo = 16;
    d.ranged.dmg = DiceParam(3, 6, 0);
    d.ranged.hit_chance_mod = 5;
    d.ranged.effective_range = 4;
    d.ranged.prop_applied = new PropParalyzed(PropTurns::specific, 2);
    d.ranged.dmg_type = DmgType::electric;
    d.ranged.ammo_item_id = ItemId::mi_go_gun_ammo;
    d.ranged.has_infinite_ammo = false;
    d.ranged.projectile_leaves_trail = true;
    d.ranged.projectile_clr = clr_yellow;
    d.melee.att_msgs = {"strike", "strikes me with a Mi-go Electric Gun"};
    d.ranged.att_msgs = {"fire", "fires a Mi-go Electric Gun"};
    d.ranged.snd_msg = "I hear a bolt of electricity.";
    d.ranged.att_sfx = SfxId::mi_go_gun_fire;
    d.ranged.reload_sfx = SfxId::machine_gun_reload;
    d.ranged.makes_ricochet_snd = false;
    data[(size_t)d.id] = d;

    reset_data(d, ItemType::ammo_mag);
    d.id = ItemId::mi_go_gun_ammo;
    d.base_name =
    {
        "Mi-go Electric Cell", "Mi-go Electric Cells", "a Mi-go Electric Cell"
    };
    d.base_descr =
    {
        "Ammunition for the Mi-go Electric gun."
    };
    d.ranged.max_ammo = data[(size_t)ItemId::mi_go_gun].ranged.max_ammo;
    d.clr = clr_yellow;
    d.spawn_std_range = Range(-1, -1);
    data[(size_t)d.id] = d;

    reset_data(d, ItemType::ranged_wpn);
    d.id = ItemId::flare_gun;
    d.base_name = {"Flare Gun", "Flare Gun", "a Flare Gun"};
    d.base_descr =
    {
        "Launches flares. Not designed to function as a weapon."
    };
    d.weight = ItemWeight((ItemWeight::light + ItemWeight::medium) / 2);
    d.tile = TileId::flare_gun;
    d.ranged.max_ammo = 1;
    d.ranged.dmg = DiceParam(1, 3, 0);
    d.ranged.effective_range = 3;
    d.ranged.dmg_info_override = "*";
    d.ranged.ammo_item_id = ItemId::flare;
    d.melee.att_msgs = {"strike", "strikes me with a flare gun"};
    d.ranged.att_msgs = {"fire", "fires a flare gun"};
    d.ranged.snd_msg = "I hear a flare gun being fired.";
    d.ranged.prop_applied = new PropFlared(PropTurns::std);
    d.native_containers.push_back(FeatureId::chest);
    d.native_containers.push_back(FeatureId::cabinet);
    d.native_containers.push_back(FeatureId::cocoon);
    data[(size_t)d.id] = d;

    reset_data(d, ItemType::ranged_wpn);
    d.id = ItemId::spike_gun;
    d.base_name = {"Spike Gun", "Spike Guns", "a Spike Gun"};
    d.base_descr =
    {
        "A very strange and crude weapon capable of launching iron spikes "
        "with enough force to pierce flesh (or even rock). It seems almost to "
        "be deliberately designed for cruelty, rather than pure stopping power."
    };
    d.weight = ItemWeight::medium;
    d.tile = TileId::tommy_gun;
    d.clr = clr_brown;
    d.melee.att_msgs = {"strike", "strikes me with a Spike Gun"};
    d.ranged.max_ammo = 12;
    d.ranged.dmg = DiceParam(1, 7, 0);
    d.ranged.hit_chance_mod = 0;
    d.ranged.effective_range = 4;
    d.ranged.dmg_type = DmgType::physical;
    d.ranged.knocks_back = true;
    d.ranged.ammo_item_id = ItemId::iron_spike;
    d.ranged.att_msgs = {"fire", "fires a Spike Gun"};
    d.ranged.snd_msg = "I hear a very crude weapon being fired.";
    d.ranged.makes_ricochet_snd = true;
    d.ranged.projectile_clr = clr_gray;
    d.spawn_std_range.min = 4;
    d.ranged.att_sfx = SfxId::spike_gun;
    d.ranged.snd_vol = SndVol::low;
    d.native_containers.push_back(FeatureId::chest);
    d.native_containers.push_back(FeatureId::cabinet);
    d.native_containers.push_back(FeatureId::cocoon);
    data[(size_t)d.id] = d;

    reset_data(d, ItemType::ranged_wpn);
    d.id = ItemId::trap_dart;
    d.allow_spawn = false;
    d.ranged.has_infinite_ammo = true;
    d.ranged.dmg = DiceParam(1, 8);
    d.ranged.hit_chance_mod = 70;
    d.ranged.effective_range = 6;
    d.ranged.snd_msg = "I hear the launching of a projectile.";
    d.ranged.att_sfx = SfxId::END; //TODO: Make a sound effect for this
    d.ranged.makes_ricochet_snd = true;
    data[(size_t)d.id] = d;

    reset_data(d, ItemType::ranged_wpn);
    d = data[(size_t)ItemId::trap_dart];
    d.id = ItemId::trap_dart_poison;
    d.ranged.prop_applied = new PropPoisoned(PropTurns::std);
    data[(size_t)d.id] = d;

    reset_data(d, ItemType::melee_wpn);
    d.id = ItemId::trap_spear;
    d.allow_spawn = false;
    d.weight = ItemWeight::heavy;
    d.melee.dmg = DiceParam(2, 6);
    d.melee.hit_chance_mod = 85;
    d.melee.hit_small_sfx = SfxId::hit_sharp;
    d.melee.hit_medium_sfx = SfxId::hit_sharp;
    d.melee.miss_sfx = SfxId::miss_heavy;
    data[(size_t)d.id] = d;

    reset_data(d, ItemType::ranged_wpn);
    d = data[(size_t)ItemId::trap_spear];
    d.id = ItemId::trap_spear_poison;
    d.ranged.prop_applied = new PropPoisoned(PropTurns::std);
    data[(size_t)d.id] = d;

    reset_data(d, ItemType::explosive);
    d.id = ItemId::dynamite;
    d.base_name = {"Dynamite", "Sticks of Dynamite", "a Stick of Dynamite"};
    d.base_descr =
    {
        "An explosive material based on nitroglycerin. The name comes from the "
        "ancient Greek word for \"power\"."
    };
    d.weight = ItemWeight::light;
    d.tile = TileId::dynamite;
    d.clr = clr_red_lgt;
    d.native_containers.push_back(FeatureId::chest);
    d.native_containers.push_back(FeatureId::cabinet);
    d.native_containers.push_back(FeatureId::cocoon);
    data[(size_t)d.id] = d;

    reset_data(d, ItemType::explosive);
    d.id = ItemId::flare;
    d.base_name = {"Flare", "Flares", "a Flare"};
    d.base_descr =
    {
        "A type of pyrotechnic that produces a brilliant light or intense heat "
        "without an explosion."
    };
    d.weight = ItemWeight::light;
    d.tile = TileId::flare;
    d.clr = clr_gray;
    d.native_containers.push_back(FeatureId::chest);
    d.native_containers.push_back(FeatureId::cabinet);
    d.native_containers.push_back(FeatureId::cocoon);
    data[(size_t)d.id] = d;

    reset_data(d, ItemType::explosive);
    d.id = ItemId::molotov;
    d.base_name =
    {
        "Molotov Cocktail", "Molotov Cocktails", "a Molotov Cocktail"
    };
    d.base_descr =
    {
        "An improvised incendiary weapon made of a glass bottle containing "
        "flammable liquid and some cloth for ignition. In action, the cloth "
        "is lit and the bottle hurled at a target, causing an immediate "
        "fireball followed by a raging fire."
    };
    d.weight = ItemWeight::light;
    d.tile = TileId::molotov;
    d.clr = clr_white;
    d.native_containers.push_back(FeatureId::chest);
    d.native_containers.push_back(FeatureId::cabinet);
    d.native_containers.push_back(FeatureId::cocoon);
    data[(size_t)d.id] = d;

    reset_data(d, ItemType::explosive);
    d.id = ItemId::smoke_grenade;
    d.base_name = {"Smoke Grenade", "Smoke Grenades", "a Smoke Grenade"};
    d.base_descr =
    {
        "A sheet steel cylinder with emission holes releasing smoke when the "
        "grenade is ignited. Their primary use is to create smoke screens for "
        "concealment. The fumes produced can harm the eyes, throat and lungs - "
        "so it is recommended to wear a protective mask."
    };
    d.weight = ItemWeight::light;
    d.tile = TileId::flare;
    d.clr = clr_green;
    d.native_containers.push_back(FeatureId::chest);
    d.native_containers.push_back(FeatureId::cabinet);
    data[(size_t)d.id] = d;

    reset_data(d, ItemType::throwing_wpn);
    d.id = ItemId::thr_knife;
    d.base_name = {"Throwing Knife", "Throwing Knives", "a Throwing Knife"};
    d.base_descr =
    {
        "A knife specially designed and weighted so that it can be thrown "
        "effectively."
    };
    d.weight = ItemWeight::extra_light;
    d.tile = TileId::dagger;
    d.glyph = '/';
    d.clr = clr_white;
    d.ranged.throw_dmg = DiceParam(1, 6);
    d.ranged.throw_hit_chance_mod = 0;
    d.ranged.effective_range = 5;
    d.max_stack_at_spawn = 8;
    d.land_on_hard_snd_msg = "I hear a clanking sound.";
    d.land_on_hard_sfx = SfxId::metal_clank;
    d.main_att_mode = AttMode::thrown;
    d.native_containers.push_back(FeatureId::chest);
    d.native_containers.push_back(FeatureId::cabinet);
    d.native_containers.push_back(FeatureId::cocoon);
    data[(size_t)d.id] = d;

    reset_data(d, ItemType::throwing_wpn);
    d.id = ItemId::rock;
    d.base_name = {"Rock", "Rocks", "a Rock"};
    d.base_descr =
    {
        "Although not a very impressive weapon, with skill they can be used "
        "with some result."
    };
    d.weight = ItemWeight::extra_light;
    d.tile = TileId::rock;
    d.glyph = '*';
    d.clr = clr_gray;
    d.ranged.throw_dmg = DiceParam(1, 4);
    d.ranged.throw_hit_chance_mod = 10;
    d.ranged.effective_range = 4;
    d.max_stack_at_spawn = 4;
    d.main_att_mode = AttMode::thrown;
    d.native_containers.push_back(FeatureId::cabinet);
    d.native_containers.push_back(FeatureId::cocoon);
    data[(size_t)d.id] = d;

    reset_data(d, ItemType::melee_wpn);
    d.id = ItemId::dagger;
    d.base_name = {"Dagger", "Daggers", "a Dagger"};
    d.base_descr =
    {
        "Commonly associated with deception, stealth, and treachery. Many "
        "assassinations have been carried out with the use of a dagger.",

        "Attacking an unaware opponent with a dagger does 300% damage "
        "(instead of the normal 150% damage from stealth attacks).",

        "Melee attacks with daggers are silent."
    };
    d.weight = ItemWeight::light;
    d.tile = TileId::dagger;
    d.melee.att_msgs = {"stab", "stabs me with a Dagger"};
    d.melee.dmg = DiceParam(1, 4);
    d.melee.hit_chance_mod = 20;
    d.melee.hit_medium_sfx = SfxId::hit_sharp;
    d.melee.hit_hard_sfx = SfxId::hit_sharp;
    d.melee.miss_sfx = SfxId::miss_light;
    d.ranged.throw_hit_chance_mod = -5;
    d.ranged.effective_range = 4;
    d.native_containers.push_back(FeatureId::chest);
    d.native_containers.push_back(FeatureId::cabinet);
    d.native_containers.push_back(FeatureId::tomb);
    d.native_containers.push_back(FeatureId::cocoon);
    data[(size_t)d.id] = d;

    reset_data(d, ItemType::melee_wpn);
    d.id = ItemId::hatchet;
    d.base_name = {"Hatchet", "Hatchets", "a Hatchet"};
    d.base_descr =
    {
        "A small axe with a short handle. Hatchets are reliable weapons - "
        "they are easy to use, and cause decent damage for their low weight.",

        "Melee attacks with hatchets are silent."
    };
    d.weight = ItemWeight::light;
    d.tile = TileId::axe;
    d.melee.att_msgs = {"strike", "strikes me with a Hatchet"};
    d.melee.dmg = DiceParam(1, 5);
    d.melee.hit_chance_mod = 15;
    d.melee.hit_medium_sfx = SfxId::hit_sharp;
    d.melee.hit_hard_sfx = SfxId::hit_sharp;
    d.melee.miss_sfx = SfxId::miss_light;
    d.ranged.throw_hit_chance_mod = 0;
    d.ranged.effective_range = 5;
    d.native_containers.push_back(FeatureId::chest);
    d.native_containers.push_back(FeatureId::cabinet);
    d.native_containers.push_back(FeatureId::cocoon);
    data[(size_t)d.id] = d;

    reset_data(d, ItemType::melee_wpn);
    d.id = ItemId::club;
    d.base_name = {"Club", "Clubs", "a Club"};
    d.base_descr =
    {
        "Wielded since prehistoric times.",

        "Melee attacks with clubs are noisy."
    };
    d.spawn_std_range = Range(dlvl_first_late_game, INT_MAX);
    d.weight = ItemWeight::medium;
    d.tile = TileId::club;
    d.clr = clr_brown;
    d.melee.att_msgs = {"strike", "strikes me with a Club"};
    d.melee.dmg = DiceParam(2, 3);
    d.melee.hit_chance_mod = 10;
    d.melee.miss_sfx = SfxId::miss_medium;
    d.ranged.throw_hit_chance_mod = -5;
    d.ranged.effective_range = 4;
    d.land_on_hard_snd_msg = "I hear a thudding sound.";
    d.land_on_hard_sfx = SfxId::END;
    data[(size_t)d.id] = d;

    reset_data(d, ItemType::melee_wpn);
    d.id = ItemId::hammer;
    d.base_name = {"Hammer", "Hammers", "a Hammer"};
    d.base_descr =
    {
        "Typically used for construction, but can be quite devastating when "
        "wielded as a weapon.",

        "Melee attacks with hammers are noisy."
    };
    d.weight = ItemWeight::medium;
    d.tile = TileId::hammer;
    d.melee.att_msgs = {"strike", "strikes me with a Hammer"};
    d.melee.dmg = DiceParam(2, 4);
    d.melee.hit_chance_mod = 5;
    d.melee.miss_sfx = SfxId::miss_medium;
    d.ranged.throw_hit_chance_mod = -5;
    d.ranged.effective_range = 4;
    d.native_containers.push_back(FeatureId::cabinet);
    d.native_containers.push_back(FeatureId::cocoon);
    data[(size_t)d.id] = d;

    reset_data(d, ItemType::melee_wpn);
    d.id = ItemId::machete;
    d.base_name = {"Machete", "Machetes", "a Machete"};
    d.base_descr =
    {
        "A large cleaver-like knife. It serves well both as a cutting tool "
        "and weapon.",

        "Melee attacks with machetes are noisy."
    };
    d.weight = ItemWeight::medium;
    d.tile = TileId::machete;
    d.melee.att_msgs = {"strike", "strikes me with a Machete"};
    d.melee.dmg = DiceParam(2, 5);
    d.melee.hit_chance_mod = 0;
    d.melee.hit_small_sfx = SfxId::hit_sharp;
    d.melee.hit_medium_sfx = SfxId::hit_sharp;
    d.melee.miss_sfx = SfxId::miss_medium;
    d.ranged.throw_hit_chance_mod = -5;
    d.ranged.effective_range = 4;
    d.native_containers.push_back(FeatureId::cabinet);
    d.native_containers.push_back(FeatureId::cocoon);
    data[(size_t)d.id] = d;

    reset_data(d, ItemType::melee_wpn);
    d.id = ItemId::axe;
    d.base_name = {"Axe", "Axes", "an Axe"};
    d.base_descr =
    {
        "A tool intended for felling trees, splitting timber, etc. Used as a "
        "weapon it can deliver devastating blows, although it requires some "
        "skill to use effectively."
        /*TODO: "Also effective for breaching wooden doors."*/,

        "Melee attacks with axes are noisy."
    };
    d.weight = ItemWeight::medium;
    d.tile = TileId::axe;
    d.melee.att_msgs = {"strike", "strikes me with an axe"};
    d.melee.dmg = DiceParam(2, 6);
    d.melee.hit_chance_mod = -5;
    d.melee.miss_sfx = SfxId::miss_medium;
    d.ranged.throw_hit_chance_mod = -5;
    d.ranged.effective_range = 4;
    d.native_containers.push_back(FeatureId::cabinet);
    d.native_containers.push_back(FeatureId::tomb);
    d.native_containers.push_back(FeatureId::cocoon);
    data[(size_t)d.id] = d;

    reset_data(d, ItemType::melee_wpn);
    d.id = ItemId::pitch_fork;
    d.base_name = {"Pitchfork", "Pitchforks", "a Pitchfork"};
    d.base_descr =
    {
        "A long staff with a forked, four-pronged end. Victims can be pushed "
        "away when stabbed, to keep them at bay."
    };
    d.weight = ItemWeight::heavy;
    d.tile = TileId::pitchfork;
    d.melee.att_msgs = {"strike", "strikes me with a Pitchfork"};
    d.melee.dmg = DiceParam(3, 4);
    d.melee.hit_chance_mod = -15;
    d.melee.knocks_back = true;
    d.melee.hit_small_sfx = SfxId::hit_sharp;
    d.melee.hit_medium_sfx = SfxId::hit_sharp;
    d.melee.miss_sfx = SfxId::miss_heavy;
    d.ranged.throw_hit_chance_mod = -10;
    d.ranged.effective_range = 3;
    d.native_containers.push_back(FeatureId::cabinet);
    d.native_containers.push_back(FeatureId::cocoon);
    data[(size_t)d.id] = d;

    reset_data(d, ItemType::melee_wpn);
    d.id = ItemId::sledge_hammer;
    d.base_name = {"Sledgehammer", "Sledgehammers", "a Sledgehammer"};
    d.base_descr =
    {
        "Often used in destruction work for breaking through walls. It can "
        "deal a great amount of damage, although it is cumbersome to carry, "
        "and it requires some skill to use effectively."
    };
    d.weight = ItemWeight::heavy;
    d.tile = TileId::sledge_hammer;
    d.melee.att_msgs = {"strike", "strikes me with a Sledgehammer"};
    d.melee.dmg = DiceParam(3, 5);
    d.melee.hit_chance_mod = -15;
    d.melee.knocks_back = true;
    d.melee.miss_sfx = SfxId::miss_heavy;
    d.ranged.throw_hit_chance_mod = -10;
    d.ranged.effective_range = 3;
    d.native_containers.push_back(FeatureId::cabinet);
    data[(size_t)d.id] = d;

    reset_data(d, ItemType::melee_wpn);
    d.id = ItemId::pharaoh_staff;
    d.base_name =
    {
        "Staff of the Pharaohs", "Staff of the Pharaohs",
        "the Staff of the Pharaohs"
    };
    d.base_descr =
    {
        "Once wielded by long-forgotten kings in ancient times, this powerful "
        "artifact grants the power to call up a loyal servant from the dead."
    };
    d.clr = clr_magenta;
    d.weight = ItemWeight::medium;
    d.tile = TileId::pharaoh_staff;
    d.melee.att_msgs = {"strike", "strikes me with the Staff of the Pharaohs"};
    d.melee.dmg = DiceParam(2, 4);
    d.melee.hit_chance_mod = 0;
    d.melee.miss_sfx = SfxId::miss_medium;
    d.ranged.throw_hit_chance_mod = -10;
    d.ranged.effective_range = 3;
    d.chance_to_incl_in_spawn_list = 1;
    d.value = ItemValue::major_treasure;
    d.is_ins_raied_while_carried = true;
    d.native_containers.push_back(FeatureId::tomb);
    data[(size_t)d.id] = d;

    reset_data(d, ItemType::throwing_wpn);
    d.id = ItemId::iron_spike;
    d.base_name = {"Iron Spike", "Iron Spikes", "an Iron Spike"};
    d.base_descr =
    {
        "Can be useful for wedging things closed."
        /*TODO: or prying things open."*/
    };
    d.weight = ItemWeight::extra_light;
    d.tile = TileId::iron_spike;
    d.is_stackable = true;
    d.clr = clr_gray;
    d.glyph = '/';
    d.ranged.throw_hit_chance_mod = -5;
    d.ranged.throw_dmg = DiceParam(1, 4);
    d.ranged.effective_range = 3;
    d.max_stack_at_spawn = 12;
    d.land_on_hard_snd_msg = "I hear a clanking sound.";
    d.land_on_hard_sfx = SfxId::metal_clank;
    d.main_att_mode = AttMode::thrown;
    d.native_containers.push_back(FeatureId::cabinet);
    d.native_containers.push_back(FeatureId::cocoon);
    data[(size_t)d.id] = d;

    reset_data(d, ItemType::melee_wpn_intr);
    d.id = ItemId::player_kick;
    d.melee.att_msgs = {"kick", ""};
    d.melee.hit_chance_mod = 15;
    d.melee.dmg = DiceParam(1, 2);
    d.melee.knocks_back = true;
    d.melee.miss_sfx = SfxId::miss_medium;
    data[(size_t)d.id] = d;

    reset_data(d, ItemType::melee_wpn_intr);
    d.id = ItemId::player_stomp;
    d.melee.att_msgs = {"stomp", ""};
    d.melee.hit_chance_mod =
        data[(size_t)ItemId::player_kick].melee.hit_chance_mod;
    d.melee.dmg = data[(size_t)ItemId::player_kick].melee.dmg;
    d.melee.miss_sfx = data[(size_t)ItemId::player_kick].melee.miss_sfx;
    d.melee.knocks_back = false;
    data[(size_t)d.id] = d;

    reset_data(d, ItemType::melee_wpn_intr);
    d.id = ItemId::player_punch;
    d.base_name = {"Punch", "", ""};
    d.melee.att_msgs = {"punch", ""};
    d.melee.hit_chance_mod = 20;
    d.melee.dmg = DiceParam(1, 1);
    d.melee.miss_sfx = SfxId::miss_light;
    data[(size_t)d.id] = d;

    reset_data(d, ItemType::melee_wpn_intr);
    d.id = ItemId::player_ghoul_claw;
    d.base_name = {"Claw", "", ""};
    d.melee.att_msgs = {"claw", ""};
    d.melee.hit_chance_mod = 20;
    d.melee.dmg = DiceParam(1, 8);
    d.melee.hit_small_sfx = SfxId::hit_sharp;
    d.melee.hit_medium_sfx = SfxId::hit_sharp;
    d.melee.miss_sfx = SfxId::miss_medium;
    data[(size_t)d.id] = d;

    reset_data(d, ItemType::melee_wpn_intr);
    d.id = ItemId::zombie_claw;
    d.melee.att_msgs = {"", "claws me"};
    set_dmg_from_mon_id(d, ActorId::zombie);
    data[(size_t)d.id] = d;

    reset_data(d, ItemType::melee_wpn_intr);
    d.id = ItemId::zombie_claw_diseased;
    d.melee.att_msgs = {"", "claws me"};
    set_dmg_from_mon_id(d, ActorId::zombie);
    d.melee.prop_applied = new PropInfected(PropTurns::std);
    data[(size_t)d.id] = d;

    reset_data(d, ItemType::melee_wpn_intr);
    d.id = ItemId::zombie_axe;
    d.melee.att_msgs = {"", "chops me with a rusty axe"};
    set_dmg_from_mon_id(d, ActorId::zombie_axe);
    data[(size_t)d.id] = d;

    reset_data(d, ItemType::melee_wpn_intr);
    d.id = ItemId::bloated_zombie_punch;
    d.melee.att_msgs = {"", "mauls me"};
    set_dmg_from_mon_id(d, ActorId::bloated_zombie);
    d.melee.knocks_back = true;
    data[(size_t)d.id] = d;

    reset_data(d, ItemType::ranged_wpn_intr);
    d.id = ItemId::bloated_zombie_spit;
    d.ranged.att_msgs = {"", "spits acid pus at me"};
    set_dmg_from_mon_id(d, ActorId::bloated_zombie);
    d.ranged.snd_msg = "I hear spitting.";
    d.ranged.projectile_clr = clr_green_lgt;
    d.ranged.dmg_type = DmgType::acid;
    d.ranged.projectile_glyph = '*';
    data[(size_t)d.id] = d;

    reset_data(d, ItemType::melee_wpn_intr);
    d.id = ItemId::crawling_intestines_strangle;
    d.melee.att_msgs = {"", "strangles me"};
    set_dmg_from_mon_id(d, ActorId::crawling_intestines);
    data[(size_t)d.id] = d;

    reset_data(d, ItemType::melee_wpn_intr);
    d.id = ItemId::floating_skull_bite;
    d.melee.att_msgs = {"", "bites me"};
    set_dmg_from_mon_id(d, ActorId::floating_skull);
    data[(size_t)d.id] = d;

    reset_data(d, ItemType::melee_wpn_intr);
    d.id = ItemId::crawling_hand_strangle;
    d.melee.att_msgs = {"", "strangles me"};
    set_dmg_from_mon_id(d, ActorId::crawling_hand);
    data[(size_t)d.id] = d;

    reset_data(d, ItemType::melee_wpn_intr);
    d.id = ItemId::thing_strangle;
    d.melee.att_msgs = {"", "strangles me"};
    set_dmg_from_mon_id(d, ActorId::thing);
    data[(size_t)d.id] = d;

    reset_data(d, ItemType::melee_wpn_intr);
    d.id = ItemId::rat_bite;
    d.melee.att_msgs = {"", "bites me"};
    set_dmg_from_mon_id(d, ActorId::rat);
    data[(size_t)d.id] = d;

    reset_data(d, ItemType::melee_wpn_intr);
    d.id = ItemId::rat_bite_diseased;
    d.melee.att_msgs = {"", "bites me"};
    set_dmg_from_mon_id(d, ActorId::rat);
    d.melee.prop_applied = new PropInfected(PropTurns::std);
    data[(size_t)d.id] = d;

    reset_data(d, ItemType::melee_wpn_intr);
    d.id = ItemId::rat_thing_bite;
    d.melee.att_msgs = {"", "bites me"};
    set_dmg_from_mon_id(d, ActorId::rat_thing);
    data[(size_t)d.id] = d;

    reset_data(d, ItemType::melee_wpn_intr);
    d.id = ItemId::brown_jenkin_bite;
    d.melee.att_msgs = {"", "bites me"};
    set_dmg_from_mon_id(d, ActorId::brown_jenkin);
    data[(size_t)d.id] = d;

    reset_data(d, ItemType::melee_wpn_intr);
    d.id = ItemId::worm_mass_bite;
    d.melee.att_msgs = {"", "bites me"};
    set_dmg_from_mon_id(d, ActorId::worm_mass);
    data[(size_t)d.id] = d;

    reset_data(d, ItemType::melee_wpn_intr);
    d.id = ItemId::mind_worms_bite;
    d.melee.att_msgs = {"", "bites me"};
    set_dmg_from_mon_id(d, ActorId::mind_worms);
    d.melee.prop_applied = new PropConfused(PropTurns::specific, 9);
    data[(size_t)d.id] = d;

    reset_data(d, ItemType::melee_wpn_intr);
    d.id = ItemId::wolf_bite;
    d.melee.att_msgs = {"", "bites me"};
    set_dmg_from_mon_id(d, ActorId::wolf);
    data[(size_t)d.id] = d;

    reset_data(d, ItemType::melee_wpn_intr);
    d.id = ItemId::green_spider_bite;
    d.melee.att_msgs = {"", "bites me"};
    set_dmg_from_mon_id(d, ActorId::green_spider);
    d.melee.prop_applied = new PropBlind(PropTurns::specific, 3);
    data[(size_t)d.id] = d;

    reset_data(d, ItemType::melee_wpn_intr);
    d.id = ItemId::white_spider_bite;
    d.melee.att_msgs = {"", "bites me"};
    set_dmg_from_mon_id(d, ActorId::white_spider);
    d.melee.prop_applied = new PropParalyzed(PropTurns::specific, 2);
    data[(size_t)d.id] = d;

    reset_data(d, ItemType::melee_wpn_intr);
    d.id = ItemId::red_spider_bite;
    d.melee.att_msgs = {"", "bites me"};
    set_dmg_from_mon_id(d, ActorId::red_spider);
    d.melee.prop_applied = new PropWeakened(PropTurns::std);
    data[(size_t)d.id] = d;

    reset_data(d, ItemType::melee_wpn_intr);
    d.id = ItemId::shadow_spider_bite;
    d.melee.att_msgs = {"", "bites me"};
    set_dmg_from_mon_id(d, ActorId::shadow_spider);
    data[(size_t)d.id] = d;

    reset_data(d, ItemType::melee_wpn_intr);
    d.id = ItemId::leng_spider_bite;
    d.melee.att_msgs = {"", "bites me"};
    set_dmg_from_mon_id(d, ActorId::leng_spider);
    data[(size_t)d.id] = d;

    reset_data(d, ItemType::melee_wpn_intr);
    d.id = ItemId::pit_viper_bite;
    d.melee.att_msgs = {"", "bites me"};
    set_dmg_from_mon_id(d, ActorId::pit_viper);
    d.melee.prop_applied = new PropPoisoned(PropTurns::specific,
                                            poison_dmg_n_turn * 4);
    data[(size_t)d.id] = d;

    reset_data(d, ItemType::melee_wpn_intr);
    d.id = ItemId::spitting_cobra_bite;
    d.melee.att_msgs = {"", "bites me"};
    set_dmg_from_mon_id(d, ActorId::spitting_cobra);
    d.melee.prop_applied = new PropPoisoned(PropTurns::specific,
                                            poison_dmg_n_turn * 4);
    data[(size_t)d.id] = d;

    reset_data(d, ItemType::ranged_wpn_intr);
    d.id = ItemId::spitting_cobra_spit;
    d.ranged.att_msgs = {"", "spits venom at me"};
    set_dmg_from_mon_id(d, ActorId::spitting_cobra);
    d.ranged.snd_msg = "I hear hissing and spitting.";
    d.ranged.projectile_clr = clr_green_lgt;
    d.ranged.dmg_type = DmgType::physical;
    d.ranged.projectile_glyph = '*';
    data[(size_t)d.id] = d;

    reset_data(d, ItemType::melee_wpn_intr);
    d.id = ItemId::black_mamba_bite;
    d.melee.att_msgs = {"", "bites me"};
    set_dmg_from_mon_id(d, ActorId::black_mamba);
    d.melee.prop_applied = new PropPoisoned(PropTurns::specific,
                                            poison_dmg_n_turn * 4);
    data[(size_t)d.id] = d;

    reset_data(d, ItemType::ranged_wpn_intr);
    d.id = ItemId::fire_hound_breath;
    d.ranged.att_msgs = {"", "breathes fire at me"};
    d.ranged.snd_msg = "I hear a burst of flames.";
    set_dmg_from_mon_id(d, ActorId::fire_hound);
    d.ranged.prop_applied = new PropBurning(PropTurns::std);
    d.ranged.projectile_clr = clr_red_lgt;
    d.ranged.projectile_glyph = '*';
    d.ranged.projectile_leaves_trail = true;
    d.ranged.projectile_leaves_smoke = true;
    d.ranged.dmg_type = DmgType::fire;
    data[(size_t)d.id] = d;

    reset_data(d, ItemType::melee_wpn_intr);
    d.id = ItemId::fire_hound_bite;
    d.melee.att_msgs = {"", "bites me"};
    set_dmg_from_mon_id(d, ActorId::fire_hound);
    d.melee.prop_applied = new PropBurning(PropTurns::std);
    d.melee.dmg_type = DmgType::fire;
    data[(size_t)d.id] = d;

    reset_data(d, ItemType::melee_wpn_intr);
    d.id = ItemId::zuul_bite;
    d.melee.att_msgs = {"", "bites me"};
    set_dmg_from_mon_id(d, ActorId::zuul);
    d.melee.dmg_type = DmgType::physical;
    data[(size_t)d.id] = d;

    reset_data(d, ItemType::melee_wpn_intr);
    d.id = ItemId::dust_vortex_engulf;
    d.melee.att_msgs = {"", "engulfs me"};
    set_dmg_from_mon_id(d, ActorId::dust_vortex);
    data[(size_t)d.id] = d;

    reset_data(d, ItemType::melee_wpn_intr);
    d.id = ItemId::fire_vortex_engulf;
    d.melee.att_msgs = {"", "engulfs me"};
    set_dmg_from_mon_id(d, ActorId::fire_vortex);
    d.melee.prop_applied = new PropBurning(PropTurns::std);
    data[(size_t)d.id] = d;

    reset_data(d, ItemType::melee_wpn_intr);
    d.id = ItemId::ghost_claw;
    d.melee.att_msgs = {"", "claws me"};
    set_dmg_from_mon_id(d, ActorId::ghost);
    d.melee.prop_applied = new PropTerrified(PropTurns::specific, 4);
    d.melee.dmg_type = DmgType::spirit;
    data[(size_t)d.id] = d;

    reset_data(d, ItemType::melee_wpn_intr);
    d.id = ItemId::phantasm_sickle;
    d.melee.att_msgs = {"", "slices me with an ethereal sickle"};
    set_dmg_from_mon_id(d, ActorId::phantasm);
    d.melee.prop_applied = new PropTerrified(PropTurns::specific, 4);
    d.melee.dmg_type = DmgType::spirit;
    data[(size_t)d.id] = d;

    reset_data(d, ItemType::melee_wpn_intr);
    d.id = ItemId::wraith_claw;
    d.melee.att_msgs = {"", "claws me"};
    set_dmg_from_mon_id(d, ActorId::wraith);
    d.melee.prop_applied = new PropTerrified(PropTurns::specific, 4);
    d.melee.dmg_type = DmgType::spirit;
    data[(size_t)d.id] = d;

    reset_data(d, ItemType::melee_wpn_intr);
    d.id = ItemId::raven_peck;
    d.melee.att_msgs = {"", "pecks at me"};
    set_dmg_from_mon_id(d, ActorId::raven);
    data[(size_t)d.id] = d;

    reset_data(d, ItemType::melee_wpn_intr);
    d.id = ItemId::giant_bat_bite;
    d.melee.att_msgs = {"", "bites me"};
    set_dmg_from_mon_id(d, ActorId::giant_bat);
    data[(size_t)d.id] = d;

    reset_data(d, ItemType::melee_wpn_intr);
    d.id = ItemId::polyp_tentacle;
    d.melee.att_msgs = {"", "grips me with a tentacle"};
    d.melee.prop_applied = new PropParalyzed(PropTurns::specific, 1);
    set_dmg_from_mon_id(d, ActorId::flying_polyp);
    data[(size_t)d.id] = d;

    reset_data(d, ItemType::melee_wpn_intr);
    d.id = ItemId::greater_polyp_tentacle;
    d.melee.att_msgs = {"", "grips me with a tentacle"};
    d.melee.prop_applied = new PropParalyzed(PropTurns::specific, 1);
    set_dmg_from_mon_id(d, ActorId::greater_polyp);
    data[(size_t)d.id] = d;

    reset_data(d, ItemType::melee_wpn_intr);
    d.id = ItemId::ghoul_claw;
    d.melee.att_msgs = {"", "claws me"};
    set_dmg_from_mon_id(d, ActorId::ghoul);
    d.melee.prop_applied = new PropInfected(PropTurns::std);
    data[(size_t)d.id] = d;

    reset_data(d, ItemType::melee_wpn_intr);
    d.id = ItemId::shadow_claw;
    d.melee.att_msgs = {"", "claws me"};
    set_dmg_from_mon_id(d, ActorId::shadow);
    data[(size_t)d.id] = d;

    reset_data(d, ItemType::melee_wpn_intr);
    d.id = ItemId::invis_stalker_claw;
    d.melee.att_msgs = {"", "claws me"};
    set_dmg_from_mon_id(d, ActorId::invis_stalker);
    data[(size_t)d.id] = d;

    reset_data(d, ItemType::melee_wpn_intr);
    d.id = ItemId::byakhee_claw;
    d.melee.att_msgs = {"", "claws me"};
    set_dmg_from_mon_id(d, ActorId::byakhee);
    data[(size_t)d.id] = d;

    reset_data(d, ItemType::melee_wpn_intr);
    d.id = ItemId::giant_mantis_claw;
    d.melee.att_msgs = {"", "claws me"};
    set_dmg_from_mon_id(d, ActorId::giant_mantis);
    data[(size_t)d.id] = d;

    reset_data(d, ItemType::melee_wpn_intr);
    d.id = ItemId::giant_locust_bite;
    d.melee.att_msgs = {"", "bites me"};
    set_dmg_from_mon_id(d, ActorId::locust);
    data[(size_t)d.id] = d;

    reset_data(d, ItemType::melee_wpn_intr);
    d.id = ItemId::mummy_maul;
    d.melee.att_msgs = {"", "mauls me"};
    set_dmg_from_mon_id(d, ActorId::mummy);
    d.melee.prop_applied = new PropCursed(PropTurns::indefinite);
    d.melee.knocks_back = true;
    data[(size_t)d.id] = d;

    reset_data(d, ItemType::melee_wpn_intr);
    d.id = ItemId::croc_head_mummy_spear;
    d.melee.att_msgs = {"", "hits me with a spear"};
    set_dmg_from_mon_id(d, ActorId::croc_head_mummy);
    data[(size_t)d.id] = d;

    reset_data(d, ItemType::ranged_wpn_intr);
    d.id = ItemId::deep_one_javelin_att;
    d.ranged.att_msgs = {"", "throws a javelin at me"};
    set_dmg_from_mon_id(d, ActorId::deep_one);
    d.ranged.snd_msg = "";
    d.ranged.projectile_clr = clr_brown;
    d.ranged.projectile_glyph = '/';
    d.ranged.snd_vol = SndVol::low;
    data[(size_t)d.id] = d;

    reset_data(d, ItemType::melee_wpn_intr);
    d.id = ItemId::deep_one_spear_att;
    d.melee.att_msgs = {"", "hits me with a spear"};
    set_dmg_from_mon_id(d, ActorId::deep_one);
    data[(size_t)d.id] = d;

    reset_data(d, ItemType::melee_wpn_intr);
    d.id = ItemId::ape_maul;
    d.melee.att_msgs = {"", "mauls me"};
    set_dmg_from_mon_id(d, ActorId::ape);
    data[(size_t)d.id] = d;

    reset_data(d, ItemType::melee_wpn_intr);
    d.id = ItemId::ooze_black_spew_pus;
    d.melee.att_msgs = {"", "spews pus on me"};
    set_dmg_from_mon_id(d, ActorId::ooze_black);
    data[(size_t)d.id] = d;

    reset_data(d, ItemType::melee_wpn_intr);
    d.id = ItemId::ooze_clear_spew_pus;
    d.melee.att_msgs = {"", "spews pus on me"};
    set_dmg_from_mon_id(d, ActorId::ooze_clear);
    data[(size_t)d.id] = d;

    reset_data(d, ItemType::melee_wpn_intr);
    d.id = ItemId::ooze_putrid_spew_pus;
    d.melee.att_msgs = {"", "spews infected pus on me"};
    set_dmg_from_mon_id(d, ActorId::ooze_putrid);
    d.melee.prop_applied = new PropInfected(PropTurns::std);
    data[(size_t)d.id] = d;

    reset_data(d, ItemType::melee_wpn_intr);
    d.id = ItemId::ooze_poison_spew_pus;
    d.melee.att_msgs = {"", "spews poisonous pus on me"};
    set_dmg_from_mon_id(d, ActorId::ooze_poison);
    d.melee.prop_applied = new PropPoisoned(PropTurns::std);
    data[(size_t)d.id] = d;

    reset_data(d, ItemType::melee_wpn_intr);
    d.id = ItemId::color_oo_space_touch;
    d.melee.att_msgs = {"", "touches me"};
    set_dmg_from_mon_id(d, ActorId::color_oo_space);
    data[(size_t)d.id] = d;

    reset_data(d, ItemType::melee_wpn_intr);
    d.id = ItemId::chthonian_bite;
    d.melee.att_msgs = {"", "strikes me with a tentacle"};
    d.melee.knocks_back = true;
    set_dmg_from_mon_id(d, ActorId::chthonian);
    data[(size_t)d.id] = d;

    reset_data(d, ItemType::melee_wpn_intr);
    d.id = ItemId::death_fiend_claw;
    d.melee.att_msgs = {"", "claws me"};
    set_dmg_from_mon_id(d, ActorId::death_fiend);
    d.melee.dmg_type = DmgType::pure;
    data[(size_t)d.id] = d;

    reset_data(d, ItemType::melee_wpn_intr);
    d.id = ItemId::hunting_horror_bite;
    d.melee.att_msgs = {"", "bites me"};
    set_dmg_from_mon_id(d, ActorId::hunting_horror);
    d.melee.prop_applied = new PropPoisoned(PropTurns::std);
    data[(size_t)d.id] = d;

    reset_data(d, ItemType::melee_wpn_intr);
    d.id = ItemId::mold_spores;
    d.melee.att_msgs = {"", "releases spores at me"};
    set_dmg_from_mon_id(d, ActorId::mold);
    d.melee.prop_applied = new PropPoisoned(PropTurns::specific,
                                            poison_dmg_n_turn * 2);
    data[(size_t)d.id] = d;

    reset_data(d, ItemType::melee_wpn_intr);
    d.id = ItemId::mi_go_sting;
    d.melee.att_msgs = {"", "stings me"};
    set_dmg_from_mon_id(d, ActorId::mi_go);
    d.melee.prop_applied = new PropPoisoned(PropTurns::specific,
                                            poison_dmg_n_turn * 2);
    data[(size_t)d.id] = d;

    reset_data(d, ItemType::melee_wpn_intr);
    d.id = ItemId::mi_go_commander_sting;
    d.melee.att_msgs = {"", "stings me"};
    set_dmg_from_mon_id(d, ActorId::mi_go_commander);
    d.melee.prop_applied = new PropPoisoned(PropTurns::specific,
                                            poison_dmg_n_turn * 2);
    data[(size_t)d.id] = d;

    reset_data(d, ItemType::melee_wpn_intr);
    d.id = ItemId::the_high_priest_claw;
    d.melee.att_msgs = {"", "claws me"};
    set_dmg_from_mon_id(d, ActorId::the_high_priest);
    data[(size_t)d.id] = d;

    reset_data(d, ItemType::armor);
    d.id = ItemId::armor_leather_jacket;
    d.base_name = {"Leather Jacket", "", "a Leather Jacket"};
    d.base_descr =
    {
        "It offers some protection."
    };
    d.weight = ItemWeight::light;
    d.clr = clr_brown;
    d.spawn_std_range.min = 1;
    d.armor.armor_points = 1;
    d.armor.dmg_to_durability_factor = 1.0;
    d.land_on_hard_snd_msg = "";
    d.native_containers.push_back(FeatureId::cabinet);
    data[(size_t)d.id] = d;

    reset_data(d, ItemType::armor);
    d.id = ItemId::armor_iron_suit;
    d.base_name = {"Iron Suit", "", "an Iron Suit"};
    d.base_descr =
    {
        "A crude armour constructed from metal plates, bolts, and leather "
        "straps.",

        "It can absorb a high amount of damage, but it makes sneaking and "
        "dodging very difficult."
    };
    d.ability_mods_while_equipped[(size_t)AbilityId::stealth]   = -50;
    d.ability_mods_while_equipped[(size_t)AbilityId::dodging]   = -50;
    d.weight = ItemWeight::heavy;
    d.clr = clr_white;
    d.spawn_std_range.min = 2;
    d.armor.armor_points = 5;
    d.armor.dmg_to_durability_factor = 0.3;
    d.land_on_hard_snd_msg = "I hear a crashing sound.";
    d.native_containers.push_back(FeatureId::cabinet);
    data[(size_t)d.id] = d;

    reset_data(d, ItemType::armor);
    d.id = ItemId::armor_flack_jacket;
    d.base_name = {"Flak Jacket", "", "a Flak Jacket"};
    d.base_descr =
    {
        "An armour consisting of steel plates sewn into a waistcoat. It "
        "offers very good protection for its weight. Sneaking and dodging is "
        "slightly more difficult."
    };
    d.ability_mods_while_equipped[(size_t)AbilityId::stealth]   = -20;
    d.ability_mods_while_equipped[(size_t)AbilityId::dodging]   = -20;
    d.weight = ItemWeight::medium;
    d.clr = clr_green;
    d.spawn_std_range.min = 3;
    d.armor.armor_points = 3;
    d.armor.dmg_to_durability_factor = 0.5;
    d.land_on_hard_snd_msg = "I hear a thudding sound.";
    d.native_containers.push_back(FeatureId::cabinet);
    data[(size_t)d.id] = d;

    reset_data(d, ItemType::armor);
    d.id = ItemId::armor_asb_suit;
    d.base_name = {"Asbestos Suit", "", "an Asbestos Suit"};
    d.base_descr =
    {
        "A one piece overall of asbestos fabric, including a hood, furnace "
        "mask, gloves and shoes. It protects the wearer against fire, acid "
        "and electricity, and also against smoke, fumes and gas.",

        "It is a bit bulky, so sneaking and dodging is slightly more difficult."
        "Also, because of the hood and mask, aiming and detecting hidden "
        "enemies and objects is somewhat harder."
    };
    d.ability_mods_while_equipped[(size_t)AbilityId::stealth]   = -20;
    d.ability_mods_while_equipped[(size_t)AbilityId::melee]     = -10;
    d.ability_mods_while_equipped[(size_t)AbilityId::ranged]    = -10;
    d.ability_mods_while_equipped[(size_t)AbilityId::dodging]   = -20;
    d.ability_mods_while_equipped[(size_t)AbilityId::searching] = -6;
    d.weight = ItemWeight::medium;
    d.clr = clr_red_lgt;
    d.spawn_std_range.min = 3;
    d.armor.armor_points = 1;
    d.armor.dmg_to_durability_factor = 1.0;
    d.land_on_hard_snd_msg = "";
    d.native_containers.push_back(FeatureId::cabinet);
    d.native_containers.push_back(FeatureId::chest);
    data[(size_t)d.id] = d;

    reset_data(d, ItemType::armor);
    d.id = ItemId::armor_mi_go;
    d.base_name = {"Mi-go Bio-armor", "", "a Mi-go Bio-armor"};
    d.base_descr =
    {
        "An extremely durable biological armor created by the Mi-go."
    };
    d.spawn_std_range = Range(-1, -1);
    d.weight = ItemWeight::medium;
    d.is_ins_raied_while_equiped = true;
    d.clr = clr_magenta;
    d.tile = TileId::mi_go_armor;
    d.armor.armor_points = 3;
    d.armor.dmg_to_durability_factor = 0.1;
    d.land_on_hard_snd_msg = "";
    data[(size_t)d.id] = d;

    reset_data(d, ItemType::head_wear);
    d.id = ItemId::gas_mask;
    d.base_name = {"Gas Mask", "", "a Gas Mask"};
    d.base_descr =
    {
        "Protects the eyes, throat and lungs from smoke and fumes. It has a "
        "limited useful lifespan that is related to the absorbent capacity of "
        "the filter.",

        "Due to the small eye windows, aiming is slightly more difficult, and "
        "it is harder to detect sneaking enemies and hidden objects."
    };
    d.ability_mods_while_equipped[(size_t)AbilityId::melee]     = -10;
    d.ability_mods_while_equipped[(size_t)AbilityId::ranged]    = -10;
    d.ability_mods_while_equipped[(size_t)AbilityId::searching] = -6;
    d.is_stackable = false;
    d.clr = clr_brown;
    d.tile = TileId::gas_mask;
    d.glyph = '[';
    d.spawn_std_range = Range(1, dlvl_last_early_game);
    d.chance_to_incl_in_spawn_list = 50;
    d.weight = ItemWeight::light;
    d.land_on_hard_snd_msg = "";
    data[(size_t)d.id] = d;

//    reset_data(d, ItemType::head_wear);
//    d.id = ItemId::hideous_mask;
//    d.base_name = {"Hideous Mask", "", "The Hideous Mask"};
//    d.base_descr =
//    {
//        "[TODO]",
//
//        item_carry_shock_descr
//    };
//    d.is_stackable = false;
//    d.clr = clr_magenta;
//    d.tile = TileId::mask;
//    d.glyph = '[';
//    d.spawn_std_range = Range(-1, -1);
//    d.weight = ItemWeight::light;
//    d.land_on_hard_snd_msg = "";
//    d.chance_to_incl_in_spawn_list = 1;
//    d.value = ItemValue::major_treasure;
//    d.shock_while_in_backpack = d.shock_while_equipped = 15;
//    d.native_containers.push_back(FeatureId::tomb, 8);
//    data[(size_t)d.id] = d;

    reset_data(d, ItemType::scroll);
    d.id = ItemId::scroll_mayhem;
    d.spell_cast_from_scroll = SpellId::mayhem;
    data[(size_t)d.id] = d;

    reset_data(d, ItemType::scroll);
    d.id = ItemId::scroll_telep;
    d.spell_cast_from_scroll = SpellId::teleport;
    data[(size_t)d.id] = d;

    reset_data(d, ItemType::scroll);
    d.id = ItemId::scroll_pest;
    d.spell_cast_from_scroll = SpellId::pest;
    data[(size_t)d.id] = d;

    reset_data(d, ItemType::scroll);
    d.id = ItemId::scroll_enfeeble_mon;
    d.spell_cast_from_scroll = SpellId::enfeeble_mon;
    data[(size_t)d.id] = d;

    reset_data(d, ItemType::scroll);
    d.id = ItemId::scroll_det_items;
    d.spell_cast_from_scroll = SpellId::det_items;
    data[(size_t)d.id] = d;

    reset_data(d, ItemType::scroll);
    d.id = ItemId::scroll_det_traps;
    d.spell_cast_from_scroll = SpellId::det_traps;
    data[(size_t)d.id] = d;

    reset_data(d, ItemType::scroll);
    d.id = ItemId::scroll_bless;
    d.spell_cast_from_scroll = SpellId::bless;
    data[(size_t)d.id] = d;

    reset_data(d, ItemType::scroll);
    d.id = ItemId::scroll_darkbolt;
    d.spell_cast_from_scroll = SpellId::darkbolt;
    data[(size_t)d.id] = d;

    reset_data(d, ItemType::scroll);
    d.id = ItemId::scroll_aza_wrath;
    d.spell_cast_from_scroll = SpellId::aza_wrath;
    data[(size_t)d.id] = d;

    reset_data(d, ItemType::scroll);
    d.id = ItemId::scroll_opening;
    d.spawn_std_range.max = dlvl_first_mid_game;
    d.spell_cast_from_scroll = SpellId::opening;
    data[(size_t)d.id] = d;

    reset_data(d, ItemType::scroll);
    d.id = ItemId::scroll_sacr_life;
    d.spell_cast_from_scroll = SpellId::sacr_life;
    data[(size_t)d.id] = d;

    reset_data(d, ItemType::scroll);
    d.id = ItemId::scroll_sacr_spi;
    d.spell_cast_from_scroll = SpellId::sacr_spi;
    data[(size_t)d.id] = d;

    reset_data(d, ItemType::scroll);
    d.id = ItemId::scroll_det_mon;
    d.spell_cast_from_scroll = SpellId::det_mon;
    data[(size_t)d.id] = d;

    reset_data(d, ItemType::scroll);
    d.id = ItemId::scroll_res;
    d.spell_cast_from_scroll = SpellId::res;
    data[(size_t)d.id] = d;

    reset_data(d, ItemType::scroll);
    d.id = ItemId::scroll_summon_mon;
    d.spell_cast_from_scroll = SpellId::summon;
    data[(size_t)d.id] = d;

    reset_data(d, ItemType::scroll);
    d.id = ItemId::scroll_light;
    d.spell_cast_from_scroll = SpellId::light;
    data[(size_t)d.id] = d;

    reset_data(d, ItemType::scroll);
    d.id = ItemId::scroll_anim_wpns;
    d.spell_cast_from_scroll = SpellId::anim_wpns;
    data[(size_t)d.id] = d;

    reset_data(d, ItemType::potion);
    d.id = ItemId::potion_vitality;
    data[(size_t)d.id] = d;

    reset_data(d, ItemType::potion);
    d.id = ItemId::potion_spirit;
    mod_spawn_chance(d, 0.66);
    data[(size_t)d.id] = d;

    reset_data(d, ItemType::potion);
    d.id = ItemId::potion_blindness;
    data[(size_t)d.id] = d;

    reset_data(d, ItemType::potion);
    d.id = ItemId::potion_fortitude;
    data[(size_t)d.id] = d;

    reset_data(d, ItemType::potion);
    d.id = ItemId::potion_paralyze;
    data[(size_t)d.id] = d;

    reset_data(d, ItemType::potion);
    d.id = ItemId::potion_r_elec;
    mod_spawn_chance(d, 0.33);
    data[(size_t)d.id] = d;

    reset_data(d, ItemType::potion);
    d.id = ItemId::potion_conf;
    mod_spawn_chance(d, 0.66);
    data[(size_t)d.id] = d;

    reset_data(d, ItemType::potion);
    d.id = ItemId::potion_poison;
    mod_spawn_chance(d, 0.66);
    data[(size_t)d.id] = d;

    reset_data(d, ItemType::potion);
    d.id = ItemId::potion_insight;
    mod_spawn_chance(d, 0.33);
    data[(size_t)d.id] = d;

    reset_data(d, ItemType::potion);
    d.id = ItemId::potion_clairv;
    mod_spawn_chance(d, 0.33);
    data[(size_t)d.id] = d;

    reset_data(d, ItemType::potion);
    d.id = ItemId::potion_r_fire;
    mod_spawn_chance(d, 0.33);
    data[(size_t)d.id] = d;

    reset_data(d, ItemType::potion);
    d.id = ItemId::potion_curing;
    mod_spawn_chance(d, 0.66);
    data[(size_t)d.id] = d;

    reset_data(d, ItemType::potion);
    d.id = ItemId::potion_descent;
    mod_spawn_chance(d, 0.66);
    data[(size_t)d.id] = d;

    reset_data(d, ItemType::potion);
    d.id = ItemId::potion_invis;
    data[(size_t)d.id] = d;

    reset_data(d, ItemType::potion);
    d.id = ItemId::potion_see_invis;
    mod_spawn_chance(d, 0.33);
    data[(size_t)d.id] = d;

    reset_data(d, ItemType::device);
    d.id = ItemId::device_blaster;
    d.base_name = {"Blaster Device", "Blaster Devices", "a Blaster Device"};
    d.value = ItemValue::minor_treasure;
    d.clr = clr_gray;
    d.native_containers.push_back(FeatureId::chest);
    d.native_containers.push_back(FeatureId::tomb);
    d.native_containers.push_back(FeatureId::cocoon);
    data[(size_t)d.id] = d;

    reset_data(d, ItemType::device);
    d.id = ItemId::device_shockwave;
    d.base_name =
    {
        "Shock Wave Device", "Shock Wave Devices", "a Shock Wave Device"
    };
    d.value = ItemValue::minor_treasure;
    d.clr = clr_gray;
    d.native_containers.push_back(FeatureId::chest);
    d.native_containers.push_back(FeatureId::tomb);
    d.native_containers.push_back(FeatureId::cocoon);
    data[(size_t)d.id] = d;

    reset_data(d, ItemType::device);
    d.id = ItemId::device_rejuvenator;
    d.base_name =
    {
        "Rejuvenator Device", "Rejuvenator Devices", "a Rejuvenator Device"
    };
    d.value = ItemValue::minor_treasure;
    d.clr = clr_gray;
    d.native_containers.push_back(FeatureId::chest);
    d.native_containers.push_back(FeatureId::tomb);
    d.native_containers.push_back(FeatureId::cocoon);
    data[(size_t)d.id] = d;

    reset_data(d, ItemType::device);
    d.id = ItemId::device_translocator;
    d.base_name =
    {
        "Translocator Device", "Translocator Devices", "a Translocator Device"
    };
    d.value = ItemValue::minor_treasure;
    d.clr = clr_gray;
    d.native_containers.push_back(FeatureId::chest);
    d.native_containers.push_back(FeatureId::tomb);
    d.native_containers.push_back(FeatureId::cocoon);
    data[(size_t)d.id] = d;

    reset_data(d, ItemType::device);
    d.id = ItemId::device_sentry_drone;
    d.base_name =
    {
        "Sentry Drone Device", "Sentry Drone Devices", "a Sentry Drone Device"
    };
    d.value = ItemValue::minor_treasure;
    d.clr = clr_gray;
    d.native_containers.push_back(FeatureId::chest);
    d.native_containers.push_back(FeatureId::tomb);
    d.native_containers.push_back(FeatureId::cocoon);
    data[(size_t)d.id] = d;

    reset_data(d, ItemType::device);
    d.id = ItemId::lantern;
    d.is_prio_in_backpack_list = true;
    d.base_name =
    {
        "Electric Lantern", "Electric Lanterns", "an Electric Lantern"
    };
    d.base_descr =
    {
        "A portable light source. It is somewhat unreliable, as it tends to "
        "malfunction."
    };
    d.spawn_std_range = Range(1, dlvl_last_mid_game);
    d.spawn_std_range = Range(1, 10);
    d.chance_to_incl_in_spawn_list = 50;
    d.is_identified = true;
    d.tile = TileId::lantern;
    d.clr = clr_yellow;
    d.is_ins_raied_while_carried = false;
    d.is_ins_raied_while_equiped = false;
    d.native_containers.push_back(FeatureId::chest);
    d.native_containers.push_back(FeatureId::cabinet);
    d.native_containers.push_back(FeatureId::cocoon);
    data[(size_t)d.id] = d;

    reset_data(d, ItemType::rod);
    d.id = ItemId::rod_purge_invis;
    data[(size_t)d.id] = d;

    reset_data(d, ItemType::rod);
    d.id = ItemId::rod_curing;
    data[(size_t)d.id] = d;

    reset_data(d, ItemType::rod);
    d.id = ItemId::rod_opening;
    d.spawn_std_range.max = dlvl_first_mid_game;
    data[(size_t)d.id] = d;

    reset_data(d, ItemType::rod);
    d.id = ItemId::rod_bless;
    data[(size_t)d.id] = d;

    reset_data(d, ItemType::rod);
    d.id = ItemId::rod_cloud_minds;
    data[(size_t)d.id] = d;

    reset_data(d, ItemType::general);
    d.id = ItemId::medical_bag;
    d.has_std_activate = true;
    d.is_prio_in_backpack_list = true;
    d.base_name = {"Medical Bag", "Medical Bags", "a Medical Bag"};
    d.base_descr =
    {
        "A portable bag of medical supplies. Can be used to treat Wounds or "
        "Infections."
    };
    d.weight = ItemWeight::medium;
    d.spawn_std_range = Range(1, dlvl_last_mid_game);
    d.is_stackable = false;
    d.glyph = '%';
    d.clr = clr_brown_drk;
    d.tile = TileId::medical_bag;
    data[(size_t)d.id] = d;

    reset_data(d, ItemType::amulet);
    d.id = ItemId::star_amulet;
    d.base_name = {"Star Amulet", "", "a Star Amulet"};
    d.base_name_un_id = d.base_name;
    d.clr = clr_white;
    data[(size_t)d.id] = d;

    reset_data(d, ItemType::amulet);
    d.id = ItemId::skull_amulet;
    d.base_name = {"Skull Amulet", "", "a Skull Amulet"};
    d.base_name_un_id = d.base_name;
    d.clr = clr_white;
    data[(size_t)d.id] = d;

    reset_data(d, ItemType::amulet);
    d.id = ItemId::spider_amulet;
    d.base_name = {"Spider Amulet", "", "a Spider Amulet"};
    d.base_name_un_id = d.base_name;
    d.clr = clr_white;
    data[(size_t)d.id] = d;

    reset_data(d, ItemType::amulet);
    d.id = ItemId::eye_amulet;
    d.base_name = {"Eye Amulet", "", "an Eye Amulet"};
    d.base_name_un_id = d.base_name;
    d.clr = clr_white;
    data[(size_t)d.id] = d;

    reset_data(d, ItemType::amulet);
    d.id = ItemId::moon_amulet;
    d.base_name = {"Moon Amulet", "", "a Moon Amulet"};
    d.base_name_un_id = d.base_name;
    d.clr = clr_white;
    data[(size_t)d.id] = d;

    reset_data(d, ItemType::amulet);
    d.id = ItemId::scarab_amulet;
    d.base_name = {"Scarab Amulet", "", "a Scarab Amulet"};
    d.base_name_un_id = d.base_name;
    d.clr = clr_white;
    data[(size_t)d.id] = d;

    reset_data(d, ItemType::amulet);
    d.id = ItemId::dagger_amulet;
    d.base_name = {"Dagger Amulet", "", "a Dagger Amulet"};
    d.base_name_un_id = d.base_name;
    d.clr = clr_white;
    data[(size_t)d.id] = d;

    reset_data(d, ItemType::amulet);
    d.id = ItemId::bat_winged_amulet;
    d.base_name = {"Bat-winged Amulet", "", "a Bat-winged Amulet"};
    d.base_name_un_id = d.base_name;
    d.clr = clr_white;
    data[(size_t)d.id] = d;

    reset_data(d, ItemType::amulet);
    d.id = ItemId::golden_amulet;
    d.base_name = {"Golden Amulet", "", "a Golden Amulet"};
    d.base_name_un_id = d.base_name;
    d.clr = clr_yellow;
    data[(size_t)d.id] = d;

    reset_data(d, ItemType::amulet);
    d.id = ItemId::silver_amulet;
    d.base_name = {"Silver Amulet", "", "a Silver Amulet"};
    d.base_name_un_id = d.base_name;
    d.clr = clr_white;
    data[(size_t)d.id] = d;

    reset_data(d, ItemType::amulet);
    d.id = ItemId::obsidian_amulet;
    d.base_name = {"Obsidian Amulet", "", "an Obsidian Amulet"};
    d.base_name_un_id = d.base_name;
    d.clr = clr_gray;
    data[(size_t)d.id] = d;

    reset_data(d, ItemType::amulet);
    d.id = ItemId::jade_amulet;
    d.base_name = {"Jade Amulet", "", "a Jade Amulet"};
    d.base_name_un_id = d.base_name;
    d.clr = clr_green_lgt;
    data[(size_t)d.id] = d;

    reset_data(d, ItemType::amulet);
    d.id = ItemId::rune_amulet;
    d.base_name = {"Rune Amulet", "", "a Rune Amulet"};
    d.base_name_un_id = d.base_name;
    d.clr = clr_white;
    data[(size_t)d.id] = d;
}

} //namespace

void init()
{
    TRACE_FUNC_BEGIN;

    init_data_list();

    TRACE_FUNC_END;
}

void cleanup()
{
    TRACE_FUNC_BEGIN;

    for (size_t i = 0; i < (size_t)ItemId::END; ++i)
    {
        ItemDataT& d = data[i];

        delete d.melee.prop_applied;
        d.melee.prop_applied = nullptr;

        delete d.ranged.prop_applied;
        d.ranged.prop_applied = nullptr;
    }

    TRACE_FUNC_END;
}


void save()
{
    for (size_t i = 0; i < (size_t)ItemId::END; ++i)
    {
        const ItemDataT& d = data[i];

        saving::put_bool(d.is_identified);
        saving::put_bool(d.is_tried);
        saving::put_bool(d.allow_spawn);
    }
}

void load()
{
    for (size_t i = 0; i < (size_t)ItemId::END; ++i)
    {
        ItemDataT& d = data[i];

        d.is_identified   = saving::get_bool();
        d.is_tried        = saving::get_bool();
        d.allow_spawn     = saving::get_bool();
    }
}

} // item_data
