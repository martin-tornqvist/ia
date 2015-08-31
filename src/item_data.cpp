#include "item_data.hpp"

#include <iostream>
#include <climits>

#include "init.hpp"
#include "colors.hpp"
#include "item.hpp"
#include "cmn_data.hpp"
#include "actor_data.hpp"
#include "item_scroll.hpp"
#include "item_potion.hpp"
#include "actor_player.hpp"
#include "sound.hpp"
#include "item_device.hpp"
#include "map.hpp"

using namespace std;

Item_data_t::Item_data_t() :
    id                                  (Item_id::END),
    type                                (Item_type::general),
    has_std_activate                    (false),
    is_prio_in_backpack_list            (false),
    value                               (Item_value::normal),
    weight                              (Item_weight::none),
    allow_spawn                         (true),
    spawn_std_range                     (Range(1, INT_MAX)),
    max_stack_at_spawn                  (1),
    chance_to_incl_in_floor_spawn_list  (100),
    is_stackable                        (true),
    is_identified                       (true),
    is_tried                            (false),
    base_name                           (),
    glyph                               ('X'),
    clr                                 (clr_white),
    tile                                (Tile_id::empty),
    main_att_mode                       (Main_att_mode::none),
    spell_cast_from_scroll              (Spell_id::END),
    land_on_hard_snd_msg                ("I hear a thudding sound."),
    land_on_hard_sfx                    (),
    shock_while_in_backpack             (0),
    shock_while_equipped                (0),
    melee                               (Item_melee_data()),
    ranged                              (Item_ranged_data()),
    armor                               (Item_armor_data())
{
    for (int i = 0; i < int(Ability_id::END); ++i)
    {
        ability_mods_while_equipped[i] = 0;
    }

    base_descr.clear();
    native_rooms.clear();
    container_spawn_rules.clear();
}

Item_data_t::Item_melee_data::Item_melee_data() :
    is_melee_wpn                        (false),
    dmg                                 (pair<int, int>(0, 0)),
    hit_chance_mod                      (0),
    att_msgs                            (Item_att_msgs()),
    prop_applied                        (nullptr),
    dmg_type                            (Dmg_type::physical),
    knocks_back                         (false),
    hit_small_sfx                       (Sfx_id::END),
    hit_medium_sfx                      (Sfx_id::END),
    hit_hard_sfx                        (Sfx_id::END),
    miss_sfx                            (Sfx_id::END) {}

Item_data_t::Item_melee_data::~Item_melee_data()
{
    if (prop_applied)
    {
        delete prop_applied;
    }
}

Item_data_t::Item_ranged_data::Item_ranged_data() :
    is_ranged_wpn                       (false),
    is_machine_gun                      (false),
    is_shotgun                          (false),
    max_ammo                            (0),
    dmg                                 (Dice_param()),
    throw_dmg                           (Dice_param()),
    hit_chance_mod                      (0),
    throw_hit_chance_mod                (0),
    effective_range                     (3),
    knocks_back                         (false),
    dmg_info_override                   (""),
    ammo_item_id                        (Item_id::END),
    dmg_type                            (Dmg_type::physical),
    has_infinite_ammo                   (false),
    projectile_glyph                    ('/'),
    projectile_tile                     (Tile_id::projectile_std_front_slash),
    projectile_clr                      (clr_white),
    projectile_leaves_trail             (false),
    projectile_leaves_smoke             (false),
    att_msgs                            (Item_att_msgs()),
    snd_msg                             (""),
    snd_vol                             (Snd_vol::low),
    makes_ricochet_snd                  (false),
    att_sfx                             (Sfx_id::END),
    reload_sfx                          (Sfx_id::END),
    prop_applied                        (nullptr) {}

Item_data_t::Item_ranged_data::~Item_ranged_data()
{
    if (prop_applied)
    {
        delete prop_applied;
    }
}

Item_data_t::Item_armor_data::Item_armor_data() :
    armor_points(0),
    dmg_to_durability_factor(0.0) {}

namespace item_data
{

Item_data_t data[int(Item_id::END)];

namespace
{

void add_feature_found_in(Item_data_t& data, const Feature_id feature_id,
                          const int CHANCE_TO_INCL = 100)
{
    data.container_spawn_rules.push_back({feature_id, CHANCE_TO_INCL});
}

//------------------------------- ITEM ARCHETYPES (DEFAULTS)
void reset_data(Item_data_t& d, Item_type const item_type)
{
    switch (item_type)
    {
    case Item_type::general:
        d = Item_data_t();
        break;

    case Item_type::melee_wpn:
        reset_data(d, Item_type::general);
        d.type = Item_type::melee_wpn;
        d.is_stackable = false;
        d.weight = Item_weight::medium;
        d.glyph = '(';
        d.clr = clr_white;
        d.main_att_mode = Main_att_mode::melee;
        d.melee.is_melee_wpn = true;
        d.melee.miss_sfx = Sfx_id::miss_medium;
        d.melee.hit_small_sfx = Sfx_id::hit_small;
        d.melee.hit_medium_sfx = Sfx_id::hit_medium;
        d.melee.hit_hard_sfx = Sfx_id::hit_hard;
        break;

    case Item_type::melee_wpn_intr:
        reset_data(d, Item_type::melee_wpn);
        d.type = Item_type::melee_wpn_intr;
        d.spawn_std_range = Range(-1, -1);
        d.chance_to_incl_in_floor_spawn_list = 0;
        d.allow_spawn = false;
        d.melee.hit_small_sfx = Sfx_id::hit_small;
        d.melee.hit_medium_sfx = Sfx_id::hit_medium;
        d.melee.hit_hard_sfx = Sfx_id::hit_hard;
        d.melee.miss_sfx = Sfx_id::END;
        break;

    case Item_type::ranged_wpn:
        reset_data(d, Item_type::general);
        d.type = Item_type::ranged_wpn;
        d.is_stackable = false;
        d.weight = Item_weight::medium;
        d.glyph = '}';
        d.clr = clr_white;
        d.melee.is_melee_wpn = true;
        d.melee.dmg = pair<int, int>(1, 6);
        d.main_att_mode = Main_att_mode::ranged;
        d.ranged.is_ranged_wpn = true;
        d.ranged.projectile_glyph = '/';
        d.ranged.projectile_clr = clr_white;
        d.spawn_std_range.max = DLVL_LAST_MID_GAME;
        d.melee.hit_small_sfx = Sfx_id::hit_small;
        d.melee.hit_medium_sfx = Sfx_id::hit_medium;
        d.melee.hit_hard_sfx = Sfx_id::hit_hard;
        d.melee.miss_sfx = Sfx_id::miss_medium;
        d.ranged.snd_vol = Snd_vol::high;
        break;

    case Item_type::ranged_wpn_intr:
        reset_data(d, Item_type::ranged_wpn);
        d.type = Item_type::ranged_wpn_intr;
        d.ranged.has_infinite_ammo = true;
        d.spawn_std_range = Range(-1, -1);
        d.chance_to_incl_in_floor_spawn_list = 0;
        d.allow_spawn = false;
        d.melee.is_melee_wpn = false;
        d.ranged.projectile_glyph = '*';
        d.ranged.snd_vol = Snd_vol::low;
        break;

    case Item_type::throwing_wpn:
        reset_data(d, Item_type::general);
        d.type = Item_type::throwing_wpn;
        d.weight = Item_weight::extra_light;
        d.is_stackable = true;
        d.spawn_std_range.max = DLVL_LAST_MID_GAME;
        d.ranged.snd_vol = Snd_vol::low;
        break;

    case Item_type::ammo:
        reset_data(d, Item_type::general);
        d.type = Item_type::ammo;
        d.weight = Item_weight::extra_light;
        d.glyph = '{';
        d.clr = clr_white;
        d.tile = Tile_id::ammo;
        d.spawn_std_range.max = DLVL_LAST_MID_GAME;
        break;

    case Item_type::ammo_clip:
        reset_data(d, Item_type::ammo);
        d.type = Item_type::ammo_clip;
        d.weight = Item_weight::light;
        d.is_stackable = false;
        d.spawn_std_range.max = DLVL_LAST_MID_GAME;
        break;

    case Item_type::scroll:
        reset_data(d, Item_type::general);
        d.type = Item_type::scroll;
        d.has_std_activate = true;
        d.base_descr =
        {
            "A short transcription of an eldritch incantation. There is a strange aura "
            "about it, as if some power was imbued in the paper itself.",
            "It should be possible to pronounce it correctly, but the purpose is unclear."
        };
        d.value = Item_value::minor_treasure;
        d.chance_to_incl_in_floor_spawn_list = 40;
        d.weight = Item_weight::none;
        d.is_identified = false;
        d.glyph = '?';
        d.clr = clr_white;
        d.tile = Tile_id::scroll;
        d.max_stack_at_spawn = 1;
        d.land_on_hard_snd_msg = "";
        add_feature_found_in(d, Feature_id::chest);
        add_feature_found_in(d, Feature_id::tomb);
        add_feature_found_in(d, Feature_id::cabinet, 25);
        add_feature_found_in(d, Feature_id::cocoon, 25);
        break;

    case Item_type::potion:
        reset_data(d, Item_type::general);
        d.type = Item_type::potion;
        d.has_std_activate = true;
        d.base_descr =
        {
            "A small glass bottle containing a mysterious concoction."
        };
        d.value = Item_value::minor_treasure;
        d.chance_to_incl_in_floor_spawn_list = 55;
        d.weight = Item_weight::light;
        d.is_identified = false;
        d.glyph = '!';
        d.tile = Tile_id::potion;
        d.ranged.throw_hit_chance_mod = 15;
        d.ranged.throw_dmg = Dice_param(1, 3, 0);
        d.max_stack_at_spawn = 2;
        d.land_on_hard_snd_msg = "";
        add_feature_found_in(d, Feature_id::chest);
        add_feature_found_in(d, Feature_id::tomb);
        add_feature_found_in(d, Feature_id::cabinet, 25);
        add_feature_found_in(d, Feature_id::cocoon, 25);
        break;

    case Item_type::device:
        reset_data(d, Item_type::general);
        d.type = Item_type::device;
        d.has_std_activate = true;
        d.base_name_un_id = {"Strange Device", "Strange Devices", "a Strange Device"};
        d.base_descr =
        {
            "A small piece of machinery. It could not possibly have been designed by a "
            "human mind. Even for its small size, it seems incredibly complex. There is "
            "no hope of understanding the purpose or function of it through normal means."
        };
        d.chance_to_incl_in_floor_spawn_list = 12;
        d.weight = Item_weight::light;
        d.is_identified = false;
        d.glyph = '~';
        d.tile = Tile_id::device1;
        d.is_stackable = false;
        d.land_on_hard_snd_msg = "I hear a clanking sound.";
        d.land_on_hard_sfx = Sfx_id::metal_clank;
        add_feature_found_in(d, Feature_id::chest, 10);
        break;

    case Item_type::armor:
        reset_data(d, Item_type::general);
        d.type = Item_type::armor;
        d.weight = Item_weight::heavy;
        d.glyph = '[';
        d.tile = Tile_id::armor;
        d.is_stackable = false;
        break;

    case Item_type::head_wear:
        reset_data(d, Item_type::general);
        d.type = Item_type::head_wear;
        d.glyph = '[';
        d.is_stackable = false;
        break;

    case Item_type::amulet:
        reset_data(d, Item_type::general);
        d.type = Item_type::amulet;
        d.value = Item_value::major_treasure;
        d.shock_while_in_backpack = d.shock_while_equipped = 10;
        d.tile = Tile_id::amulet;
        d.glyph = '\"';
        d.weight = Item_weight::light;
        d.is_identified = false;
        d.is_stackable = false;
        d.chance_to_incl_in_floor_spawn_list = 1;
        add_feature_found_in(d, Feature_id::tomb, 16);
        add_feature_found_in(d, Feature_id::chest, 5);
        break;

    case Item_type::ring:
        reset_data(d, Item_type::general);
        d.type = Item_type::ring;
        d.value = Item_value::major_treasure;
        d.shock_while_in_backpack = d.shock_while_equipped = 10;
        d.tile = Tile_id::ring;
        d.glyph = '=';
        d.weight = Item_weight::extra_light;
        d.is_identified = false;
        d.is_stackable = false;
        d.chance_to_incl_in_floor_spawn_list = 1;
        add_feature_found_in(d, Feature_id::tomb, 16);
        add_feature_found_in(d, Feature_id::chest, 5);
        break;

    case Item_type::explosive:
        reset_data(d, Item_type::general);
        d.type = Item_type::explosive;
        d.has_std_activate = true;
        d.weight = Item_weight::light;
        d.glyph = '-';
        d.max_stack_at_spawn = 2;
        d.land_on_hard_snd_msg = "";
        break;

    default:
        break;
    }
}

void set_dmg_from_mon_id(Item_data_t& item_data, const Actor_id id)
{
    const auto& actor_data      = actor_data::data[int(id)];
    item_data.melee.dmg         = pair<int, int>(1, actor_data.dmg_melee);
    item_data.ranged.dmg        = Dice_param(1, actor_data.dmg_ranged, 0);
    item_data.ranged.throw_dmg  = Dice_param(1, actor_data.dmg_ranged, 0);
}

//------------------------------- LIST OF ITEMS
void init_data_list()
{
    Item_data_t d;

    reset_data(d, Item_type::general);
    d.id = Item_id::trapezohedron;;
    d.base_name =
    {
        "Shining Trapezohedron", "Shining Trapezohedrons", "The Shining Trapezohedron"
    };
    d.spawn_std_range = Range(-1, -1);
    d.chance_to_incl_in_floor_spawn_list = 0;
    d.allow_spawn = false;
    d.is_stackable = false;
    d.glyph = '*';
    d.clr = clr_red_lgt;
    d.tile = Tile_id::trapezohedron;
    data[size_t(d.id)] = d;

    reset_data(d, Item_type::ranged_wpn);
    d.id = Item_id::sawed_off;
    d.base_name =
    {
        "Sawed-off Shotgun", "Sawed-off shotguns", "a Sawed-off Shotgun"
    };
    d.base_descr =
    {
        "Compared to a standard shotgun, the sawed-off has a shorter effective range. "
        "At close range it is more devastating however. It holds two barrels, and needs "
        "to be reloaded after both are discharged"
    };
    d.weight = Item_weight::medium;
    d.tile = Tile_id::shotgun;
    d.ranged.is_shotgun = true;
    d.melee.att_msgs = {"strike", "strikes me with a shotgun"};
    d.ranged.max_ammo = 2;
    d.ranged.dmg = Dice_param(8, 3);
    d.ranged.hit_chance_mod = -5;
    d.ranged.effective_range = 3;
    d.ranged.ammo_item_id = Item_id::shotgun_shell;
    d.ranged.att_msgs = {"fire", "fires a shotgun"};
    d.ranged.snd_msg = "I hear a shotgun blast.";
    d.ranged.att_sfx = Sfx_id::shotgun_sawed_off_fire;
    d.ranged.makes_ricochet_snd = true;
    d.ranged.reload_sfx = Sfx_id::shotgun_reload;
    add_feature_found_in(d, Feature_id::chest);
    add_feature_found_in(d, Feature_id::cabinet);
    add_feature_found_in(d, Feature_id::cocoon);
    data[size_t(d.id)] = d;

    reset_data(d, Item_type::ranged_wpn);
    d.id = Item_id::pump_shotgun;
    d.base_name = {"Pump Shotgun", "Pump shotguns", "a Pump Shotgun"};
    d.base_descr =
    {
        "A pump-action shotgun has a handgrip that can be pumped back and forth in "
        "order to eject a spent round of ammunition and to chamber a fresh one. It has "
        "a single barrel above a tube magazine into which shells are inserted. The "
        "magazine has a capacity of 8 shells."
    };
    d.weight = Item_weight::medium;
    d.tile = Tile_id::shotgun;
    d.ranged.is_shotgun = true;
    d.melee.att_msgs = {"strike", "strikes me with a shotgun"};
    d.ranged.max_ammo = 8;
    d.ranged.dmg = Dice_param(6, 3);
    d.ranged.hit_chance_mod = 5;
    d.ranged.effective_range = 5;
    d.ranged.ammo_item_id = Item_id::shotgun_shell;
    d.ranged.att_msgs = {"fire", "fires a shotgun"};
    d.ranged.snd_msg = "I hear a shotgun blast.";
    d.ranged.att_sfx = Sfx_id::shotgun_pump_fire ;
    d.ranged.makes_ricochet_snd = true;
    d.ranged.reload_sfx = Sfx_id::shotgun_reload;
    add_feature_found_in(d, Feature_id::chest);
    add_feature_found_in(d, Feature_id::cabinet);
    add_feature_found_in(d, Feature_id::cocoon);
    data[size_t(d.id)] = d;

    reset_data(d, Item_type::ammo);
    d.id = Item_id::shotgun_shell;
    d.base_name = {"Shotgun shell", "Shotgun shells", "a shotgun shell"};
    d.base_descr =
    {
        "A cartridge designed to be fired from a shotgun."
    };
    d.max_stack_at_spawn = 10;
    add_feature_found_in(d, Feature_id::chest);
    add_feature_found_in(d, Feature_id::cabinet);
    add_feature_found_in(d, Feature_id::cocoon);
    data[size_t(d.id)] = d;

    reset_data(d, Item_type::ranged_wpn);
    d.id = Item_id::incinerator;
    d.base_name = {"Incinerator", "Incinerators", "an Incinerator"};
    d.base_descr =
    {
        "This hellish, experimental weapon launches an explosive fireball. Best used "
        "with extreme caution."
    };
    d.weight = Item_weight((Item_weight::medium + Item_weight::heavy) / 2);
    d.tile = Tile_id::incinerator;
    d.melee.att_msgs = {"strike", "strikes me with an Incinerator"};
    d.ranged.max_ammo = 5;
    d.ranged.dmg = Dice_param(1, 3);
    d.ranged.effective_range = 8;
    d.ranged.dmg_info_override = "* ";
    d.ranged.ammo_item_id = Item_id::incinerator_ammo;
    d.ranged.att_msgs = {"fire", "fires an incinerator"};
    d.ranged.snd_msg = "I hear the blast of a launched missile.";
    d.ranged.projectile_glyph = '*';
    d.ranged.projectile_clr = clr_red_lgt;
    d.spawn_std_range.min = 5;
    d.chance_to_incl_in_floor_spawn_list = 25;
    add_feature_found_in(d, Feature_id::chest, 25);
    add_feature_found_in(d, Feature_id::cabinet, 25);
    add_feature_found_in(d, Feature_id::cocoon, 25);
    data[size_t(d.id)] = d;

    reset_data(d, Item_type::ammo_clip);
    d.id = Item_id::incinerator_ammo;
    d.base_name =
    {
        "Incinerator Cartridge", "Incinerator Cartridges", "an Incinerator Cartridge"
    };
    d.base_descr =
    {
        "Ammunition designed for Incinerators."
    };
    d.weight = Item_weight::light;
    d.spawn_std_range.min = 5;
    d.max_stack_at_spawn = 1;
    d.ranged.max_ammo = data[size_t(Item_id::incinerator)].ranged.max_ammo;
    d.chance_to_incl_in_floor_spawn_list = 25;
    add_feature_found_in(d, Feature_id::chest, 25);
    add_feature_found_in(d, Feature_id::cabinet, 25);
    add_feature_found_in(d, Feature_id::cocoon, 25);
    data[size_t(d.id)] = d;

    reset_data(d, Item_type::ranged_wpn);
    d.id = Item_id::machine_gun;
    d.base_name = {"Tommy Gun", "Tommy Guns", "a Tommy Gun"};
    d.base_descr =
    {
        "\"Tommy Gun\" is a nickname for the Thompson submachine gun-an automatic "
        "firearm with a drum magazine and verical foregrip. It fires .45 ACP "
        "ammunition. The drum magazine has a capacity of 50 rounds."
    };
    d.weight = Item_weight::medium;
    d.tile = Tile_id::tommy_gun;
    d.melee.att_msgs = {"strike", "strikes me with a Tommy Gun"};
    d.ranged.is_machine_gun = true;
    d.ranged.max_ammo = 50;
    d.ranged.dmg = Dice_param(2, 2, 2);
    d.ranged.hit_chance_mod = -10;
    d.ranged.effective_range = 8;
    d.ranged.ammo_item_id = Item_id::drum_of_bullets;
    d.ranged.att_msgs = {"fire", "fires a Tommy Gun"};
    d.ranged.snd_msg = "I hear the burst of a machine gun.";
    d.ranged.att_sfx = Sfx_id::machine_gun_fire;
    d.ranged.makes_ricochet_snd = true;
    d.ranged.reload_sfx = Sfx_id::machine_gun_reload;
    add_feature_found_in(d, Feature_id::chest);
    add_feature_found_in(d, Feature_id::cabinet);
    add_feature_found_in(d, Feature_id::cocoon);
    data[size_t(d.id)] = d;

    reset_data(d, Item_type::ammo_clip);
    d.id = Item_id::drum_of_bullets;
    d.base_name = {"Drum of .45 ACP", "Drums of .45 ACP", "a Drum of .45 ACP"};
    d.base_descr =
    {
        "Ammunition used by Tommy Guns."
    };
    d.ranged.max_ammo = data[size_t(Item_id::machine_gun)].ranged.max_ammo;
    add_feature_found_in(d, Feature_id::chest);
    add_feature_found_in(d, Feature_id::cabinet);
    add_feature_found_in(d, Feature_id::cocoon);
    data[size_t(d.id)] = d;

    reset_data(d, Item_type::ranged_wpn);
    d.id = Item_id::pistol;
    d.base_name = {"M1911 Colt", "M1911 Colt", "an M1911 Colt"};
    d.base_descr =
    {
        "A semi-automatic, magazine-fed pistol chambered for the .45 ACP cartridge."
    };
    d.weight = Item_weight((Item_weight::light + Item_weight::medium) / 2);
    d.tile = Tile_id::pistol;
    d.ranged.max_ammo = 7;
    d.ranged.dmg = Dice_param(1, 8, 4);
    d.ranged.effective_range = 6;
    d.ranged.ammo_item_id = Item_id::pistol_clip;
    d.melee.att_msgs = {"strike", "strikes me with a pistol"};
    d.ranged.att_msgs = {"fire", "fires a pistol"};
    d.ranged.snd_msg = "I hear a pistol being fired.";
    d.ranged.att_sfx = Sfx_id::pistol_fire;
    d.ranged.makes_ricochet_snd = true;
    d.ranged.reload_sfx = Sfx_id::pistol_reload;
    add_feature_found_in(d, Feature_id::chest);
    add_feature_found_in(d, Feature_id::cabinet);
    add_feature_found_in(d, Feature_id::cocoon);
    data[size_t(d.id)] = d;

    reset_data(d, Item_type::ammo_clip);
    d.id = Item_id::pistol_clip;
    d.base_name = {".45ACP Colt cartridge", ".45ACP Colt cartridges",
                   "a .45ACP Colt cartridge"
                  };
    d.base_descr =
    {
        "Ammunition used by Colt pistols."
    };
    d.ranged.max_ammo = data[size_t(Item_id::pistol)].ranged.max_ammo;
    add_feature_found_in(d, Feature_id::chest);
    add_feature_found_in(d, Feature_id::cabinet);
    add_feature_found_in(d, Feature_id::cocoon);
    data[size_t(d.id)] = d;

    reset_data(d, Item_type::ranged_wpn);
    d.id = Item_id::mi_go_gun;
    d.base_name =
    {
        "Electric Gun", "Electric Gun", "an Electric Gun"
    };
    d.base_descr =
    {
        "A weapon created by the Mi-go. It fires devastating bolts of electricity.",
        "If there is no ammunition to fuel the weapon, it can draw power from the "
        "essence of the wielder (press [f] while no ammo loaded)."
    };
    d.spawn_std_range = Range(-1, -1);
    d.weight = Item_weight::medium;
    d.tile = Tile_id::mi_go_gun;
    d.clr = clr_yellow;
    d.ranged.max_ammo = 16;
    d.ranged.dmg = Dice_param(3, 6, 0);
    d.ranged.hit_chance_mod = 5;
    d.ranged.effective_range = 4;
    d.ranged.prop_applied = new Prop_paralyzed(Prop_turns::specific, 2);
    d.ranged.dmg_type = Dmg_type::electric;
    d.ranged.ammo_item_id = Item_id::mi_go_gun_ammo;
    d.ranged.has_infinite_ammo = false;
    d.ranged.projectile_leaves_trail = true;
    d.ranged.projectile_clr = clr_yellow;
    d.melee.att_msgs = {"strike", "strikes me with a Mi-go Electric Gun"};
    d.ranged.att_msgs = {"fire", "fires a Mi-go Electric Gun"};
    d.ranged.snd_msg = "I hear a bolt of electricity.";
    d.ranged.att_sfx = Sfx_id::mi_go_gun_fire;
    d.ranged.reload_sfx = Sfx_id::machine_gun_reload;
    d.ranged.makes_ricochet_snd = false;
    data[size_t(d.id)] = d;

    reset_data(d, Item_type::ammo_clip);
    d.id = Item_id::mi_go_gun_ammo;
    d.base_name =
    {
        "Mi-go Electric Cell", "Mi-go Electric Cells", "a Mi-go Electric Cell"
    };
    d.base_descr =
    {
        "Ammunition for the Mi-go Electric gun."
    };
    d.ranged.max_ammo = data[size_t(Item_id::mi_go_gun)].ranged.max_ammo;
    d.clr = clr_yellow;
    d.spawn_std_range = Range(-1, -1);
    data[size_t(d.id)] = d;

    reset_data(d, Item_type::ranged_wpn);
    d.id = Item_id::flare_gun;
    d.base_name = {"Flare Gun", "Flare Gun", "a Flare Gun"};
    d.base_descr =
    {
        "Launches flares. Not designed to function as a weapon."
    };
    d.weight = Item_weight((Item_weight::light + Item_weight::medium) / 2);
    d.tile = Tile_id::flare_gun;
    d.ranged.max_ammo = 1;
    d.ranged.dmg = Dice_param(1, 3, 0);
    d.ranged.effective_range = 3;
    d.ranged.dmg_info_override = "*";
    d.ranged.ammo_item_id = Item_id::flare;
    d.melee.att_msgs = {"strike", "strikes me with a flare gun"};
    d.ranged.att_msgs = {"fire", "fires a flare gun"};
    d.ranged.snd_msg = "I hear a flare gun being fired.";
    d.ranged.prop_applied = new Prop_flared(Prop_turns::std);
    add_feature_found_in(d, Feature_id::chest);
    add_feature_found_in(d, Feature_id::cabinet);
    add_feature_found_in(d, Feature_id::cocoon);
    data[size_t(d.id)] = d;

    reset_data(d, Item_type::ranged_wpn);
    d.id = Item_id::spike_gun;
    d.base_name = {"Spike Gun", "Spike Guns", "a Spike Gun"};
    d.base_descr =
    {
        "A very strange and crude weapon capable of launching iron spikes with enough "
        "force to pierce flesh (or even rock). It seems almost to be deliberately "
        "designed for cruelty, rather than pure stopping power."
    };
    d.weight = Item_weight::medium;
    d.tile = Tile_id::tommy_gun;
    d.clr = clr_blue_lgt;
    d.melee.att_msgs = {"strike", "strikes me with a Spike Gun"};
    d.ranged.max_ammo = 12;
    d.ranged.dmg = Dice_param(1, 7, 0);
    d.ranged.hit_chance_mod = 0;
    d.ranged.effective_range = 4;
    d.ranged.dmg_type = Dmg_type::physical;
    d.ranged.knocks_back = true;
    d.ranged.ammo_item_id = Item_id::iron_spike;
    d.ranged.att_msgs = {"fire", "fires a Spike Gun"};
    d.ranged.snd_msg = "I hear a very crude weapon being fired.";
    d.ranged.makes_ricochet_snd = true;
    d.ranged.projectile_glyph = '/';
    d.ranged.projectile_clr = clr_gray;
    d.spawn_std_range.min = 4;
    d.ranged.att_sfx = Sfx_id::spike_gun;
    d.ranged.snd_vol = Snd_vol::low;
    add_feature_found_in(d, Feature_id::chest, 50);
    add_feature_found_in(d, Feature_id::cabinet, 50);
    add_feature_found_in(d, Feature_id::cocoon, 50);
    data[size_t(d.id)] = d;

    reset_data(d, Item_type::ranged_wpn);
    d.id = Item_id::trap_dart;
    d.allow_spawn = false;
    d.ranged.has_infinite_ammo = true;
    d.ranged.dmg = Dice_param(1, 8);
    d.ranged.hit_chance_mod = 70;
    d.ranged.effective_range = 6;
    d.ranged.snd_msg = "I hear the launching of a projectile.";
    d.ranged.att_sfx = Sfx_id::END; //TODO: Make a sound effect for this
    d.ranged.makes_ricochet_snd = true;
    data[size_t(d.id)] = d;

    reset_data(d, Item_type::ranged_wpn);
    d = data[size_t(Item_id::trap_dart)];
    d.id = Item_id::trap_dart_poison;
    d.ranged.prop_applied = new Prop_poisoned(Prop_turns::std);
    data[size_t(d.id)] = d;

    reset_data(d, Item_type::melee_wpn);
    d.id = Item_id::trap_spear;
    d.allow_spawn = false;
    d.weight = Item_weight::heavy;
    d.melee.dmg = pair<int, int>(2, 6);
    d.melee.hit_chance_mod = 85;
    d.melee.hit_small_sfx = Sfx_id::hit_sharp;
    d.melee.hit_medium_sfx = Sfx_id::hit_sharp;
    d.melee.miss_sfx = Sfx_id::miss_heavy;
    data[size_t(d.id)] = d;

    reset_data(d, Item_type::ranged_wpn);
    d = data[size_t(Item_id::trap_spear)];
    d.id = Item_id::trap_spear_poison;
    d.ranged.prop_applied = new Prop_poisoned(Prop_turns::std);
    data[size_t(d.id)] = d;

    reset_data(d, Item_type::explosive);
    d.id = Item_id::dynamite;
    d.base_name = {"Dynamite", "Sticks of Dynamite", "a Stick of Dynamite"};
    d.base_descr =
    {
        "An explosive material based on nitroglycerin. The name comes from the ancient "
        "Greek word for \"power\"."
    };
    d.weight = Item_weight::light;
    d.tile = Tile_id::dynamite;
    d.clr = clr_red_lgt;
    add_feature_found_in(d, Feature_id::chest);
    add_feature_found_in(d, Feature_id::cabinet);
    add_feature_found_in(d, Feature_id::cocoon);
    data[size_t(d.id)] = d;

    reset_data(d, Item_type::explosive);
    d.id = Item_id::flare;
    d.base_name = {"Flare", "Flares", "a Flare"};
    d.base_descr =
    {
        "A type of pyrotechnic that produces a brilliant light or intense heat without "
        "an explosion."
    };
    d.weight = Item_weight::light;
    d.tile = Tile_id::flare;
    d.clr = clr_gray;
    add_feature_found_in(d, Feature_id::chest);
    add_feature_found_in(d, Feature_id::cabinet);
    add_feature_found_in(d, Feature_id::cocoon);
    data[size_t(d.id)] = d;

    reset_data(d, Item_type::explosive);
    d.id = Item_id::molotov;
    d.base_name = {"Molotov Cocktail", "Molotov Cocktails", "a Molotov Cocktail"};
    d.base_descr =
    {
        "An improvised incendiary weapon made of a glass bottle containing flammable "
        "liquid and some cloth for ignition. In action, the cloth is lit and the bottle "
        "hurled at a target, causing an immediate fireball followed by a raging fire."
    };
    d.weight = Item_weight::light;
    d.tile = Tile_id::molotov;
    d.clr = clr_white;
    add_feature_found_in(d, Feature_id::chest);
    add_feature_found_in(d, Feature_id::cabinet);
    add_feature_found_in(d, Feature_id::cocoon);
    data[size_t(d.id)] = d;

    reset_data(d, Item_type::explosive);
    d.id = Item_id::smoke_grenade;
    d.base_name = {"Smoke Grenade", "Smoke Grenades", "a Smoke Grenade"};
    d.base_descr =
    {
        "A sheet steel cylinder with emission holes releasing smoke when the grenade is "
        "ignited. Their primary use is to create smoke screens for concealment. "
        "The fumes produced can harm the eyes, throat and lungs - so it is "
        "recommended to wear a protective mask."
    };
    d.weight = Item_weight::light;
    d.tile = Tile_id::flare;
    d.clr = clr_green;
    add_feature_found_in(d, Feature_id::chest);
    add_feature_found_in(d, Feature_id::cabinet);
    data[size_t(d.id)] = d;

    reset_data(d, Item_type::throwing_wpn);
    d.id = Item_id::thr_knife;
    d.base_name = {"Throwing Knife", "Throwing Knives", "a Throwing Knife"};
    d.base_descr =
    {
        "A knife specially designed and weighted so that it can be thrown effectively."
    };
    d.weight = Item_weight::extra_light;
    d.tile = Tile_id::dagger;
    d.glyph = '/';
    d.clr = clr_white;
    d.ranged.throw_hit_chance_mod = 0;
    d.ranged.throw_dmg = Dice_param(2, 4);
    d.ranged.effective_range = 5;
    d.max_stack_at_spawn = 8;
    d.land_on_hard_snd_msg = "I hear a clanking sound.";
    d.land_on_hard_sfx = Sfx_id::metal_clank;
    d.main_att_mode = Main_att_mode::thrown;
    add_feature_found_in(d, Feature_id::chest);
    add_feature_found_in(d, Feature_id::cabinet);
    add_feature_found_in(d, Feature_id::cocoon);
    data[size_t(d.id)] = d;

    reset_data(d, Item_type::throwing_wpn);
    d.id = Item_id::rock;
    d.base_name = {"Rock", "Rocks", "a Rock"};
    d.base_descr =
    {
        "Although not a very impressive weapon, with skill they can be used with some "
        "result."
    };
    d.weight = Item_weight::extra_light;
    d.tile = Tile_id::rock;
    d.glyph = '*';
    d.clr = clr_gray;
    d.ranged.throw_hit_chance_mod = 10;
    d.ranged.throw_dmg = Dice_param(1, 3);
    d.ranged.effective_range = 4;
    d.max_stack_at_spawn = 6;
    d.main_att_mode = Main_att_mode::thrown;
    add_feature_found_in(d, Feature_id::cabinet);
    add_feature_found_in(d, Feature_id::cocoon);
    data[size_t(d.id)] = d;

    reset_data(d, Item_type::melee_wpn);
    d.id = Item_id::dagger;
    d.base_name = {"Dagger", "Daggers", "a Dagger"};
    d.base_descr =
    {
        "Commonly associated with deception, stealth, and treachery. Many "
        "assassinations have been carried out with the use of a dagger.",

        "Attacking an unaware opponent with a dagger does 300% damage (instead of the "
        "normal 150% damage from stealth attacks).",

        "Melee attacks with daggers are silent."
    };
    d.weight = Item_weight::light;
    d.tile = Tile_id::dagger;
    d.melee.att_msgs = {"stab", "stabs me with a Dagger"};
    d.melee.dmg = pair<int, int>(1, 4);
    d.melee.hit_chance_mod = 20;
    d.melee.hit_medium_sfx = Sfx_id::hit_sharp;
    d.melee.hit_hard_sfx = Sfx_id::hit_sharp;
    d.melee.miss_sfx = Sfx_id::miss_light;
    add_feature_found_in(d, Feature_id::chest);
    add_feature_found_in(d, Feature_id::cabinet);
    add_feature_found_in(d, Feature_id::tomb);
    add_feature_found_in(d, Feature_id::cocoon);
    data[size_t(d.id)] = d;

    reset_data(d, Item_type::melee_wpn);
    d.id = Item_id::hatchet;
    d.base_name = {"Hatchet", "Hatchets", "a Hatchet"};
    d.base_descr =
    {
        "A small axe with a short handle. Hatchets are reliable weapons - they are easy "
        "to use, and cause decent damage for their low weight. "
        /*TODO: "They can also serve well as thrown weapons."*/,

        "Melee attacks with hatchets are silent."
    };
    d.weight = Item_weight::light;
    d.tile = Tile_id::axe;
    d.melee.att_msgs = {"strike", "strikes me with a Hatchet"};
    d.melee.dmg = pair<int, int>(1, 5);
    d.melee.hit_chance_mod = 15;
    d.ranged.throw_hit_chance_mod = -5;
    d.ranged.throw_dmg = Dice_param(1, 10);
    d.melee.hit_medium_sfx = Sfx_id::hit_sharp;
    d.melee.hit_hard_sfx = Sfx_id::hit_sharp;
    d.melee.miss_sfx = Sfx_id::miss_light;
    add_feature_found_in(d, Feature_id::chest);
    add_feature_found_in(d, Feature_id::cabinet);
    add_feature_found_in(d, Feature_id::cocoon);
    data[size_t(d.id)] = d;

    reset_data(d, Item_type::melee_wpn);
    d.id = Item_id::club;
    d.base_name = {"Club", "Clubs", "a Club"};
    d.base_descr =
    {
        "Wielded since prehistoric times.",

        "Melee attacks with clubs are noisy."
    };
    d.spawn_std_range = Range(DLVL_FIRST_LATE_GAME, INT_MAX);
    d.weight = Item_weight::medium;
    d.tile = Tile_id::club;
    d.clr = clr_brown;
    d.melee.att_msgs = {"strike", "strikes me with a Club"};
    d.melee.dmg = pair<int, int>(2, 3);
    d.melee.hit_chance_mod = 10;
    d.melee.miss_sfx = Sfx_id::miss_medium;
    data[size_t(d.id)] = d;

    reset_data(d, Item_type::melee_wpn);
    d.id = Item_id::hammer;
    d.base_name = {"Hammer", "Hammers", "a Hammer"};
    d.base_descr =
    {
        "Typically used for construction, but can be quite devastating when wielded as "
        "a weapon.",

        "Melee attacks with hammers are noisy."
    };
    d.weight = Item_weight::medium;
    d.tile = Tile_id::hammer;
    d.melee.att_msgs = {"strike", "strikes me with a Hammer"};
    d.melee.dmg = pair<int, int>(2, 4);
    d.melee.hit_chance_mod = 5;
    d.melee.miss_sfx = Sfx_id::miss_medium;
    add_feature_found_in(d, Feature_id::cabinet);
    add_feature_found_in(d, Feature_id::cocoon);
    data[size_t(d.id)] = d;

    reset_data(d, Item_type::melee_wpn);
    d.id = Item_id::machete;
    d.base_name = {"Machete", "Machetes", "a Machete"};
    d.base_descr =
    {
        "A large cleaver-like knife. It serves well both as a cutting tool and weapon.",

        "Melee attacks with machetes are noisy."
    };
    d.weight = Item_weight::medium;
    d.tile = Tile_id::machete;
    d.melee.att_msgs = {"strike", "strikes me with a Machete"};
    d.melee.dmg = pair<int, int>(2, 5);
    d.melee.hit_chance_mod = 0;
    d.melee.hit_small_sfx = Sfx_id::hit_sharp;
    d.melee.hit_medium_sfx = Sfx_id::hit_sharp;
    d.melee.miss_sfx = Sfx_id::miss_medium;
    add_feature_found_in(d, Feature_id::cabinet);
    add_feature_found_in(d, Feature_id::cocoon);
    data[size_t(d.id)] = d;

    reset_data(d, Item_type::melee_wpn);
    d.id = Item_id::axe;
    d.base_name = {"Axe", "Axes", "an Axe"};
    d.base_descr =
    {
        "A tool intended for felling trees, splitting timber, etc. Used as a weapon it "
        "can deliver devastating blows, although it requires some skill to use "
        "effectively. Also effective for breaching wooden doors.",

        "Melee attacks with axes are noisy."
    };
    d.weight = Item_weight::medium;
    d.tile = Tile_id::axe;
    d.melee.att_msgs = {"strike", "strikes me with an axe"};
    d.melee.dmg = pair<int, int>(2, 6);
    d.melee.hit_chance_mod = -5;
    d.melee.miss_sfx = Sfx_id::miss_medium;
    add_feature_found_in(d, Feature_id::cabinet);
    add_feature_found_in(d, Feature_id::tomb);
    add_feature_found_in(d, Feature_id::cocoon);
    data[size_t(d.id)] = d;

    reset_data(d, Item_type::melee_wpn);
    d.id = Item_id::pitch_fork;
    d.base_name = {"Pitchfork", "Pitchforks", "a Pitchfork"};
    d.base_descr =
    {
        "A long staff with a forked, four-pronged end. Victims can be pushed away when "
        "stabbed, to keep them at bay."
    };
    d.weight = Item_weight::heavy;
    d.tile = Tile_id::pitchfork;
    d.melee.att_msgs = {"strike", "strikes me with a Pitchfork"};
    d.melee.dmg = pair<int, int>(3, 4);
    d.melee.hit_chance_mod = -5;
    d.melee.knocks_back = true;
    d.melee.hit_small_sfx = Sfx_id::hit_sharp;
    d.melee.hit_medium_sfx = Sfx_id::hit_sharp;
    d.melee.miss_sfx = Sfx_id::miss_heavy;
    add_feature_found_in(d, Feature_id::cabinet);
    add_feature_found_in(d, Feature_id::cocoon);
    data[size_t(d.id)] = d;

    reset_data(d, Item_type::melee_wpn);
    d.id = Item_id::sledge_hammer;
    d.base_name = {"Sledgehammer", "Sledgehammers", "a Sledgehammer"};
    d.base_descr =
    {
        "Often used in destruction work for breaking through walls. It can deal a great "
        "amount of damage, although it is cumbersome to carry, and it requires some "
        "skill to use effectively."
    };
    d.weight = Item_weight::heavy;
    d.tile = Tile_id::sledge_hammer;
    d.melee.att_msgs = {"strike", "strikes me with a Sledgehammer"};
    d.melee.dmg = pair<int, int>(3, 5);
    d.melee.hit_chance_mod = -10;
    d.melee.knocks_back = true;
    d.melee.miss_sfx = Sfx_id::miss_heavy;
    add_feature_found_in(d, Feature_id::cabinet);
    data[size_t(d.id)] = d;

    reset_data(d, Item_type::melee_wpn);
    d.id = Item_id::pharaoh_staff;
    d.base_name =
    {
        "Staff of the Pharaohs", "Staff of the Pharaohs", "the Staff of the Pharaohs"
    };
    d.base_descr =
    {
        "Once wielded by long-forgotten kings in ancient times, this powerful artifact "
        "grants the power to call up a loyal servant from the dead.",

        item_carry_shock_descr
    };
    d.clr = clr_magenta;
    d.weight = Item_weight::medium;
    d.tile = Tile_id::pharaoh_staff;
    d.melee.att_msgs = {"strike", "strikes me with the Staff of the Pharaohs"};
    d.melee.dmg = pair<int, int>(2, 4);
    d.melee.hit_chance_mod = 0;
    d.melee.miss_sfx = Sfx_id::miss_medium;
    d.chance_to_incl_in_floor_spawn_list = 1;
    d.value = Item_value::major_treasure;
    d.shock_while_in_backpack = d.shock_while_equipped = 15;
    add_feature_found_in(d, Feature_id::tomb, 20);
    data[size_t(d.id)] = d;

    reset_data(d, Item_type::throwing_wpn);
    d.id = Item_id::iron_spike;
    d.base_name = {"Iron Spike", "Iron Spikes", "an Iron Spike"};
    d.base_descr =
    {
        "Can be useful for wedging things closed."
        /*TODO: or prying things open."*/
    };
    d.weight = Item_weight::extra_light;
    d.tile = Tile_id::iron_spike;
    d.is_stackable = true;
    d.clr = clr_gray;
    d.glyph = '/';
    d.ranged.throw_hit_chance_mod = -5;
    d.ranged.throw_dmg = Dice_param(1, 3);
    d.ranged.effective_range = 3;
    d.max_stack_at_spawn = 12;
    d.land_on_hard_snd_msg = "I hear a clanking sound.";
    d.land_on_hard_sfx = Sfx_id::metal_clank;
    d.main_att_mode = Main_att_mode::thrown;
    add_feature_found_in(d, Feature_id::cabinet);
    add_feature_found_in(d, Feature_id::cocoon);
    data[size_t(d.id)] = d;

    reset_data(d, Item_type::melee_wpn_intr);
    d.id = Item_id::player_kick;
    d.melee.att_msgs = {"kick", ""};
    d.melee.hit_chance_mod = 15;
    d.melee.dmg = pair<int, int>(1, 3);
    d.melee.knocks_back = true;
    d.melee.miss_sfx = Sfx_id::miss_medium;
    data[size_t(d.id)] = d;

    reset_data(d, Item_type::melee_wpn_intr);
    d.id = Item_id::player_stomp;
    d.melee.att_msgs = {"stomp", ""};
    d.melee.hit_chance_mod = data[size_t(Item_id::player_kick)].melee.hit_chance_mod;
    d.melee.dmg = data[size_t(Item_id::player_kick)].melee.dmg;
    d.melee.miss_sfx = data[size_t(Item_id::player_kick)].melee.miss_sfx;
    d.melee.knocks_back = false;
    data[size_t(d.id)] = d;

    reset_data(d, Item_type::melee_wpn_intr);
    d.id = Item_id::player_punch;
    d.melee.att_msgs = {"punch", ""};
    d.melee.hit_chance_mod = 20;
    d.melee.dmg = pair<int, int>(1, 1);
    d.melee.miss_sfx = Sfx_id::miss_light;
    data[size_t(d.id)] = d;

    reset_data(d, Item_type::melee_wpn_intr);
    d.id = Item_id::player_ghoul_claw;
    d.melee.att_msgs = {"claw", ""};
    d.melee.hit_chance_mod = 25;
    d.melee.dmg = pair<int, int>(2, 5);
    data[size_t(d.id)] = d;

    reset_data(d, Item_type::melee_wpn_intr);
    d.id = Item_id::zombie_claw;
    d.melee.att_msgs = {"", "claws me"};
    set_dmg_from_mon_id(d, Actor_id::zombie);
    data[size_t(d.id)] = d;

    reset_data(d, Item_type::melee_wpn_intr);
    d.id = Item_id::zombie_claw_diseased;
    d.melee.att_msgs = {"", "claws me"};
    set_dmg_from_mon_id(d, Actor_id::zombie);
    d.melee.prop_applied = new Prop_infected(Prop_turns::std);
    data[size_t(d.id)] = d;

    reset_data(d, Item_type::melee_wpn_intr);
    d.id = Item_id::zombie_axe;
    d.melee.att_msgs = {"", "chops me with a rusty axe"};
    set_dmg_from_mon_id(d, Actor_id::zombie_axe);
    data[size_t(d.id)] = d;

    reset_data(d, Item_type::melee_wpn_intr);
    d.id = Item_id::bloated_zombie_punch;
    d.melee.att_msgs = {"", "mauls me"};
    set_dmg_from_mon_id(d, Actor_id::bloated_zombie);
    d.melee.knocks_back = true;
    data[size_t(d.id)] = d;

    reset_data(d, Item_type::ranged_wpn_intr);
    d.id = Item_id::bloated_zombie_spit;
    d.ranged.att_msgs = {"", "spits acid pus at me"};
    set_dmg_from_mon_id(d, Actor_id::bloated_zombie);
    d.ranged.snd_msg = "I hear spitting.";
    d.ranged.projectile_clr = clr_green_lgt;
    d.ranged.dmg_type = Dmg_type::acid;
    d.ranged.projectile_glyph = '*';
    data[size_t(d.id)] = d;

    reset_data(d, Item_type::melee_wpn_intr);
    d.id = Item_id::crawling_intestines_strangle;
    d.melee.att_msgs = {"", "strangles me"};
    set_dmg_from_mon_id(d, Actor_id::crawling_intestines);
    data[size_t(d.id)] = d;

    reset_data(d, Item_type::melee_wpn_intr);
    d.id = Item_id::crawling_hand_strangle;
    d.melee.att_msgs = {"", "strangles me"};
    set_dmg_from_mon_id(d, Actor_id::crawling_hand);
    data[size_t(d.id)] = d;

    reset_data(d, Item_type::melee_wpn_intr);
    d.id = Item_id::thing_strangle;
    d.melee.att_msgs = {"", "strangles me"};
    set_dmg_from_mon_id(d, Actor_id::thing);
    data[size_t(d.id)] = d;

    reset_data(d, Item_type::melee_wpn_intr);
    d.id = Item_id::rat_bite;
    d.melee.att_msgs = {"", "bites me"};
    set_dmg_from_mon_id(d, Actor_id::rat);
    data[size_t(d.id)] = d;

    reset_data(d, Item_type::melee_wpn_intr);
    d.id = Item_id::rat_bite_diseased;
    d.melee.att_msgs = {"", "bites me"};
    set_dmg_from_mon_id(d, Actor_id::rat);
    d.melee.prop_applied = new Prop_infected(Prop_turns::std);
    data[size_t(d.id)] = d;

    reset_data(d, Item_type::melee_wpn_intr);
    d.id = Item_id::rat_thing_bite;
    d.melee.att_msgs = {"", "bites me"};
    set_dmg_from_mon_id(d, Actor_id::rat_thing);
    data[size_t(d.id)] = d;

    reset_data(d, Item_type::melee_wpn_intr);
    d.id = Item_id::brown_jenkin_bite;
    d.melee.att_msgs = {"", "bites me"};
    set_dmg_from_mon_id(d, Actor_id::brown_jenkin);
    data[size_t(d.id)] = d;

    reset_data(d, Item_type::melee_wpn_intr);
    d.id = Item_id::worm_mass_bite;
    d.melee.att_msgs = {"", "bites me"};
    set_dmg_from_mon_id(d, Actor_id::worm_mass);
    data[size_t(d.id)] = d;

    reset_data(d, Item_type::melee_wpn_intr);
    d.id = Item_id::wolf_bite;
    d.melee.att_msgs = {"", "bites me"};
    set_dmg_from_mon_id(d, Actor_id::wolf);
    data[size_t(d.id)] = d;

    reset_data(d, Item_type::melee_wpn_intr);
    d.id = Item_id::green_spider_bite;
    d.melee.att_msgs = {"", "bites me"};
    set_dmg_from_mon_id(d, Actor_id::green_spider);
    d.melee.prop_applied = new Prop_blind(Prop_turns::specific, 4);
    data[size_t(d.id)] = d;

    reset_data(d, Item_type::melee_wpn_intr);
    d.id = Item_id::white_spider_bite;
    d.melee.att_msgs = {"", "bites me"};
    set_dmg_from_mon_id(d, Actor_id::white_spider);
    d.melee.prop_applied = new Prop_paralyzed(Prop_turns::specific, 2);
    data[size_t(d.id)] = d;

    reset_data(d, Item_type::melee_wpn_intr);
    d.id = Item_id::red_spider_bite;
    d.melee.att_msgs = {"", "bites me"};
    set_dmg_from_mon_id(d, Actor_id::red_spider);
    d.melee.prop_applied = new Prop_weakened(Prop_turns::std);
    data[size_t(d.id)] = d;

    reset_data(d, Item_type::melee_wpn_intr);
    d.id = Item_id::shadow_spider_bite;
    d.melee.att_msgs = {"", "bites me"};
    set_dmg_from_mon_id(d, Actor_id::shadow_spider);
    data[size_t(d.id)] = d;

    reset_data(d, Item_type::melee_wpn_intr);
    d.id = Item_id::leng_spider_bite;
    d.melee.att_msgs = {"", "bites me"};
    set_dmg_from_mon_id(d, Actor_id::leng_spider);
    data[size_t(d.id)] = d;

    reset_data(d, Item_type::ranged_wpn_intr);
    d.id = Item_id::fire_hound_breath;
    d.ranged.att_msgs = {"", "breaths fire at me"};
    d.ranged.snd_msg = "I hear a burst of flames.";
    set_dmg_from_mon_id(d, Actor_id::fire_hound);
    d.ranged.prop_applied = new Prop_burning(Prop_turns::std);
    d.ranged.projectile_clr = clr_red_lgt;
    d.ranged.projectile_glyph = '*';
    d.ranged.projectile_leaves_trail = true;
    d.ranged.projectile_leaves_smoke = true;
    d.ranged.dmg_type = Dmg_type::fire;
    data[size_t(d.id)] = d;

    reset_data(d, Item_type::melee_wpn_intr);
    d.id = Item_id::fire_hound_bite;
    d.melee.att_msgs = {"", "bites me"};
    set_dmg_from_mon_id(d, Actor_id::fire_hound);
    d.melee.dmg_type = Dmg_type::fire;
    data[size_t(d.id)] = d;

    reset_data(d, Item_type::ranged_wpn_intr);
    d.id = Item_id::frost_hound_breath;
    d.ranged.att_msgs = {"", "breaths frost at me"};
    d.ranged.snd_msg = "I hear a chilling sound.";
    set_dmg_from_mon_id(d, Actor_id::frost_hound);
    d.ranged.projectile_clr = clr_blue_lgt;
    d.ranged.projectile_glyph = '*';
    d.ranged.projectile_leaves_trail = true;
    d.ranged.projectile_leaves_smoke = true;
    d.ranged.dmg_type = Dmg_type::cold;
    data[size_t(d.id)] = d;

    reset_data(d, Item_type::melee_wpn_intr);
    d.id = Item_id::frost_hound_bite;
    d.melee.att_msgs = {"", "bites me"};
    set_dmg_from_mon_id(d, Actor_id::frost_hound);
    d.melee.dmg_type = Dmg_type::cold;
    data[size_t(d.id)] = d;

    reset_data(d, Item_type::melee_wpn_intr);
    d.id = Item_id::zuul_bite;
    d.melee.att_msgs = {"", "bites me"};
    set_dmg_from_mon_id(d, Actor_id::zuul);
    d.melee.dmg_type = Dmg_type::physical;
    data[size_t(d.id)] = d;

    reset_data(d, Item_type::melee_wpn_intr);
    d.id = Item_id::dust_vortex_engulf;
    d.melee.att_msgs = {"", "engulfs me"};
    set_dmg_from_mon_id(d, Actor_id::dust_vortex);
    d.melee.prop_applied = new Prop_blind(Prop_turns::std);
    data[size_t(d.id)] = d;

    reset_data(d, Item_type::melee_wpn_intr);
    d.id = Item_id::fire_vortex_engulf;
    d.melee.att_msgs = {"", "engulfs me"};
    set_dmg_from_mon_id(d, Actor_id::fire_vortex);
    d.melee.prop_applied = new Prop_burning(Prop_turns::std);
    data[size_t(d.id)] = d;

    reset_data(d, Item_type::melee_wpn_intr);
    d.id = Item_id::frost_vortex_engulf;
    d.melee.att_msgs = {"", "engulfs me"};
    set_dmg_from_mon_id(d, Actor_id::frost_vortex);
    data[size_t(d.id)] = d;

    reset_data(d, Item_type::melee_wpn_intr);
    d.id = Item_id::ghost_claw;
    d.melee.att_msgs = {"", "claws me"};
    set_dmg_from_mon_id(d, Actor_id::ghost);
    d.melee.prop_applied = new Prop_terrified(Prop_turns::specific, 4);
    d.melee.dmg_type = Dmg_type::spirit;
    data[size_t(d.id)] = d;

    reset_data(d, Item_type::melee_wpn_intr);
    d.id = Item_id::phantasm_sickle;
    d.melee.att_msgs = {"", "slices me with an ethereal sickle"};
    set_dmg_from_mon_id(d, Actor_id::phantasm);
    d.melee.prop_applied = new Prop_terrified(Prop_turns::specific, 4);
    d.melee.dmg_type = Dmg_type::spirit;
    data[size_t(d.id)] = d;

    reset_data(d, Item_type::melee_wpn_intr);
    d.id = Item_id::wraith_claw;
    d.melee.att_msgs = {"", "claws me"};
    set_dmg_from_mon_id(d, Actor_id::wraith);
    d.melee.prop_applied = new Prop_terrified(Prop_turns::specific, 4);
    d.melee.dmg_type = Dmg_type::spirit;
    data[size_t(d.id)] = d;

    reset_data(d, Item_type::melee_wpn_intr);
    d.id = Item_id::raven_peck;
    d.melee.att_msgs = {"", "pecks at me"};
    set_dmg_from_mon_id(d, Actor_id::raven);
    d.melee.prop_applied = new Prop_blind(Prop_turns::specific, 3);
    data[size_t(d.id)] = d;

    reset_data(d, Item_type::melee_wpn_intr);
    d.id = Item_id::giant_bat_bite;
    d.melee.att_msgs = {"", "bites me"};
    set_dmg_from_mon_id(d, Actor_id::giant_bat);
    data[size_t(d.id)] = d;

    reset_data(d, Item_type::melee_wpn_intr);
    d.id = Item_id::polyp_tentacle;
    d.melee.att_msgs = {"", "grips me with a tentacle"};
    d.melee.prop_applied = new Prop_paralyzed(Prop_turns::specific, 1);
    set_dmg_from_mon_id(d, Actor_id::flying_polyp);
    data[size_t(d.id)] = d;

    reset_data(d, Item_type::melee_wpn_intr);
    d.id = Item_id::greater_polyp_tentacle;
    d.melee.att_msgs = {"", "grips me with a tentacle"};
    d.melee.prop_applied = new Prop_paralyzed(Prop_turns::specific, 1);
    set_dmg_from_mon_id(d, Actor_id::greater_polyp);
    data[size_t(d.id)] = d;

    reset_data(d, Item_type::melee_wpn_intr);
    d.id = Item_id::ghoul_claw;
    d.melee.att_msgs = {"", "claws me"};
    set_dmg_from_mon_id(d, Actor_id::ghoul);
    d.melee.prop_applied = new Prop_infected(Prop_turns::std);
    data[size_t(d.id)] = d;

    reset_data(d, Item_type::melee_wpn_intr);
    d.id = Item_id::shadow_claw;
    d.melee.att_msgs = {"", "claws me"};
    set_dmg_from_mon_id(d, Actor_id::shadow);
    data[size_t(d.id)] = d;

    reset_data(d, Item_type::melee_wpn_intr);
    d.id = Item_id::byakhee_claw;
    d.melee.att_msgs = {"", "claws me"};
    set_dmg_from_mon_id(d, Actor_id::byakhee);
    data[size_t(d.id)] = d;

    reset_data(d, Item_type::melee_wpn_intr);
    d.id = Item_id::giant_mantis_claw;
    d.melee.att_msgs = {"", "claws me"};
    set_dmg_from_mon_id(d, Actor_id::giant_mantis);
    data[size_t(d.id)] = d;

    reset_data(d, Item_type::melee_wpn_intr);
    d.id = Item_id::giant_locust_bite;
    d.melee.att_msgs = {"", "bites me"};
    set_dmg_from_mon_id(d, Actor_id::locust);
    data[size_t(d.id)] = d;

    reset_data(d, Item_type::melee_wpn_intr);
    d.id = Item_id::mummy_maul;
    d.melee.att_msgs = {"", "mauls me"};
    set_dmg_from_mon_id(d, Actor_id::mummy);
    d.melee.prop_applied = new Prop_cursed(Prop_turns::indefinite);
    d.melee.knocks_back = true;
    data[size_t(d.id)] = d;

    reset_data(d, Item_type::melee_wpn_intr);
    d.id = Item_id::croc_head_mummy_spear;
    d.melee.att_msgs = {"", "hits me with a spear"};
    set_dmg_from_mon_id(d, Actor_id::croc_head_mummy);
    data[size_t(d.id)] = d;

    reset_data(d, Item_type::ranged_wpn_intr);
    d.id = Item_id::deep_one_javelin_att;
    d.ranged.att_msgs = {"", "throws a javelin at me"};
    set_dmg_from_mon_id(d, Actor_id::deep_one);
    d.ranged.snd_msg = "";
    d.ranged.projectile_clr = clr_brown;
    d.ranged.projectile_glyph = '/';
    d.ranged.snd_vol = Snd_vol::low;
    data[size_t(d.id)] = d;

    reset_data(d, Item_type::melee_wpn_intr);
    d.id = Item_id::deep_one_spear_att;
    d.melee.att_msgs = {"", "hits me with a spear"};
    set_dmg_from_mon_id(d, Actor_id::deep_one);
    data[size_t(d.id)] = d;

    reset_data(d, Item_type::melee_wpn_intr);
    d.id = Item_id::ape_maul;
    d.melee.att_msgs = {"", "mauls me"};
    set_dmg_from_mon_id(d, Actor_id::ape);
    data[size_t(d.id)] = d;

    reset_data(d, Item_type::melee_wpn_intr);
    d.id = Item_id::ooze_black_spew_pus;
    d.melee.att_msgs = {"", "spews pus on me"};
    set_dmg_from_mon_id(d, Actor_id::ooze_black);
    data[size_t(d.id)] = d;

    reset_data(d, Item_type::melee_wpn_intr);
    d.id = Item_id::ooze_clear_spew_pus;
    d.melee.att_msgs = {"", "spews pus on me"};
    set_dmg_from_mon_id(d, Actor_id::ooze_clear);
    data[size_t(d.id)] = d;

    reset_data(d, Item_type::melee_wpn_intr);
    d.id = Item_id::ooze_putrid_spew_pus;
    d.melee.att_msgs = {"", "spews infected pus on me"};
    set_dmg_from_mon_id(d, Actor_id::ooze_putrid);
    d.melee.prop_applied = new Prop_infected(Prop_turns::std);
    data[size_t(d.id)] = d;

    reset_data(d, Item_type::melee_wpn_intr);
    d.id = Item_id::ooze_poison_spew_pus;
    d.melee.att_msgs = {"", "spews poisonous pus on me"};
    set_dmg_from_mon_id(d, Actor_id::ooze_poison);
    d.melee.prop_applied = new Prop_poisoned(Prop_turns::std);
    data[size_t(d.id)] = d;

    reset_data(d, Item_type::melee_wpn_intr);
    d.id = Item_id::color_oo_space_touch;
    d.melee.att_msgs = {"", "touches me"};
    set_dmg_from_mon_id(d, Actor_id::color_oo_space);
    data[size_t(d.id)] = d;

    reset_data(d, Item_type::melee_wpn_intr);
    d.id = Item_id::chthonian_bite;
    d.melee.att_msgs = {"", "strikes me with a tentacle"};
    d.melee.knocks_back = true;
    set_dmg_from_mon_id(d, Actor_id::chthonian);
    data[size_t(d.id)] = d;

    reset_data(d, Item_type::melee_wpn_intr);
    d.id = Item_id::death_fiend_claw;
    d.melee.att_msgs = {"", "claws me"};
    set_dmg_from_mon_id(d, Actor_id::death_fiend);
    d.melee.dmg_type = Dmg_type::pure;
    data[size_t(d.id)] = d;

    reset_data(d, Item_type::melee_wpn_intr);
    d.id = Item_id::hunting_horror_bite;
    d.melee.att_msgs = {"", "bites me"};
    set_dmg_from_mon_id(d, Actor_id::hunting_horror);
    d.melee.prop_applied = new Prop_poisoned(Prop_turns::std);
    data[size_t(d.id)] = d;

    reset_data(d, Item_type::melee_wpn_intr);
    d.id = Item_id::mold_spores;
    d.melee.att_msgs = {"", "releases spores at me"};
    set_dmg_from_mon_id(d, Actor_id::mold);
    d.melee.prop_applied = new Prop_poisoned(Prop_turns::specific, POISON_DMG_N_TURN * 2);
    data[size_t(d.id)] = d;

    reset_data(d, Item_type::melee_wpn_intr);
    d.id = Item_id::mi_go_sting;
    d.melee.att_msgs = {"", "stings me"};
    set_dmg_from_mon_id(d, Actor_id::mi_go);
    d.melee.prop_applied = new Prop_poisoned(Prop_turns::specific, POISON_DMG_N_TURN * 2);
    data[size_t(d.id)] = d;

    reset_data(d, Item_type::melee_wpn_intr);
    d.id = Item_id::mi_go_commander_sting;
    d.melee.att_msgs = {"", "stings me"};
    set_dmg_from_mon_id(d, Actor_id::mi_go_commander);
    d.melee.prop_applied = new Prop_poisoned(Prop_turns::specific, POISON_DMG_N_TURN * 2);
    data[size_t(d.id)] = d;

    reset_data(d, Item_type::melee_wpn_intr);
    d.id = Item_id::the_high_priest_claw;
    d.melee.att_msgs = {"", "claws me"};
    set_dmg_from_mon_id(d, Actor_id::the_high_priest);
    data[size_t(d.id)] = d;

    reset_data(d, Item_type::armor);
    d.id = Item_id::armor_leather_jacket;
    d.base_name = {"Leather Jacket", "", "a Leather Jacket"};
    d.base_descr =
    {
        "It offers some protection."
    };
    d.weight = Item_weight::light;
    d.clr = clr_brown;
    d.spawn_std_range.min = 1;
    d.armor.armor_points = 1;
    d.armor.dmg_to_durability_factor = 1.0;
    d.land_on_hard_snd_msg = "";
    add_feature_found_in(d, Feature_id::cabinet);
    data[size_t(d.id)] = d;

    reset_data(d, Item_type::armor);
    d.id = Item_id::armor_iron_suit;
    d.base_name = {"Iron Suit", "", "an Iron Suit"};
    d.base_descr =
    {
        "A crude armour constructed from metal plates, bolts, and leather straps.",

        "It can absorb a high amount of damage, but it makes sneaking and dodging very "
        "difficult. Also, due to the narrow slit of the helmet, aiming is slightly "
        "more difficult, and it is harder to detect sneaking enemies and hidden objects."
    };
    d.ability_mods_while_equipped[int(Ability_id::stealth)]    = -50;
    d.ability_mods_while_equipped[int(Ability_id::dodge_att)]   = -50;
    d.ability_mods_while_equipped[int(Ability_id::dodge_trap)]  = -50;
    d.ability_mods_while_equipped[int(Ability_id::melee)]      = -10;
    d.ability_mods_while_equipped[int(Ability_id::ranged)]     = -10;
    d.ability_mods_while_equipped[int(Ability_id::searching)]  = -6;
    d.weight = Item_weight::heavy;
    d.clr = clr_white;
    d.spawn_std_range.min = 2;
    d.armor.armor_points = 5;
    d.armor.dmg_to_durability_factor = 0.3;
    d.land_on_hard_snd_msg = "I hear a crashing sound.";
    add_feature_found_in(d, Feature_id::cabinet);
    data[size_t(d.id)] = d;

    reset_data(d, Item_type::armor);
    d.id = Item_id::armor_flack_jacket;
    d.base_name = {"Flak Jacket", "", "a Flak Jacket"};
    d.base_descr =
    {
        "An armour consisting of steel plates sewn into a waistcoat. It offers very good "
        "protection for its weight. Sneaking and dodging is slightly more difficult."
    };
    d.ability_mods_while_equipped[int(Ability_id::stealth)]    = -20;
    d.ability_mods_while_equipped[int(Ability_id::dodge_att)]   = -20;
    d.ability_mods_while_equipped[int(Ability_id::dodge_trap)]  = -20;
    d.weight = Item_weight::medium;
    d.clr = clr_green;
    d.spawn_std_range.min = 3;
    d.armor.armor_points = 3;
    d.armor.dmg_to_durability_factor = 0.5;
    d.land_on_hard_snd_msg = "I hear a thudding sound.";
    add_feature_found_in(d, Feature_id::cabinet);
    data[size_t(d.id)] = d;

    reset_data(d, Item_type::armor);
    d.id = Item_id::armor_asb_suit;
    d.base_name = {"Asbestos Suit", "", "an Asbestos Suit"};
    d.base_descr =
    {
        "A one piece overall of asbestos fabric, including a hood, furnace mask, gloves "
        "and shoes. It protects the wearer against fire, acid and electricity, and also "
        "against smoke, fumes and gas.",

        "It is a bit bulky, so sneaking and dodging is slightly more difficult. Also, "
        "because of the hood and mask, aiming and detecting hidden enemies and objects "
        "is somewhat harder."
    };
    d.ability_mods_while_equipped[int(Ability_id::stealth)]    = -20;
    d.ability_mods_while_equipped[int(Ability_id::dodge_att)]   = -20;
    d.ability_mods_while_equipped[int(Ability_id::dodge_trap)]  = -20;
    d.ability_mods_while_equipped[int(Ability_id::melee)]      = -10;
    d.ability_mods_while_equipped[int(Ability_id::ranged)]     = -10;
    d.ability_mods_while_equipped[int(Ability_id::searching)]  = -6;
    d.weight = Item_weight::medium;
    d.clr = clr_red_lgt;
    d.spawn_std_range.min = 3;
    d.armor.armor_points = 1;
    d.armor.dmg_to_durability_factor = 1.0;
    d.land_on_hard_snd_msg = "";
    add_feature_found_in(d, Feature_id::cabinet);
    add_feature_found_in(d, Feature_id::chest);
    data[size_t(d.id)] = d;

    reset_data(d, Item_type::armor);
    d.id = Item_id::armor_heavy_coat;
    d.base_name = {"Heavy Coat", "", "a Heavy Coat"};
    d.base_descr =
    {
        "A very thick coat. It gives more protection against physical harm than a "
        "leather jacket, and it also protects against cold.",

        "Sneaking and dodging is slightly more difficult due to its higher weight."
    };
    d.ability_mods_while_equipped[int(Ability_id::stealth)]    = -15;
    d.ability_mods_while_equipped[int(Ability_id::dodge_att)]   = -15;
    d.ability_mods_while_equipped[int(Ability_id::dodge_trap)]  = -15;
    d.weight = Item_weight::medium;
    d.clr = clr_blue_lgt;
    d.spawn_std_range.min = 3;
    d.armor.armor_points = 2;
    d.armor.dmg_to_durability_factor = 1.0;
    d.land_on_hard_snd_msg = "";
    add_feature_found_in(d, Feature_id::cabinet);
    data[size_t(d.id)] = d;

    reset_data(d, Item_type::armor);
    d.id = Item_id::armor_mi_go;
    d.base_name = {"Mi-go Bio-armor", "", "a Mi-go Bio-armor"};
    d.base_descr =
    {
        "A self-repairing biological armor created by the Mi-go.",

        "It is very disturbing for a human to wear."
    };
    d.spawn_std_range = Range(-1, -1);
    d.weight = Item_weight::medium;
    d.shock_while_equipped = 15;
    d.clr = clr_magenta;
    d.tile = Tile_id::mi_go_armor;
    d.armor.armor_points = 2;
    d.armor.dmg_to_durability_factor = 1.5;
    d.land_on_hard_snd_msg = "";
    data[size_t(d.id)] = d;

    reset_data(d, Item_type::head_wear);
    d.id = Item_id::gas_mask;
    d.base_name = {"Gas Mask", "", "a Gas Mask"};
    d.base_descr =
    {
        "Protects the eyes, throat and lungs from smoke and fumes. It has a limited "
        "useful lifespan that is related to the absorbent capacity of the filter.",

        "Due to the small eye windows, aiming is slightly more difficult, and it is "
        "harder to detect sneaking enemies and hidden objects."
    };
    d.ability_mods_while_equipped[int(Ability_id::melee)]      = -10;
    d.ability_mods_while_equipped[int(Ability_id::ranged)]     = -10;
    d.ability_mods_while_equipped[int(Ability_id::searching)]  = -6;
    d.is_stackable = false;
    d.clr = clr_brown;
    d.tile = Tile_id::mask;
    d.glyph = '[';
    d.spawn_std_range = Range(1, DLVL_LAST_EARLY_GAME);
    d.chance_to_incl_in_floor_spawn_list = 50;
    d.weight = Item_weight::light;
    d.land_on_hard_snd_msg = "";
    data[size_t(d.id)] = d;

//    reset_data(d, Item_type::head_wear);
//    d.id = Item_id::hideous_mask;
//    d.base_name = {"Hideous Mask", "", "The Hideous Mask"};
//    d.base_descr =
//    {
//        "[TODO]",
//
//        item_carry_shock_descr
//    };
//    d.is_stackable = false;
//    d.clr = clr_magenta;
//    d.tile = Tile_id::mask;
//    d.glyph = '[';
//    d.spawn_std_range = Range(-1, -1);
//    d.weight = Item_weight::light;
//    d.land_on_hard_snd_msg = "";
//    d.chance_to_incl_in_floor_spawn_list = 1;
//    d.value = Item_value::major_treasure;
//    d.shock_while_in_backpack = d.shock_while_equipped = 15;
//    add_feature_found_in(d, Feature_id::tomb, 8);
//    data[size_t(d.id)] = d;

    reset_data(d, Item_type::scroll);
    d.id = Item_id::scroll_mayhem;
    d.spell_cast_from_scroll = Spell_id::mayhem;
    data[size_t(d.id)] = d;

    reset_data(d, Item_type::scroll);
    d.id = Item_id::scroll_telep;
    d.spell_cast_from_scroll = Spell_id::teleport;
    data[size_t(d.id)] = d;

    reset_data(d, Item_type::scroll);
    d.id = Item_id::scroll_pest;
    d.spell_cast_from_scroll = Spell_id::pest;
    data[size_t(d.id)] = d;

    reset_data(d, Item_type::scroll);
    d.id = Item_id::scroll_slow_mon;
    d.spell_cast_from_scroll = Spell_id::slow_mon;
    data[size_t(d.id)] = d;

    reset_data(d, Item_type::scroll);
    d.id = Item_id::scroll_terrify_mon;
    d.spell_cast_from_scroll = Spell_id::terrify_mon;
    data[size_t(d.id)] = d;

    reset_data(d, Item_type::scroll);
    d.id = Item_id::scroll_paral_mon;
    d.spell_cast_from_scroll = Spell_id::paralyze_mon;
    data[size_t(d.id)] = d;

    reset_data(d, Item_type::scroll);
    d.id = Item_id::scroll_det_items;
    d.spell_cast_from_scroll = Spell_id::det_items;
    data[size_t(d.id)] = d;

    reset_data(d, Item_type::scroll);
    d.id = Item_id::scroll_det_traps;
    d.spell_cast_from_scroll = Spell_id::det_traps;
    data[size_t(d.id)] = d;

    reset_data(d, Item_type::scroll);
    d.id = Item_id::scroll_bless;
    d.spell_cast_from_scroll = Spell_id::bless;
    data[size_t(d.id)] = d;

    reset_data(d, Item_type::scroll);
    d.id = Item_id::scroll_darkbolt;
    d.spell_cast_from_scroll = Spell_id::darkbolt;
    data[size_t(d.id)] = d;

    reset_data(d, Item_type::scroll);
    d.id = Item_id::scroll_aza_wrath;
    d.spell_cast_from_scroll = Spell_id::aza_wrath;
    data[size_t(d.id)] = d;

    reset_data(d, Item_type::scroll);
    d.id = Item_id::scroll_opening;
    d.spell_cast_from_scroll = Spell_id::opening;
    data[size_t(d.id)] = d;

    reset_data(d, Item_type::scroll);
    d.id = Item_id::scroll_sacr_life;
    d.spell_cast_from_scroll = Spell_id::sacr_life;
    data[size_t(d.id)] = d;

    reset_data(d, Item_type::scroll);
    d.id = Item_id::scroll_sacr_spi;
    d.spell_cast_from_scroll = Spell_id::sacr_spi;
    data[size_t(d.id)] = d;

    reset_data(d, Item_type::scroll);
    d.id = Item_id::scroll_det_mon;
    d.spell_cast_from_scroll = Spell_id::det_mon;
    data[size_t(d.id)] = d;

    reset_data(d, Item_type::scroll);
    d.id = Item_id::scroll_elem_res;
    d.spell_cast_from_scroll = Spell_id::elem_res;
    data[size_t(d.id)] = d;

    reset_data(d, Item_type::scroll);
    d.id = Item_id::scroll_summon_mon;
    d.spell_cast_from_scroll = Spell_id::summon;
    data[size_t(d.id)] = d;

    reset_data(d, Item_type::scroll);
    d.id = Item_id::scroll_light;
    d.spell_cast_from_scroll = Spell_id::light;
    data[size_t(d.id)] = d;

    reset_data(d, Item_type::potion);
    d.id = Item_id::potion_vitality;
    data[size_t(d.id)] = d;

    reset_data(d, Item_type::potion);
    d.id = Item_id::potion_spirit;
    data[size_t(d.id)] = d;

    reset_data(d, Item_type::potion);
    d.id = Item_id::potion_blindness;
    data[size_t(d.id)] = d;

    reset_data(d, Item_type::potion);
    d.id = Item_id::potion_frenzy;
    data[size_t(d.id)] = d;

    reset_data(d, Item_type::potion);
    d.id = Item_id::potion_fortitude;
    data[size_t(d.id)] = d;

    reset_data(d, Item_type::potion);
    d.id = Item_id::potion_paralyze;
    data[size_t(d.id)] = d;

    reset_data(d, Item_type::potion);
    d.id = Item_id::potion_rElec;
    data[size_t(d.id)] = d;

    reset_data(d, Item_type::potion);
    d.id = Item_id::potion_conf;
    data[size_t(d.id)] = d;

    reset_data(d, Item_type::potion);
    d.id = Item_id::potion_poison;
    data[size_t(d.id)] = d;

    reset_data(d, Item_type::potion);
    d.id = Item_id::potion_insight;
    data[size_t(d.id)] = d;

    reset_data(d, Item_type::potion);
    d.id = Item_id::potion_clairv;
    data[size_t(d.id)] = d;

    reset_data(d, Item_type::potion);
    d.id = Item_id::potion_rFire;
    data[size_t(d.id)] = d;

    reset_data(d, Item_type::potion);
    d.id = Item_id::potion_antidote;
    data[size_t(d.id)] = d;

    reset_data(d, Item_type::potion);
    d.id = Item_id::potion_descent;
    data[size_t(d.id)] = d;

    reset_data(d, Item_type::potion);
    d.id = Item_id::potion_invis;
    data[size_t(d.id)] = d;

    reset_data(d, Item_type::potion);
    d.id = Item_id::potion_seeing;
    data[size_t(d.id)] = d;

    reset_data(d, Item_type::device);
    d.id = Item_id::device_blaster;
    d.base_name = {"Blaster Device", "Blaster Devices", "a Blaster Device"};
    d.value = Item_value::minor_treasure;
    d.clr = clr_gray;
    add_feature_found_in(d, Feature_id::chest, 10);
    add_feature_found_in(d, Feature_id::tomb, 10);
    add_feature_found_in(d, Feature_id::cocoon, 10);
    data[size_t(d.id)] = d;

    reset_data(d, Item_type::device);
    d.id = Item_id::device_shockwave;
    d.base_name =
    {
        "Shock Wave Device", "Shock Wave Devices", "a Shock Wave Device"
    };
    d.value = Item_value::minor_treasure;
    d.clr = clr_gray;
    add_feature_found_in(d, Feature_id::chest, 10);
    add_feature_found_in(d, Feature_id::tomb, 10);
    add_feature_found_in(d, Feature_id::cocoon, 10);
    data[size_t(d.id)] = d;

    reset_data(d, Item_type::device);
    d.id = Item_id::device_rejuvenator;
    d.base_name =
    {
        "Rejuvenator Device", "Rejuvenator Devices", "a Rejuvenator Device"
    };
    d.value = Item_value::minor_treasure;
    d.clr = clr_gray;
    add_feature_found_in(d, Feature_id::chest, 10);
    add_feature_found_in(d, Feature_id::tomb, 10);
    add_feature_found_in(d, Feature_id::cocoon, 10);
    data[size_t(d.id)] = d;

    reset_data(d, Item_type::device);
    d.id = Item_id::device_translocator;
    d.base_name =
    {
        "Translocator Device", "Translocator Devices", "a Translocator Device"
    };
    d.value = Item_value::minor_treasure;
    d.clr = clr_gray;
    add_feature_found_in(d, Feature_id::chest, 10);
    add_feature_found_in(d, Feature_id::tomb, 10);
    add_feature_found_in(d, Feature_id::cocoon, 10);
    data[size_t(d.id)] = d;

    reset_data(d, Item_type::device);
    d.id = Item_id::device_sentry_drone;
    d.base_name =
    {
        "Sentry Drone Device", "Sentry Drone Devices", "a Sentry Drone Device"
    };
    d.value = Item_value::minor_treasure;
    d.clr = clr_gray;
    add_feature_found_in(d, Feature_id::chest, 10);
    add_feature_found_in(d, Feature_id::tomb, 10);
    add_feature_found_in(d, Feature_id::cocoon, 10);
    data[size_t(d.id)] = d;

    reset_data(d, Item_type::device);
    d.id = Item_id::electric_lantern;
    d.is_prio_in_backpack_list = true;
    d.base_name = {"Electric Lantern", "Electric Lanterns", "an Electric Lantern"};
    d.base_descr =
    {
        "A portable light source. It is somewhat unreliable as it tends to flicker and "
        "malfunction often."
    };
    d.spawn_std_range = Range(1, DLVL_LAST_MID_GAME);
    d.spawn_std_range = Range(1, 10);
    d.chance_to_incl_in_floor_spawn_list = 50;
    d.is_identified = true;
    d.tile = Tile_id::electric_lantern;
    d.clr = clr_yellow;
    add_feature_found_in(d, Feature_id::chest);
    add_feature_found_in(d, Feature_id::cabinet);
    add_feature_found_in(d, Feature_id::cocoon);
    data[size_t(d.id)] = d;

    reset_data(d, Item_type::general);
    d.id = Item_id::medical_bag;
    d.has_std_activate = true;
    d.is_prio_in_backpack_list = true;
    d.base_name = {"Medical Bag", "Medical Bags", "a Medical Bag"};
    d.base_descr =
    {
        "A portable bag of medical supplies."
    };
    d.weight = Item_weight::medium;
    d.spawn_std_range = Range(1, DLVL_LAST_MID_GAME);
    d.is_stackable = false;
    d.glyph = '~';
    d.clr = clr_brown_drk;
    d.tile = Tile_id::medical_bag;
    data[size_t(d.id)] = d;

    reset_data(d, Item_type::amulet);
    d.id = Item_id::star_amulet;
    d.base_name = {"Star Amulet", "", "a Star Amulet"};
    d.base_name_un_id = d.base_name;
    d.clr = clr_white;
    data[size_t(d.id)] = d;

    reset_data(d, Item_type::amulet);
    d.id = Item_id::skull_amulet;
    d.base_name = {"Skull Amulet", "", "a Skull Amulet"};
    d.base_name_un_id = d.base_name;
    d.clr = clr_white;
    data[size_t(d.id)] = d;

    reset_data(d, Item_type::amulet);
    d.id = Item_id::spider_amulet;
    d.base_name = {"Spider Amulet", "", "a Spider Amulet"};
    d.base_name_un_id = d.base_name;
    d.clr = clr_white;
    data[size_t(d.id)] = d;

    reset_data(d, Item_type::amulet);
    d.id = Item_id::eye_amulet;
    d.base_name = {"Eye Amulet", "", "an Eye Amulet"};
    d.base_name_un_id = d.base_name;
    d.clr = clr_white;
    data[size_t(d.id)] = d;

    reset_data(d, Item_type::amulet);
    d.id = Item_id::moon_amulet;
    d.base_name = {"Moon Amulet", "", "a Moon Amulet"};
    d.base_name_un_id = d.base_name;
    d.clr = clr_white;
    data[size_t(d.id)] = d;

    reset_data(d, Item_type::amulet);
    d.id = Item_id::bat_amulet;
    d.base_name = {"Bat Amulet", "", "a Bat Amulet"};
    d.base_name_un_id = d.base_name;
    d.clr = clr_white;
    data[size_t(d.id)] = d;

    reset_data(d, Item_type::amulet);
    d.id = Item_id::scarab_amulet;
    d.base_name = {"Scarab Amulet", "", "a Scarab Amulet"};
    d.base_name_un_id = d.base_name;
    d.clr = clr_white;
    data[size_t(d.id)] = d;

    reset_data(d, Item_type::amulet);
    d.id = Item_id::dagger_amulet;
    d.base_name = {"Dagger Amulet", "", "a Dagger Amulet"};
    d.base_name_un_id = d.base_name;
    d.clr = clr_white;
    data[size_t(d.id)] = d;

    reset_data(d, Item_type::ring);
    d.id = Item_id::golden_ring;
    d.base_name = {"Golden Ring", "", "a Golden Ring"};
    d.base_name_un_id = d.base_name;
    d.clr = clr_yellow;
    data[size_t(d.id)] = d;

    reset_data(d, Item_type::ring);
    d.id = Item_id::silver_ring;
    d.base_name = {"Silver Ring", "", "a Silver Ring"};
    d.base_name_un_id = d.base_name;
    d.clr = clr_white;
    data[size_t(d.id)] = d;

    reset_data(d, Item_type::ring);
    d.id = Item_id::carnelian_ring;
    d.base_name = {"Carnelian Ring", "", "a Carnelian Ring"};
    d.base_name_un_id = d.base_name;
    d.clr = clr_red_lgt;
    data[size_t(d.id)] = d;

    reset_data(d, Item_type::ring);
    d.id = Item_id::garnet_ring;
    d.base_name = {"Garnet Ring", "", "a Garnet Ring"};
    d.base_name_un_id = d.base_name;
    d.clr = clr_red_lgt;
    data[size_t(d.id)] = d;

    reset_data(d, Item_type::ring);
    d.id = Item_id::iron_ring;
    d.base_name = {"Iron Ring", "", "an Iron Ring"};
    d.base_name_un_id = d.base_name;
    d.clr = clr_gray;
    data[size_t(d.id)] = d;

    reset_data(d, Item_type::ring);
    d.id = Item_id::jade_ring;
    d.base_name = {"Jade Ring", "", "a Jade Ring"};
    d.base_name_un_id = d.base_name;
    d.clr = clr_green_lgt;
    data[size_t(d.id)] = d;

    reset_data(d, Item_type::ring);
    d.id = Item_id::moonstone_ring;
    d.base_name = {"Moonstone Ring", "", "a Moonstone Ring"};
    d.base_name_un_id = d.base_name;
    d.clr = clr_blue_lgt;
    data[size_t(d.id)] = d;

    reset_data(d, Item_type::ring);
    d.id = Item_id::obsidian_ring;
    d.base_name = {"Obsidian Ring", "", "an Obsidian Ring"};
    d.base_name_un_id = d.base_name;
    d.clr = clr_gray;
    data[size_t(d.id)] = d;

    reset_data(d, Item_type::ring);
    d.id = Item_id::onyx_ring;
    d.base_name = {"Onyx Ring", "", "an Onyx Ring"};
    d.base_name_un_id = d.base_name;
    d.clr = clr_gray;
    data[size_t(d.id)] = d;

    reset_data(d, Item_type::ring);
    d.id = Item_id::topaz_ring;
    d.base_name = {"Topaz Ring", "", "a Topaz Ring"};
    d.base_name_un_id = d.base_name;
    d.clr = clr_blue_lgt;
    data[size_t(d.id)] = d;

    reset_data(d, Item_type::ring);
    d.id = Item_id::emerald_ring;
    d.base_name = {"Emerald Ring", "", "an Emerald Ring"};
    d.base_name_un_id = d.base_name;
    d.clr = clr_green_lgt;
    data[size_t(d.id)] = d;
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


    TRACE_FUNC_END;
}


void store_to_save_lines(vector<string>& lines)
{
    for (int i = 0; i < int(Item_id::END); ++i)
    {
        lines.push_back(data[i].is_identified ? "1" : "0");
        lines.push_back(data[i].allow_spawn   ? "1" : "0");

        if (
            data[i].type == Item_type::scroll ||
            data[i].type == Item_type::potion)
        {
            lines.push_back(data[i].is_tried ? "1" : "0");
        }
    }
}

void setup_from_save_lines(vector<string>& lines)
{
    for (int i = 0; i < int(Item_id::END); ++i)
    {
        data[i].is_identified = lines.front() == "0" ? false : true;
        lines.erase(begin(lines));

        data[i].allow_spawn = lines.front()   == "0" ? false : true;
        lines.erase(begin(lines));

        if (
            data[i].type == Item_type::scroll ||
            data[i].type == Item_type::potion)
        {
            data[i].is_tried = lines.front() == "0" ? false : true;
            lines.erase(begin(lines));
        }
    }
}

} //Item_data
