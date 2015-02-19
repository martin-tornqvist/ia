#include "item_data.h"

#include <iostream>
#include <climits>

#include "init.h"
#include "colors.h"
#include "item.h"
#include "cmn_data.h"
#include "actor_data.h"
#include "item_scroll.h"
#include "item_potion.h"
#include "actor_player.h"
#include "sound.h"
#include "item_device.h"
#include "map.h"

using namespace std;

Item_data_t::Item_data_t(const Item_id id_) :
    id                              (id_),
    type                            (Item_type::general),
    value                           (Item_value::normal),
    weight                          (Item_weight::none),
    allow_spawn                      (true),
    spawn_std_range                   (Range(1, INT_MAX)),
    max_stack_at_spawn                 (1),
    chance_to_include_in_floor_spawn_list (100),
    is_stackable                     (true),
    is_identified                    (true),
    is_tried                         (false),
    base_name                        (),
    glyph                           ('X'),
    clr                             (clr_white),
    tile                            (Tile_id::empty),
    main_att_mode                     (Main_att_mode::none),
    spell_cast_from_scroll             (Spell_id::END),
    land_on_hard_snd_msg                ("I hear a thudding sound."),
    land_on_hard_sfx                   (),
    shock_while_in_backpack            (0),
    shock_while_equipped              (0),
    melee                           (Item_melee_data()),
    ranged                          (Item_ranged_data()),
    armor                           (Item_armor_data())
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
    is_melee_wpn                      (false),
    dmg                             (pair<int, int>(0, 0)),
    hit_chance_mod                    (0),
    att_msgs                         (Item_att_msgs()),
    prop_applied                     (nullptr),
    dmg_type                         (Dmg_type::physical),
    knocks_back                      (false),
    hit_small_sfx                     (Sfx_id::END),
    hit_medium_sfx                    (Sfx_id::END),
    hit_hard_sfx                      (Sfx_id::END),
    miss_sfx                         (Sfx_id::END) {}

Item_data_t::Item_melee_data::~Item_melee_data()
{
    if (prop_applied) {delete prop_applied;}
}

Item_data_t::Item_ranged_data::Item_ranged_data() :
    is_ranged_wpn                     (false),
    is_throwing_wpn                   (false),
    is_machine_gun                    (false),
    is_shotgun                       (false),
    max_nr_ammo_in_clip                 (0),
    dmg                             (Dice_param()),
    throw_dmg                        (Dice_param()),
    hit_chance_mod                    (0),
    throw_hit_chance_mod               (0),
    effective_range                  (3),
    knocks_back                      (false),
    dmg_info_override                 (""),
    ammo_item_id                      (Item_id::END),
    dmg_type                         (Dmg_type::physical),
    has_infinite_ammo                 (false),
    missile_glyph                    ('/'),
    missile_tile                     (Tile_id::projectile_std_front_slash),
    missile_clr                      (clr_white),
    missile_leaves_trail              (false),
    missile_leaves_smoke              (false),
    att_msgs                         (Item_att_msgs()),
    snd_msg                          (""),
    snd_vol                          (Snd_vol::low),
    makes_ricochet_snd                (false),
    att_sfx                          (Sfx_id::END),
    reload_sfx                       (Sfx_id::END),
    prop_applied                     (nullptr) {}

Item_data_t::Item_ranged_data::~Item_ranged_data()
{
    if (prop_applied) {delete prop_applied;}
}

Item_data_t::Item_armor_data::Item_armor_data() :
    armor_points             (0),
    dmg_to_durability_factor   (0.0) {}

namespace Item_data
{

Item_data_t* data[int(Item_id::END)];

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
    {
        d = Item_data_t(d.id);
    } break;

    case Item_type::melee_wpn:
    {
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
    } break;

    case Item_type::melee_wpn_intr:
    {
        reset_data(d, Item_type::melee_wpn);
        d.type = Item_type::melee_wpn_intr;
        d.spawn_std_range = Range(-1, -1);
        d.chance_to_include_in_floor_spawn_list = 0;
        d.allow_spawn = false;
        d.melee.hit_small_sfx = Sfx_id::hit_small;
        d.melee.hit_medium_sfx = Sfx_id::hit_medium;
        d.melee.hit_hard_sfx = Sfx_id::hit_hard;
        d.melee.miss_sfx = Sfx_id::miss_medium;
    } break;

    case Item_type::ranged_wpn:
    {
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
        d.ranged.missile_glyph = '/';
        d.ranged.missile_clr = clr_white;
        d.spawn_std_range.upper = DLVL_LAST_MID_GAME;
        d.melee.hit_small_sfx = Sfx_id::hit_small;
        d.melee.hit_medium_sfx = Sfx_id::hit_medium;
        d.melee.hit_hard_sfx = Sfx_id::hit_hard;
        d.melee.miss_sfx = Sfx_id::miss_medium;
        d.ranged.snd_vol = Snd_vol::high;
    } break;

    case Item_type::ranged_wpn_intr:
    {
        reset_data(d, Item_type::ranged_wpn);
        d.type = Item_type::ranged_wpn_intr;
        d.ranged.has_infinite_ammo = true;
        d.spawn_std_range = Range(-1, -1);
        d.chance_to_include_in_floor_spawn_list = 0;
        d.allow_spawn = false;
        d.melee.is_melee_wpn = false;
        d.ranged.missile_glyph = '*';
        d.ranged.snd_vol = Snd_vol::low;
    } break;

    case Item_type::throwing_wpn:
    {
        reset_data(d, Item_type::general);
        d.type = Item_type::throwing_wpn;
        d.weight = Item_weight::extra_light;
        d.is_stackable = true;
        d.ranged.is_throwing_wpn = true;
        d.spawn_std_range.upper = DLVL_LAST_MID_GAME;
        d.ranged.snd_vol = Snd_vol::low;
    } break;

    case Item_type::ammo:
    {
        reset_data(d, Item_type::general);
        d.type = Item_type::ammo;
        d.weight = Item_weight::extra_light;
        d.glyph = '{';
        d.clr = clr_white;
        d.tile = Tile_id::ammo;
        d.spawn_std_range.upper = DLVL_LAST_MID_GAME;
    } break;

    case Item_type::ammo_clip:
    {
        reset_data(d, Item_type::ammo);
        d.type = Item_type::ammo_clip;
        d.weight = Item_weight::light;
        d.is_stackable = false;
        d.spawn_std_range.upper = DLVL_LAST_MID_GAME;
    } break;

    case Item_type::scroll:
    {
        reset_data(d, Item_type::general);
        d.type = Item_type::scroll;
        d.base_descr =
        {
            "A short transcription of an eldritch incantation. There is a strange aura "
            "about it, as if some power was imbued in the paper itself.",
            "It should be possible to pronounce it correctly, but the purpose is unclear."
        };
        d.value = Item_value::minor_treasure;
        d.chance_to_include_in_floor_spawn_list = 40;
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
    } break;

    case Item_type::potion:
    {
        reset_data(d, Item_type::general);
        d.type = Item_type::potion;
        d.base_descr =
        {
            "A small glass bottle containing a mysterious concoction."
        };
        d.value = Item_value::minor_treasure;
        d.chance_to_include_in_floor_spawn_list = 55;
        d.weight = Item_weight::light;
        d.is_identified = false;
        d.glyph = '!';
        d.tile = Tile_id::potion;
        d.ranged.is_throwing_wpn = true;
        d.ranged.throw_hit_chance_mod = 15;
        d.ranged.throw_dmg = Dice_param(1, 3, 0);
        d.max_stack_at_spawn = 2;
        d.land_on_hard_snd_msg = "";
        add_feature_found_in(d, Feature_id::chest);
        add_feature_found_in(d, Feature_id::tomb);
        add_feature_found_in(d, Feature_id::cabinet, 25);
        add_feature_found_in(d, Feature_id::cocoon, 25);
    } break;

    case Item_type::device:
    {
        reset_data(d, Item_type::general);
        d.type = Item_type::device;
        d.base_name_un_id = {"Strange Device", "Strange Devices", "a Strange Device"};
        d.base_descr =
        {
            "A small piece of machinery. It could not possibly have been designed by a "
            "human mind. Even for its small size, it seems incredibly complex. There is "
            "no hope of understanding the purpose or function of it through normal means."
        };
        d.chance_to_include_in_floor_spawn_list = 12;
        d.weight = Item_weight::light;
        d.is_identified = false;
        d.glyph = '~';
        d.tile = Tile_id::device1;
        d.is_stackable = false;
        d.land_on_hard_snd_msg = "I hear a clanking sound.";
        d.land_on_hard_sfx = Sfx_id::metal_clank;
        add_feature_found_in(d, Feature_id::chest, 10);
    } break;

    case Item_type::armor:
    {
        reset_data(d, Item_type::general);
        d.type = Item_type::armor;
        d.weight = Item_weight::heavy;
        d.glyph = '[';
        d.tile = Tile_id::armor;
        d.is_stackable = false;
    } break;

    case Item_type::head_wear:
    {
        reset_data(d, Item_type::general);
        d.type = Item_type::head_wear;
        d.glyph = '[';
        d.is_stackable = false;
    } break;

    case Item_type::amulet:
    {
        reset_data(d, Item_type::general);
        d.type = Item_type::amulet;
        d.value = Item_value::major_treasure;
        d.shock_while_in_backpack = d.shock_while_equipped = 10;
        d.tile = Tile_id::amulet;
        d.glyph = '\"';
        d.weight = Item_weight::light;
        d.is_identified = false;
        d.is_stackable = false;
        d.chance_to_include_in_floor_spawn_list = 1;
        add_feature_found_in(d, Feature_id::tomb, 16);
        add_feature_found_in(d, Feature_id::chest, 5);
    } break;

    case Item_type::ring:
    {
        reset_data(d, Item_type::general);
        d.type = Item_type::ring;
        d.value = Item_value::major_treasure;
        d.shock_while_in_backpack = d.shock_while_equipped = 10;
        d.tile = Tile_id::ring;
        d.glyph = '=';
        d.weight = Item_weight::extra_light;
        d.is_identified = false;
        d.is_stackable = false;
        d.chance_to_include_in_floor_spawn_list = 1;
        add_feature_found_in(d, Feature_id::tomb, 16);
        add_feature_found_in(d, Feature_id::chest, 5);
    } break;

    case Item_type::explosive:
    {
        reset_data(d, Item_type::general);
        d.type = Item_type::explosive;
        d.weight = Item_weight::light;
        d.glyph = '-';
        d.max_stack_at_spawn = 2;
        d.land_on_hard_snd_msg = "";
    } break;

    default: {} break;
    }
}

void set_dmg_from_mon_id(Item_data_t& item_data, const Actor_id id)
{
    const auto& actor_data     = Actor_data::data[int(id)];
    item_data.melee.dmg        = pair<int, int>(1, actor_data.dmg_melee);
    item_data.ranged.dmg       = Dice_param(1, actor_data.dmg_ranged, 0);
    item_data.ranged.throw_dmg  = Dice_param(1, actor_data.dmg_ranged, 0);
}

//------------------------------- LIST OF ITEMS
void init_data_list()
{
    Item_data_t* d = nullptr;

    d = new Item_data_t(Item_id::trapezohedron);
    reset_data(*d, Item_type::general);
    d->base_name =
    {
        "Shining Trapezohedron", "Shining Trapezohedrons", "The Shining Trapezohedron"
    };
    d->spawn_std_range = Range(-1, -1);
    d->chance_to_include_in_floor_spawn_list = 0;
    d->allow_spawn = false;
    d->is_stackable = false;
    d->glyph = '*';
    d->clr = clr_red_lgt;
    d->tile = Tile_id::trapezohedron;
    data[size_t(d->id)] = d;

    d = new Item_data_t(Item_id::sawed_off);
    reset_data(*d, Item_type::ranged_wpn);
    d->base_name =
    {
        "Sawed-off Shotgun", "Sawed-off shotguns", "a Sawed-off Shotgun"
    };
    d->base_descr =
    {
        "Compared to a standard shotgun, the sawed-off has a shorter effective range. "
        "At close range it is more devastating however. It holds two barrels, and needs "
        "to be reloaded after both are discharged"
    };
    d->weight = Item_weight::medium;
    d->tile = Tile_id::shotgun;
    d->ranged.is_shotgun = true;
    d->melee.att_msgs = {"strike", "strikes me with a shotgun"};
    d->ranged.dmg = Dice_param(8, 3);
    d->ranged.effective_range = 3;
    d->ranged.ammo_item_id = Item_id::shotgun_shell;
    d->ranged.att_msgs = {"fire", "fires a shotgun"};
    d->ranged.snd_msg = "I hear a shotgun blast.";
    d->ranged.att_sfx = Sfx_id::shotgun_sawed_off_fire;
    d->ranged.makes_ricochet_snd = true;
    d->ranged.reload_sfx = Sfx_id::shotgun_reload;
    add_feature_found_in(*d, Feature_id::chest);
    add_feature_found_in(*d, Feature_id::cabinet);
    add_feature_found_in(*d, Feature_id::cocoon);
    data[size_t(d->id)] = d;

    d = new Item_data_t(Item_id::pump_shotgun);
    reset_data(*d, Item_type::ranged_wpn);
    d->base_name = {"Pump Shotgun", "Pump shotguns", "a Pump Shotgun"};
    d->base_descr =
    {
        "A pump-action shotgun has a handgrip that can be pumped back and forth in "
        "order to eject a spent round of ammunition and to chamber a fresh one. It has "
        "a single barrel above a tube magazine into which shells are inserted. The "
        "magazine has a capacity of 8 shells."
    };
    d->weight = Item_weight::medium;
    d->tile = Tile_id::shotgun;
    d->ranged.is_shotgun = true;
    d->melee.att_msgs = {"strike", "strikes me with a shotgun"};
    d->ranged.dmg = Dice_param(6, 3);
    d->ranged.effective_range = 5;
    d->ranged.ammo_item_id = Item_id::shotgun_shell;
    d->ranged.att_msgs = {"fire", "fires a shotgun"};
    d->ranged.snd_msg = "I hear a shotgun blast.";
    d->ranged.att_sfx = Sfx_id::shotgun_pump_fire ;
    d->ranged.makes_ricochet_snd = true;
    d->ranged.reload_sfx = Sfx_id::shotgun_reload;
    add_feature_found_in(*d, Feature_id::chest);
    add_feature_found_in(*d, Feature_id::cabinet);
    add_feature_found_in(*d, Feature_id::cocoon);
    data[size_t(d->id)] = d;

    d = new Item_data_t(Item_id::shotgun_shell);
    reset_data(*d, Item_type::ammo);
    d->base_name = {"Shotgun shell", "Shotgun shells", "a shotgun shell"};
    d->base_descr =
    {
        "A cartridge designed to be fired from a shotgun."
    };
    d->max_stack_at_spawn = 10;
    add_feature_found_in(*d, Feature_id::chest);
    add_feature_found_in(*d, Feature_id::cabinet);
    add_feature_found_in(*d, Feature_id::cocoon);
    data[size_t(d->id)] = d;

    d = new Item_data_t(Item_id::incinerator);
    reset_data(*d, Item_type::ranged_wpn);
    d->base_name = {"Incinerator", "Incinerators", "an Incinerator"};
    d->base_descr =
    {
        "This hellish, experimental weapon launches an explosive fireball. Best ued "
        "with extreme caution."
    };
    d->weight = Item_weight::heavy;
    d->tile = Tile_id::incinerator;
    d->melee.att_msgs = {"strike", "strikes me with an Incinerator"};
    d->ranged.dmg = Dice_param(1, 3);
    d->ranged.effective_range = 8;
    d->ranged.dmg_info_override = "* ";
    d->ranged.ammo_item_id = Item_id::incinerator_ammo;
    d->ranged.att_msgs = {"fire", "fires an incinerator"};
    d->ranged.snd_msg = "I hear the blast of a launched missile.";
    d->ranged.missile_glyph = '*';
    d->ranged.missile_clr = clr_red_lgt;
    d->spawn_std_range.lower = 5;
    d->chance_to_include_in_floor_spawn_list = 25;
    add_feature_found_in(*d, Feature_id::chest, 25);
    add_feature_found_in(*d, Feature_id::cabinet, 25);
    add_feature_found_in(*d, Feature_id::cocoon, 25);
    data[size_t(d->id)] = d;

    d = new Item_data_t(Item_id::incinerator_ammo);
    reset_data(*d, Item_type::ammo_clip);
    d->base_name =
    {
        "Incinerator Cartridge", "Incinerator Cartridges", "an Incinerator Cartridge"
    };
    d->base_descr =
    {
        "Ammunition designed for Incinerators."
    };
    d->weight = Item_weight::light;
    d->ranged.max_nr_ammo_in_clip = 5;
    d->spawn_std_range.lower = 5;
    d->max_stack_at_spawn = 1;
    d->chance_to_include_in_floor_spawn_list = 25;
    add_feature_found_in(*d, Feature_id::chest, 25);
    add_feature_found_in(*d, Feature_id::cabinet, 25);
    add_feature_found_in(*d, Feature_id::cocoon, 25);
    data[size_t(d->id)] = d;

    d = new Item_data_t(Item_id::machine_gun);
    reset_data(*d, Item_type::ranged_wpn);
    d->base_name = {"Tommy Gun", "Tommy Guns", "a Tommy Gun"};
    d->base_descr =
    {
        "\"Tommy Gun\" is a nickname for the Thompson submachine gun-an automatic "
        "firearm with a drum magazine and verical foregrip. It fires .45 ACP "
        "ammunition. The drum magazine has a capacity of 50 rounds."
    };
    d->weight = Item_weight::medium;
    d->tile = Tile_id::tommy_gun;
    d->melee.att_msgs = {"strike", "strikes me with a Tommy Gun"};
    d->ranged.is_machine_gun = true;
    d->ranged.dmg = Dice_param(2, 2, 2);
    d->ranged.hit_chance_mod = -10;
    d->ranged.effective_range = 8;
    d->ranged.ammo_item_id = Item_id::drum_of_bullets;
    d->ranged.att_msgs = {"fire", "fires a Tommy Gun"};
    d->ranged.snd_msg = "I hear the burst of a machine gun.";
    d->ranged.att_sfx = Sfx_id::machine_gun_fire;
    d->ranged.makes_ricochet_snd = true;
    d->ranged.reload_sfx = Sfx_id::machine_gun_reload;
    add_feature_found_in(*d, Feature_id::chest);
    add_feature_found_in(*d, Feature_id::cabinet);
    add_feature_found_in(*d, Feature_id::cocoon);
    data[size_t(d->id)] = d;

    d = new Item_data_t(Item_id::drum_of_bullets);
    reset_data(*d, Item_type::ammo_clip);
    d->base_name = {"Drum of .45 ACP", "Drums of .45 ACP", "a Drum of .45 ACP"};
    d->base_descr =
    {
        "Ammunition used by Tommy Guns."
    };
    d->ranged.max_nr_ammo_in_clip = 50;
    add_feature_found_in(*d, Feature_id::chest);
    add_feature_found_in(*d, Feature_id::cabinet);
    add_feature_found_in(*d, Feature_id::cocoon);
    data[size_t(d->id)] = d;

    d = new Item_data_t(Item_id::pistol);
    reset_data(*d, Item_type::ranged_wpn);
    d->base_name = {"M1911 Colt", "M1911 Colt", "an M1911 Colt"};
    d->base_descr =
    {
        "A semi-automatic, magazine-fed pistol chambered for the .45 ACP cartridge."
    };
    d->weight = Item_weight::light;
    d->tile = Tile_id::pistol;
    d->ranged.dmg = Dice_param(1, 8, 4);
    d->ranged.effective_range = 6;
    d->ranged.ammo_item_id = Item_id::pistol_clip;
    d->melee.att_msgs = {"strike", "strikes me with a pistol"};
    d->ranged.att_msgs = {"fire", "fires a pistol"};
    d->ranged.snd_msg = "I hear a pistol being fired.";
    d->ranged.att_sfx = Sfx_id::pistol_fire;
    d->ranged.makes_ricochet_snd = true;
    d->ranged.reload_sfx = Sfx_id::pistol_reload;
    add_feature_found_in(*d, Feature_id::chest);
    add_feature_found_in(*d, Feature_id::cabinet);
    add_feature_found_in(*d, Feature_id::cocoon);
    data[size_t(d->id)] = d;

    d = new Item_data_t(Item_id::pistol_clip);
    reset_data(*d, Item_type::ammo_clip);
    d->base_name = {".45ACP Colt cartridge", ".45ACP Colt cartridges",
                   "a .45ACP Colt cartridge"
                  };
    d->base_descr =
    {
        "Ammunition used by Colt pistols."
    };
    d->ranged.max_nr_ammo_in_clip = 7;
    add_feature_found_in(*d, Feature_id::chest);
    add_feature_found_in(*d, Feature_id::cabinet);
    add_feature_found_in(*d, Feature_id::cocoon);
    data[size_t(d->id)] = d;

    d = new Item_data_t(Item_id::mi_go_gun);
    reset_data(*d, Item_type::ranged_wpn);
    d->base_name =
    {
        "Mi-go Electric Gun", "Mi-go Electric Gun", "a Mi-go Electric Gun"
    };
    d->base_descr =
    {
        "A weapon created by the Mi-go. It fires devastating bolts of electricity.",
        "If there is no ammunition to fuel the weapon, it can draw power from the "
        "essence of the wielder (press [f] while no ammo loaded)."
    };
    d->spawn_std_range = Range(-1, -1);
    d->weight = Item_weight::medium;
    d->tile = Tile_id::mi_go_gun;
    d->clr = clr_yellow;
    d->ranged.dmg = Dice_param(3, 6, 0);
    d->ranged.hit_chance_mod = 5;
    d->ranged.effective_range = 4;
    d->ranged.prop_applied = new Prop_paralyzed(Prop_turns::specific, 2);
    d->ranged.dmg_type = Dmg_type::electric;
    d->ranged.ammo_item_id = Item_id::mi_go_gun_ammo;
    d->ranged.has_infinite_ammo = false;
    d->ranged.missile_leaves_trail = true;
    d->ranged.missile_clr = clr_yellow;
    d->melee.att_msgs = {"strike", "strikes me with a Mi-go Electric Gun"};
    d->ranged.att_msgs = {"fire", "fires a Mi-go Electric Gun"};
    d->ranged.snd_msg = "I hear a bolt of electricity.";
    d->ranged.att_sfx = Sfx_id::mi_go_gun_fire;
    d->ranged.reload_sfx = Sfx_id::machine_gun_reload;
    d->ranged.makes_ricochet_snd = false;
    data[size_t(d->id)] = d;

    d = new Item_data_t(Item_id::mi_go_gun_ammo);
    reset_data(*d, Item_type::ammo_clip);
    d->base_name =
    {
        "Mi-go Electric Cell", "Mi-go Electric Cells", "a Mi-go Electric Cell"
    };
    d->base_descr =
    {
        "Ammunition for the Mi-go Electric gun."
    };
    d->clr = clr_yellow;
    d->spawn_std_range = Range(-1, -1);
    d->ranged.max_nr_ammo_in_clip = 20;
    data[size_t(d->id)] = d;

    d = new Item_data_t(Item_id::flare_gun);
    reset_data(*d, Item_type::ranged_wpn);
    d->base_name = {"Flare Gun", "Flare Gun", "a Flare Gun"};
    d->base_descr =
    {
        "Launches flares. Not designed to function as a weapon."
    };
    d->weight = Item_weight::light;
    d->tile = Tile_id::flare_gun;
    d->ranged.dmg = Dice_param(1, 3, 0);
    d->ranged.effective_range = 3;
    d->ranged.dmg_info_override = "*";
    d->ranged.ammo_item_id = Item_id::flare;
    d->melee.att_msgs = {"strike", "strikes me with a flare gun"};
    d->ranged.att_msgs = {"fire", "fires a flare gun"};
    d->ranged.snd_msg = "I hear a flare gun being fired.";
    d->ranged.prop_applied = new Prop_flared(Prop_turns::std);
    add_feature_found_in(*d, Feature_id::chest);
    add_feature_found_in(*d, Feature_id::cabinet);
    add_feature_found_in(*d, Feature_id::cocoon);
    data[size_t(d->id)] = d;

    d = new Item_data_t(Item_id::spike_gun);
    reset_data(*d, Item_type::ranged_wpn);
    d->base_name = {"Spike Gun", "Spike Guns", "a Spike Gun"};
    d->base_descr =
    {
        "A very strange and crude weapon capable of launching iron spikes with enough "
        "force to pierce flesh (or even rock). It seems almost to be deliberately "
        "designed for cruelty, rather than pure stopping power."
    };
    d->weight = Item_weight::medium;
    d->tile = Tile_id::tommy_gun;
    d->clr = clr_blue_lgt;
    d->melee.att_msgs = {"strike", "strikes me with a Spike Gun"};
    d->ranged.is_machine_gun = false;
    d->ranged.hit_chance_mod = 0;
    d->ranged.dmg = Dice_param(1, 7, 0);
    d->ranged.effective_range = 4;
    d->ranged.dmg_type = Dmg_type::physical;
    d->ranged.knocks_back = true;
    d->ranged.ammo_item_id = Item_id::iron_spike;
    d->ranged.att_msgs = {"fire", "fires a Spike Gun"};
    d->ranged.snd_msg = "I hear a very crude weapon being fired.";
    d->ranged.makes_ricochet_snd = true;
    d->ranged.missile_glyph = '/';
    d->ranged.missile_clr = clr_gray;
    d->spawn_std_range.lower = 4;
    d->ranged.att_sfx = Sfx_id::spike_gun;
    d->ranged.snd_vol = Snd_vol::low;
    add_feature_found_in(*d, Feature_id::chest, 50);
    add_feature_found_in(*d, Feature_id::cabinet, 50);
    add_feature_found_in(*d, Feature_id::cocoon, 50);
    data[size_t(d->id)] = d;

    d = new Item_data_t(Item_id::dynamite);
    reset_data(*d, Item_type::explosive);
    d->base_name = {"Dynamite", "Sticks of Dynamite", "a Stick of Dynamite"};
    d->base_descr =
    {
        "An explosive material based on nitroglycerin. The name comes from the ancient "
        "Greek word for \"power\"."
    };
    d->weight = Item_weight::light;
    d->tile = Tile_id::dynamite;
    d->clr = clr_red_lgt;
    add_feature_found_in(*d, Feature_id::chest);
    add_feature_found_in(*d, Feature_id::cabinet);
    add_feature_found_in(*d, Feature_id::cocoon);
    data[size_t(d->id)] = d;

    d = new Item_data_t(Item_id::flare);
    reset_data(*d, Item_type::explosive);
    d->base_name = {"Flare", "Flares", "a Flare"};
    d->base_descr =
    {
        "A type of pyrotechnic that produces a brilliant light or intense heat without "
        "an explosion."
    };
    d->weight = Item_weight::light;
    d->tile = Tile_id::flare;
    d->clr = clr_gray;
    add_feature_found_in(*d, Feature_id::chest);
    add_feature_found_in(*d, Feature_id::cabinet);
    add_feature_found_in(*d, Feature_id::cocoon);
    data[size_t(d->id)] = d;

    d = new Item_data_t(Item_id::molotov);
    reset_data(*d, Item_type::explosive);
    d->base_name = {"Molotov Cocktail", "Molotov Cocktails", "a Molotov Cocktail"};
    d->base_descr =
    {
        "An improvised incendiary weapon made of a glass bottle containing flammable "
        "liquid and some cloth for ignition. In action, the cloth is lit and the bottle "
        "hurled at a target, causing an immediate fireball followed by a raging fire."
    };
    d->weight = Item_weight::light;
    d->tile = Tile_id::molotov;
    d->clr = clr_white;
    add_feature_found_in(*d, Feature_id::chest);
    add_feature_found_in(*d, Feature_id::cabinet);
    add_feature_found_in(*d, Feature_id::cocoon);
    data[size_t(d->id)] = d;

    d = new Item_data_t(Item_id::smoke_grenade);
    reset_data(*d, Item_type::explosive);
    d->base_name = {"Smoke Grenade", "Smoke Grenades", "a Smoke Grenade"};
    d->base_descr =
    {
        "A sheet steel cylinder with emission holes releasing smoke when the grenade is "
        "ignited. Their primary use is to create smoke screens for concealment. "
        "The fumes produced can harm the eyes, throat and lungs - so it is "
        "recommended to wear a protective mask."
    };
    d->weight = Item_weight::light;
    d->tile = Tile_id::flare;
    d->clr = clr_green;
    add_feature_found_in(*d, Feature_id::chest);
    add_feature_found_in(*d, Feature_id::cabinet);
    data[size_t(d->id)] = d;

    d = new Item_data_t(Item_id::thr_knife);
    reset_data(*d, Item_type::throwing_wpn);
    d->base_name = {"Throwing Knife", "Throwing Knives", "a Throwing Knife"};
    d->base_descr =
    {
        "A knife specially designed and weighted so that it can be thrown effectively."
    };
    d->weight = Item_weight::extra_light;
    d->tile = Tile_id::dagger;
    d->glyph = '/';
    d->clr = clr_white;
    d->ranged.throw_hit_chance_mod = 0;
    d->ranged.throw_dmg = Dice_param(2, 4);
    d->ranged.effective_range = 5;
    d->max_stack_at_spawn = 8;
    d->land_on_hard_snd_msg = "I hear a clanking sound.";
    d->land_on_hard_sfx = Sfx_id::metal_clank;
    d->main_att_mode = Main_att_mode::thrown;
    add_feature_found_in(*d, Feature_id::chest);
    add_feature_found_in(*d, Feature_id::cabinet);
    add_feature_found_in(*d, Feature_id::cocoon);
    data[size_t(d->id)] = d;

    d = new Item_data_t(Item_id::rock);
    reset_data(*d, Item_type::throwing_wpn);
    d->base_name = {"Rock", "Rocks", "a Rock"};
    d->base_descr =
    {
        "Although not a very impressive weapon, with skill they can be used with some "
        "result."
    };
    d->weight = Item_weight::extra_light;
    d->tile = Tile_id::rock;
    d->glyph = '*';
    d->clr = clr_gray;
    d->ranged.throw_hit_chance_mod = 10;
    d->ranged.throw_dmg = Dice_param(1, 3);
    d->ranged.effective_range = 4;
    d->max_stack_at_spawn = 6;
    d->main_att_mode = Main_att_mode::thrown;
    add_feature_found_in(*d, Feature_id::cabinet);
    add_feature_found_in(*d, Feature_id::cocoon);
    data[size_t(d->id)] = d;

    d = new Item_data_t(Item_id::dagger);
    reset_data(*d, Item_type::melee_wpn);
    d->base_name = {"Dagger", "Daggers", "a Dagger"};
    d->base_descr =
    {
        "Commonly associated with deception, stealth, and treachery. Many "
        "assassinations have been carried out with the use of a dagger.",

        "Attacking an unaware opponent with a dagger does 300% damage (instead of the "
        "normal 150% damage from stealth attacks).",

        "Melee attacks with daggers are silent."
    };
    d->weight = Item_weight::light;
    d->tile = Tile_id::dagger;
    d->melee.att_msgs = {"stab", "stabs me with a Dagger"};
    d->melee.dmg = pair<int, int>(1, 4);
    d->melee.hit_chance_mod = 20;
    d->melee.hit_medium_sfx = Sfx_id::hit_sharp;
    d->melee.hit_hard_sfx = Sfx_id::hit_sharp;
    d->melee.miss_sfx = Sfx_id::miss_light;
    add_feature_found_in(*d, Feature_id::chest);
    add_feature_found_in(*d, Feature_id::cabinet);
    add_feature_found_in(*d, Feature_id::tomb);
    add_feature_found_in(*d, Feature_id::cocoon);
    data[size_t(d->id)] = d;

    d = new Item_data_t(Item_id::hatchet);
    reset_data(*d, Item_type::melee_wpn);
    d->base_name = {"Hatchet", "Hatchets", "a Hatchet"};
    d->base_descr =
    {
        "A small axe with a short handle. Hatchets are reliable weapons - they are easy "
        "to use, and cause decent damage for their low weight. "
        /*TODO: "They can also serve well as thrown weapons."*/,

        "Melee attacks with hatchets are silent."
    };
    d->weight = Item_weight::light;
    d->tile = Tile_id::axe;
    d->melee.att_msgs = {"strike", "strikes me with a Hatchet"};
    d->melee.dmg = pair<int, int>(1, 5);
    d->melee.hit_chance_mod = 15;
    d->ranged.throw_hit_chance_mod = -5;
    d->ranged.throw_dmg = Dice_param(1, 10);
    d->ranged.is_throwing_wpn = false;
    d->melee.hit_medium_sfx = Sfx_id::hit_sharp;
    d->melee.hit_hard_sfx = Sfx_id::hit_sharp;
    d->melee.miss_sfx = Sfx_id::miss_light;
    add_feature_found_in(*d, Feature_id::chest);
    add_feature_found_in(*d, Feature_id::cabinet);
    add_feature_found_in(*d, Feature_id::cocoon);
    data[size_t(d->id)] = d;

    d = new Item_data_t(Item_id::club);
    reset_data(*d, Item_type::melee_wpn);
    d->base_name = {"Club", "Clubs", "a Club"};
    d->base_descr =
    {
        "Wielded since prehistoric times.",

        "Melee attacks with clubs are noisy."
    };
    d->spawn_std_range = Range(DLVL_FIRST_LATE_GAME, INT_MAX);
    d->weight = Item_weight::medium;
    d->tile = Tile_id::club;
    d->clr = clr_brown;
    d->melee.att_msgs = {"strike", "strikes me with a Club"};
    d->melee.dmg = pair<int, int>(2, 3);
    d->melee.hit_chance_mod = 10;
    d->melee.miss_sfx = Sfx_id::miss_medium;
    data[size_t(d->id)] = d;

    d = new Item_data_t(Item_id::hammer);
    reset_data(*d, Item_type::melee_wpn);
    d->base_name = {"Hammer", "Hammers", "a Hammer"};
    d->base_descr =
    {
        "Typically used for construction, but can be quite devastating when wielded as "
        "a weapon.",

        "Melee attacks with hammers are noisy."
    };
    d->weight = Item_weight::medium;
    d->tile = Tile_id::hammer;
    d->melee.att_msgs = {"strike", "strikes me with a Hammer"};
    d->melee.dmg = pair<int, int>(2, 4);
    d->melee.hit_chance_mod = 5;
    d->melee.miss_sfx = Sfx_id::miss_medium;
    add_feature_found_in(*d, Feature_id::cabinet);
    add_feature_found_in(*d, Feature_id::cocoon);
    data[size_t(d->id)] = d;

    d = new Item_data_t(Item_id::machete);
    reset_data(*d, Item_type::melee_wpn);
    d->base_name = {"Machete", "Machetes", "a Machete"};
    d->base_descr =
    {
        "A large cleaver-like knife. It serves well both as a cutting tool and weapon.",

        "Melee attacks with machetes are noisy."
    };
    d->weight = Item_weight::medium;
    d->tile = Tile_id::machete;
    d->melee.att_msgs = {"strike", "strikes me with a Machete"};
    d->melee.dmg = pair<int, int>(2, 5);
    d->melee.hit_chance_mod = 0;
    d->melee.hit_small_sfx = Sfx_id::hit_sharp;
    d->melee.hit_medium_sfx = Sfx_id::hit_sharp;
    d->melee.miss_sfx = Sfx_id::miss_medium;
    add_feature_found_in(*d, Feature_id::cabinet);
    add_feature_found_in(*d, Feature_id::cocoon);
    data[size_t(d->id)] = d;

    d = new Item_data_t(Item_id::axe);
    reset_data(*d, Item_type::melee_wpn);
    d->base_name = {"Axe", "Axes", "an Axe"};
    d->base_descr =
    {
        "A tool intended for felling trees, splitting timber, etc. Used as a weapon it "
        "can deliver devastating blows, although it requires some skill to use "
        "effectively. Also effective for breaching wooden doors.",

        "Melee attacks with axes are noisy."
    };
    d->weight = Item_weight::medium;
    d->tile = Tile_id::axe;
    d->melee.att_msgs = {"strike", "strikes me with an axe"};
    d->melee.dmg = pair<int, int>(2, 6);
    d->melee.hit_chance_mod = -5;
    d->melee.hit_small_sfx = Sfx_id::hit_small;
    d->melee.hit_medium_sfx = Sfx_id::hit_medium;
    d->melee.hit_hard_sfx = Sfx_id::hit_hard;
    d->melee.miss_sfx = Sfx_id::miss_medium;
    add_feature_found_in(*d, Feature_id::cabinet);
    add_feature_found_in(*d, Feature_id::tomb);
    add_feature_found_in(*d, Feature_id::cocoon);
    data[size_t(d->id)] = d;

    d = new Item_data_t(Item_id::pitch_fork);
    reset_data(*d, Item_type::melee_wpn);
    d->base_name = {"Pitchfork", "Pitchforks", "a Pitchfork"};
    d->base_descr =
    {
        "A long staff with a forked, four-pronged end. Victims can be pushed away when "
        "stabbed, to keep them at bay."
    };
    d->weight = Item_weight::heavy;
    d->tile = Tile_id::pitchfork;
    d->melee.att_msgs = {"strike", "strikes me with a Pitchfork"};
    d->melee.dmg = pair<int, int>(3, 4);
    d->melee.hit_chance_mod = -5;
    d->melee.knocks_back = true;
    d->melee.hit_small_sfx = Sfx_id::hit_sharp;
    d->melee.hit_medium_sfx = Sfx_id::hit_sharp;
    d->melee.miss_sfx = Sfx_id::miss_heavy;
    add_feature_found_in(*d, Feature_id::cabinet);
    add_feature_found_in(*d, Feature_id::cocoon);
    data[size_t(d->id)] = d;

    d = new Item_data_t(Item_id::sledge_hammer);
    reset_data(*d, Item_type::melee_wpn);
    d->base_name = {"Sledgehammer", "Sledgehammers", "a Sledgehammer"};
    d->base_descr =
    {
        "Often used in destruction work for breaking through walls. It can deal a great "
        "amount of damage, although it is cumbersome to carry, and it requires some "
        "skill to use effectively."
    };
    d->weight = Item_weight::heavy;
    d->tile = Tile_id::sledge_hammer;
    d->melee.att_msgs = {"strike", "strikes me with a Sledgehammer"};
    d->melee.dmg = pair<int, int>(3, 5);
    d->melee.hit_chance_mod = -10;
    d->melee.knocks_back = true;
    d->melee.miss_sfx = Sfx_id::miss_heavy;
    add_feature_found_in(*d, Feature_id::cabinet);
    data[size_t(d->id)] = d;

    d = new Item_data_t(Item_id::pharaoh_staff);
    reset_data(*d, Item_type::melee_wpn);
    d->base_name =
    {
        "Staff of the Pharaohs", "Staff of the Pharaohs", "the Staff of the Pharaohs"
    };
    d->base_descr =
    {
        "Once wielded by long-forgotten kings in ancient times, this powerful artifact "
        "grants the power to call up a loyal servant from the dead.",

        item_carry_shock_descr
    };
    d->clr = clr_magenta;
    d->weight = Item_weight::medium;
    d->tile = Tile_id::pharaoh_staff;
    d->melee.att_msgs = {"strike", "strikes me with the Staff of the Pharaohs"};
    d->melee.dmg = pair<int, int>(2, 4);
    d->melee.hit_chance_mod = 0;
    d->melee.miss_sfx = Sfx_id::miss_medium;
    d->chance_to_include_in_floor_spawn_list = 1;
    d->value = Item_value::major_treasure;
    d->shock_while_in_backpack = d->shock_while_equipped = 15;
    add_feature_found_in(*d, Feature_id::tomb, 20);
    data[size_t(d->id)] = d;

    d = new Item_data_t(Item_id::iron_spike);
    reset_data(*d, Item_type::throwing_wpn);
    d->base_name = {"Iron Spike", "Iron Spikes", "an Iron Spike"};
    d->base_descr =
    {
        "Can be useful for wedging things closed."
        /*TODO: or prying things open."*/
    };
    d->weight = Item_weight::extra_light;
    d->tile = Tile_id::iron_spike;
    d->is_stackable = true;
    d->clr = clr_gray;
    d->glyph = '/';
    d->ranged.throw_hit_chance_mod = -5;
    d->ranged.throw_dmg = Dice_param(1, 3);
    d->ranged.effective_range = 3;
    d->max_stack_at_spawn = 12;
    d->land_on_hard_snd_msg = "I hear a clanking sound.";
    d->land_on_hard_sfx = Sfx_id::metal_clank;
    d->main_att_mode = Main_att_mode::thrown;
    add_feature_found_in(*d, Feature_id::cabinet);
    add_feature_found_in(*d, Feature_id::cocoon);
    data[size_t(d->id)] = d;

    d = new Item_data_t(Item_id::player_kick);
    reset_data(*d, Item_type::melee_wpn_intr);
    d->melee.att_msgs = {"kick", ""};
    d->melee.hit_chance_mod = 20;
    d->melee.dmg = pair<int, int>(1, 3);
    d->melee.knocks_back = true;
    d->melee.miss_sfx = Sfx_id::miss_medium;
    data[size_t(d->id)] = d;

    d = new Item_data_t(Item_id::player_stomp);
    reset_data(*d, Item_type::melee_wpn_intr);
    d->melee.att_msgs = {"stomp", ""};
    d->melee.hit_chance_mod = 20;
    d->melee.dmg = pair<int, int>(1, 3);
    d->melee.knocks_back = false;
    data[size_t(d->id)] = d;

    d = new Item_data_t(Item_id::player_punch);
    reset_data(*d, Item_type::melee_wpn_intr);
    d->melee.att_msgs = {"punch", ""};
    d->melee.hit_chance_mod = 25;
    d->melee.dmg = pair<int, int>(1, 2);
    data[size_t(d->id)] = d;

    d = new Item_data_t(Item_id::zombie_claw);
    reset_data(*d, Item_type::melee_wpn_intr);
    d->melee.att_msgs = {"", "claws me"};
    set_dmg_from_mon_id(*d, Actor_id::zombie);
    data[size_t(d->id)] = d;

    d = new Item_data_t(Item_id::zombie_claw_diseased);
    reset_data(*d, Item_type::melee_wpn_intr);
    d->melee.att_msgs = {"", "claws me"};
    set_dmg_from_mon_id(*d, Actor_id::zombie);
    d->melee.prop_applied = new Prop_infected(Prop_turns::std);
    data[size_t(d->id)] = d;

    d = new Item_data_t(Item_id::zombie_axe);
    reset_data(*d, Item_type::melee_wpn_intr);
    d->melee.att_msgs = {"", "chops me with a rusty axe"};
    d->melee.hit_small_sfx = Sfx_id::hit_small;
    d->melee.hit_medium_sfx = Sfx_id::hit_medium;
    d->melee.hit_hard_sfx = Sfx_id::hit_hard;
    set_dmg_from_mon_id(*d, Actor_id::zombie_axe);
    data[size_t(d->id)] = d;

    d = new Item_data_t(Item_id::bloated_zombie_punch);
    reset_data(*d, Item_type::melee_wpn_intr);
    d->melee.att_msgs = {"", "mauls me"};
    set_dmg_from_mon_id(*d, Actor_id::bloated_zombie);
    d->melee.knocks_back = true;
    data[size_t(d->id)] = d;

    d = new Item_data_t(Item_id::bloated_zombie_spit);
    reset_data(*d, Item_type::ranged_wpn_intr);
    d->ranged.att_msgs = {"", "spits acid pus at me"};
    set_dmg_from_mon_id(*d, Actor_id::bloated_zombie);
    d->ranged.snd_msg = "I hear spitting.";
    d->ranged.missile_clr = clr_green_lgt;
    d->ranged.dmg_type = Dmg_type::acid;
    d->ranged.missile_glyph = '*';
    data[size_t(d->id)] = d;

    d = new Item_data_t(Item_id::rat_bite);
    reset_data(*d, Item_type::melee_wpn_intr);
    d->melee.att_msgs = {"", "bites me"};
    set_dmg_from_mon_id(*d, Actor_id::rat);
    data[size_t(d->id)] = d;

    d = new Item_data_t(Item_id::rat_bite_diseased);
    reset_data(*d, Item_type::melee_wpn_intr);
    d->melee.att_msgs = {"", "bites me"};
    set_dmg_from_mon_id(*d, Actor_id::rat);
    d->melee.prop_applied = new Prop_infected(Prop_turns::std);
    data[size_t(d->id)] = d;

    d = new Item_data_t(Item_id::rat_thing_bite);
    reset_data(*d, Item_type::melee_wpn_intr);
    d->melee.att_msgs = {"", "bites me"};
    set_dmg_from_mon_id(*d, Actor_id::rat_thing);
    data[size_t(d->id)] = d;

    d = new Item_data_t(Item_id::brown_jenkin_bite);
    reset_data(*d, Item_type::melee_wpn_intr);
    d->melee.att_msgs = {"", "bites me"};
    set_dmg_from_mon_id(*d, Actor_id::brown_jenkin);
    data[size_t(d->id)] = d;

    d = new Item_data_t(Item_id::worm_mass_bite);
    reset_data(*d, Item_type::melee_wpn_intr);
    d->melee.att_msgs = {"", "bites me"};
    set_dmg_from_mon_id(*d, Actor_id::worm_mass);
    data[size_t(d->id)] = d;

    d = new Item_data_t(Item_id::wolf_bite);
    reset_data(*d, Item_type::melee_wpn_intr);
    d->melee.att_msgs = {"", "bites me"};
    set_dmg_from_mon_id(*d, Actor_id::wolf);
    data[size_t(d->id)] = d;

    d = new Item_data_t(Item_id::green_spider_bite);
    reset_data(*d, Item_type::melee_wpn_intr);
    d->melee.att_msgs = {"", "bites me"};
    set_dmg_from_mon_id(*d, Actor_id::green_spider);
    d->melee.prop_applied = new Prop_blind(Prop_turns::specific, 4);
    data[size_t(d->id)] = d;

    d = new Item_data_t(Item_id::white_spider_bite);
    reset_data(*d, Item_type::melee_wpn_intr);
    d->melee.att_msgs = {"", "bites me"};
    set_dmg_from_mon_id(*d, Actor_id::white_spider);
    d->melee.prop_applied = new Prop_paralyzed(Prop_turns::specific, 2);
    data[size_t(d->id)] = d;

    d = new Item_data_t(Item_id::red_spider_bite);
    reset_data(*d, Item_type::melee_wpn_intr);
    d->melee.att_msgs = {"", "bites me"};
    set_dmg_from_mon_id(*d, Actor_id::red_spider);
    d->melee.prop_applied = new Prop_weakened(Prop_turns::std);
    data[size_t(d->id)] = d;

    d = new Item_data_t(Item_id::shadow_spider_bite);
    reset_data(*d, Item_type::melee_wpn_intr);
    d->melee.att_msgs = {"", "bites me"};
    set_dmg_from_mon_id(*d, Actor_id::shadow_spider);
    data[size_t(d->id)] = d;

    d = new Item_data_t(Item_id::leng_spider_bite);
    reset_data(*d, Item_type::melee_wpn_intr);
    d->melee.att_msgs = {"", "bites me"};
    set_dmg_from_mon_id(*d, Actor_id::leng_spider);
    data[size_t(d->id)] = d;

    d = new Item_data_t(Item_id::fire_hound_breath);
    reset_data(*d, Item_type::ranged_wpn_intr);
    d->ranged.att_msgs = {"", "breaths fire at me"};
    d->ranged.snd_msg = "I hear a burst of flames.";
    set_dmg_from_mon_id(*d, Actor_id::fire_hound);
    d->ranged.prop_applied = new Prop_burning(Prop_turns::std);
    d->ranged.missile_clr = clr_red_lgt;
    d->ranged.missile_glyph = '*';
    d->ranged.missile_leaves_trail = true;
    d->ranged.missile_leaves_smoke = true;
    d->ranged.dmg_type = Dmg_type::fire;
    data[size_t(d->id)] = d;

    d = new Item_data_t(Item_id::fire_hound_bite);
    reset_data(*d, Item_type::melee_wpn_intr);
    d->melee.att_msgs = {"", "bites me"};
    set_dmg_from_mon_id(*d, Actor_id::fire_hound);
    d->melee.dmg_type = Dmg_type::fire;
    data[size_t(d->id)] = d;

    d = new Item_data_t(Item_id::frost_hound_breath);
    reset_data(*d, Item_type::ranged_wpn_intr);
    d->ranged.att_msgs = {"", "breaths frost at me"};
    d->ranged.snd_msg = "I hear a chilling sound.";
    set_dmg_from_mon_id(*d, Actor_id::frost_hound);
    d->ranged.missile_clr = clr_blue_lgt;
    d->ranged.missile_glyph = '*';
    d->ranged.missile_leaves_trail = true;
    d->ranged.missile_leaves_smoke = true;
    d->ranged.dmg_type = Dmg_type::cold;
    data[size_t(d->id)] = d;

    d = new Item_data_t(Item_id::frost_hound_bite);
    reset_data(*d, Item_type::melee_wpn_intr);
    d->melee.att_msgs = {"", "bites me"};
    set_dmg_from_mon_id(*d, Actor_id::frost_hound);
    d->melee.dmg_type = Dmg_type::cold;
    data[size_t(d->id)] = d;

    d = new Item_data_t(Item_id::zuul_bite);
    reset_data(*d, Item_type::melee_wpn_intr);
    d->melee.att_msgs = {"", "bites me"};
    set_dmg_from_mon_id(*d, Actor_id::zuul);
    d->melee.dmg_type = Dmg_type::physical;
    data[size_t(d->id)] = d;

    d = new Item_data_t(Item_id::dust_vortex_engulf);
    reset_data(*d, Item_type::melee_wpn_intr);
    d->melee.att_msgs = {"", "engulfs me"};
    set_dmg_from_mon_id(*d, Actor_id::dust_vortex);
    d->melee.prop_applied = new Prop_blind(Prop_turns::std);
    data[size_t(d->id)] = d;

    d = new Item_data_t(Item_id::fire_vortex_engulf);
    reset_data(*d, Item_type::melee_wpn_intr);
    d->melee.att_msgs = {"", "engulfs me"};
    set_dmg_from_mon_id(*d, Actor_id::fire_vortex);
    d->melee.prop_applied = new Prop_burning(Prop_turns::std);
    data[size_t(d->id)] = d;

    d = new Item_data_t(Item_id::frost_vortex_engulf);
    reset_data(*d, Item_type::melee_wpn_intr);
    d->melee.att_msgs = {"", "engulfs me"};
    set_dmg_from_mon_id(*d, Actor_id::frost_vortex);
    data[size_t(d->id)] = d;

    d = new Item_data_t(Item_id::ghost_claw);
    reset_data(*d, Item_type::melee_wpn_intr);
    d->melee.att_msgs = {"", "claws me"};
    set_dmg_from_mon_id(*d, Actor_id::ghost);
    d->melee.prop_applied = new Prop_terrified(Prop_turns::specific, 4);
    d->melee.dmg_type = Dmg_type::spirit;
    data[size_t(d->id)] = d;

    d = new Item_data_t(Item_id::phantasm_sickle);
    reset_data(*d, Item_type::melee_wpn_intr);
    d->melee.att_msgs = {"", "slices me with a sickle"};
    set_dmg_from_mon_id(*d, Actor_id::phantasm);
    d->melee.prop_applied = new Prop_terrified(Prop_turns::specific, 4);
    d->melee.dmg_type = Dmg_type::spirit;
    data[size_t(d->id)] = d;

    d = new Item_data_t(Item_id::wraith_claw);
    reset_data(*d, Item_type::melee_wpn_intr);
    d->melee.att_msgs = {"", "claws me"};
    set_dmg_from_mon_id(*d, Actor_id::wraith);
    d->melee.prop_applied = new Prop_terrified(Prop_turns::specific, 4);
    d->melee.dmg_type = Dmg_type::spirit;
    data[size_t(d->id)] = d;

    d = new Item_data_t(Item_id::giant_bat_bite);
    reset_data(*d, Item_type::melee_wpn_intr);
    d->melee.att_msgs = {"", "bites me"};
    set_dmg_from_mon_id(*d, Actor_id::giant_bat);
    data[size_t(d->id)] = d;

    d = new Item_data_t(Item_id::polyp_tentacle);
    reset_data(*d, Item_type::melee_wpn_intr);
    d->melee.att_msgs = {"", "grips me with a tentacle"};
    d->melee.prop_applied = new Prop_paralyzed(Prop_turns::specific, 1);
    set_dmg_from_mon_id(*d, Actor_id::flying_polyp);
    data[size_t(d->id)] = d;

    d = new Item_data_t(Item_id::greater_polyp_tentacle);
    reset_data(*d, Item_type::melee_wpn_intr);
    d->melee.att_msgs = {"", "grips me with a tentacle"};
    d->melee.prop_applied = new Prop_paralyzed(Prop_turns::specific, 1);
    set_dmg_from_mon_id(*d, Actor_id::greater_polyp);
    data[size_t(d->id)] = d;

    d = new Item_data_t(Item_id::ghoul_claw);
    reset_data(*d, Item_type::melee_wpn_intr);
    d->melee.att_msgs = {"", "claws me"};
    set_dmg_from_mon_id(*d, Actor_id::ghoul);
    d->melee.prop_applied = new Prop_infected(Prop_turns::std);
    data[size_t(d->id)] = d;

    d = new Item_data_t(Item_id::shadow_claw);
    reset_data(*d, Item_type::melee_wpn_intr);
    d->melee.att_msgs = {"", "claws me"};
    set_dmg_from_mon_id(*d, Actor_id::shadow);
    data[size_t(d->id)] = d;

    d = new Item_data_t(Item_id::byakhee_claw);
    reset_data(*d, Item_type::melee_wpn_intr);
    d->melee.att_msgs = {"", "claws me"};
    set_dmg_from_mon_id(*d, Actor_id::byakhee);
    data[size_t(d->id)] = d;

    d = new Item_data_t(Item_id::giant_mantis_claw);
    reset_data(*d, Item_type::melee_wpn_intr);
    d->melee.att_msgs = {"", "claws me"};
    set_dmg_from_mon_id(*d, Actor_id::giant_mantis);
    data[size_t(d->id)] = d;

    d = new Item_data_t(Item_id::giant_locust_bite);
    reset_data(*d, Item_type::melee_wpn_intr);
    d->melee.att_msgs = {"", "bites me"};
    set_dmg_from_mon_id(*d, Actor_id::locust);
    data[size_t(d->id)] = d;

    d = new Item_data_t(Item_id::mummy_maul);
    reset_data(*d, Item_type::melee_wpn_intr);
    d->melee.att_msgs = {"", "mauls me"};
    set_dmg_from_mon_id(*d, Actor_id::mummy);
    d->melee.prop_applied = new Prop_cursed(Prop_turns::std);
    d->melee.knocks_back = true;
    data[size_t(d->id)] = d;

    d = new Item_data_t(Item_id::croc_head_mummy_spear);
    reset_data(*d, Item_type::melee_wpn_intr);
    d->melee.att_msgs = {"", "hits me with a spear"};
    set_dmg_from_mon_id(*d, Actor_id::croc_head_mummy);
    data[size_t(d->id)] = d;

    d = new Item_data_t(Item_id::deep_one_javelin_att);
    reset_data(*d, Item_type::ranged_wpn_intr);
    d->ranged.att_msgs = {"", "throws a javelin at me"};
    set_dmg_from_mon_id(*d, Actor_id::deep_one);
    d->ranged.snd_msg = "";
    d->ranged.missile_clr = clr_brown;
    d->ranged.missile_glyph = '/';
    d->ranged.snd_vol = Snd_vol::low;
    data[size_t(d->id)] = d;

    d = new Item_data_t(Item_id::deep_one_spear_att);
    reset_data(*d, Item_type::melee_wpn_intr);
    d->melee.att_msgs = {"", "hits me with a spear"};
    set_dmg_from_mon_id(*d, Actor_id::deep_one);
    data[size_t(d->id)] = d;

    d = new Item_data_t(Item_id::ape_maul);
    reset_data(*d, Item_type::melee_wpn_intr);
    d->melee.att_msgs = {"", "mauls me"};
    set_dmg_from_mon_id(*d, Actor_id::ape);
    data[size_t(d->id)] = d;

    d = new Item_data_t(Item_id::ooze_black_spew_pus);
    reset_data(*d, Item_type::melee_wpn_intr);
    d->melee.att_msgs = {"", "spews pus on me"};
    set_dmg_from_mon_id(*d, Actor_id::ooze_black);
    data[size_t(d->id)] = d;

    d = new Item_data_t(Item_id::ooze_clear_spew_pus);
    reset_data(*d, Item_type::melee_wpn_intr);
    d->melee.att_msgs = {"", "spews pus on me"};
    set_dmg_from_mon_id(*d, Actor_id::ooze_clear);
    data[size_t(d->id)] = d;

    d = new Item_data_t(Item_id::ooze_putrid_spew_pus);
    reset_data(*d, Item_type::melee_wpn_intr);
    d->melee.att_msgs = {"", "spews infected pus on me"};
    set_dmg_from_mon_id(*d, Actor_id::ooze_putrid);
    d->melee.prop_applied = new Prop_infected(Prop_turns::std);
    data[size_t(d->id)] = d;

    d = new Item_data_t(Item_id::ooze_poison_spew_pus);
    reset_data(*d, Item_type::melee_wpn_intr);
    d->melee.att_msgs = {"", "spews poisonous pus on me"};
    set_dmg_from_mon_id(*d, Actor_id::ooze_poison);
    d->melee.prop_applied = new Prop_poisoned(Prop_turns::std);
    data[size_t(d->id)] = d;

    d = new Item_data_t(Item_id::colour_oOSpace_touch);
    reset_data(*d, Item_type::melee_wpn_intr);
    d->melee.att_msgs = {"", "touches me"};
    set_dmg_from_mon_id(*d, Actor_id::colour_oOSpace);
    data[size_t(d->id)] = d;

    d = new Item_data_t(Item_id::chthonian_bite);
    reset_data(*d, Item_type::melee_wpn_intr);
    d->melee.att_msgs = {"", "strikes me with a tentacle"};
    d->melee.knocks_back = true;
    set_dmg_from_mon_id(*d, Actor_id::chthonian);
    data[size_t(d->id)] = d;

    d = new Item_data_t(Item_id::hunting_horror_bite);
    reset_data(*d, Item_type::melee_wpn_intr);
    d->melee.att_msgs = {"", "bites me"};
    set_dmg_from_mon_id(*d, Actor_id::hunting_horror);
    d->melee.prop_applied = new Prop_poisoned(Prop_turns::std);
    data[size_t(d->id)] = d;

    d = new Item_data_t(Item_id::mold_spores);
    reset_data(*d, Item_type::melee_wpn_intr);
    d->melee.att_msgs = {"", "releases spores at me"};
    set_dmg_from_mon_id(*d, Actor_id::mold);
    d->melee.prop_applied = new Prop_poisoned(Prop_turns::specific, POISON_DMG_N_TURN * 2);
    data[size_t(d->id)] = d;

    d = new Item_data_t(Item_id::mi_go_sting);
    reset_data(*d, Item_type::melee_wpn_intr);
    d->melee.att_msgs = {"", "stings me"};
    set_dmg_from_mon_id(*d, Actor_id::mi_go);
    d->melee.prop_applied = new Prop_poisoned(Prop_turns::specific, POISON_DMG_N_TURN * 2);
    data[size_t(d->id)] = d;

    d = new Item_data_t(Item_id::mi_go_commander_sting);
    reset_data(*d, Item_type::melee_wpn_intr);
    d->melee.att_msgs = {"", "stings me"};
    set_dmg_from_mon_id(*d, Actor_id::mi_go_commander);
    d->melee.prop_applied = new Prop_poisoned(Prop_turns::specific, POISON_DMG_N_TURN * 2);
    data[size_t(d->id)] = d;

    d = new Item_data_t(Item_id::the_high_priest_claw);
    reset_data(*d, Item_type::melee_wpn_intr);
    d->melee.att_msgs = {"", "claws me"};
    set_dmg_from_mon_id(*d, Actor_id::the_high_priest);
    data[size_t(d->id)] = d;

    d = new Item_data_t(Item_id::armor_leather_jacket);
    reset_data(*d, Item_type::armor);
    d->base_name = {"Leather Jacket", "", "a Leather Jacket"};
    d->base_descr =
    {
        "It offers some protection."
    };
    d->weight = Item_weight::light;
    d->clr = clr_brown;
    d->spawn_std_range.lower = 1;
    d->armor.armor_points = 1;
    d->armor.dmg_to_durability_factor = 1.0;
    d->land_on_hard_snd_msg = "";
    add_feature_found_in(*d, Feature_id::cabinet);
    data[size_t(d->id)] = d;

    d = new Item_data_t(Item_id::armor_iron_suit);
    reset_data(*d, Item_type::armor);
    d->base_name = {"Iron Suit", "", "an Iron Suit"};
    d->base_descr =
    {
        "A crude armour constructed from metal plates, bolts, and leather straps.",

        "It can absorb a high amount of damage, but it makes sneaking and dodging very "
        "difficult. Also, due to the narrow slit of the helmet, aiming is slightly "
        "more difficult, and it is harder to detect sneaking enemies and hidden objects."
    };
    d->ability_mods_while_equipped[int(Ability_id::stealth)]    = -50;
    d->ability_mods_while_equipped[int(Ability_id::dodge_att)]   = -50;
    d->ability_mods_while_equipped[int(Ability_id::dodge_trap)]  = -50;
    d->ability_mods_while_equipped[int(Ability_id::melee)]      = -10;
    d->ability_mods_while_equipped[int(Ability_id::ranged)]     = -10;
    d->ability_mods_while_equipped[int(Ability_id::searching)]  = -6;
    d->weight = Item_weight::heavy;
    d->clr = clr_white;
    d->spawn_std_range.lower = 2;
    d->armor.armor_points = 5;
    d->armor.dmg_to_durability_factor = 0.3;
    d->land_on_hard_snd_msg = "I hear a crashing sound.";
    add_feature_found_in(*d, Feature_id::cabinet);
    data[size_t(d->id)] = d;

    d = new Item_data_t(Item_id::armor_flack_jacket);
    reset_data(*d, Item_type::armor);
    d->base_name = {"Flak Jacket", "", "a Flak Jacket"};
    d->base_descr =
    {
        "An armour consisting of steel plates sewn into a waistcoat. It offers very good "
        "protection for its weight. Sneaking and dodging is slightly more difficult."
    };
    d->ability_mods_while_equipped[int(Ability_id::stealth)]    = -20;
    d->ability_mods_while_equipped[int(Ability_id::dodge_att)]   = -20;
    d->ability_mods_while_equipped[int(Ability_id::dodge_trap)]  = -20;
    d->weight = Item_weight::medium;
    d->clr = clr_green;
    d->spawn_std_range.lower = 3;
    d->armor.armor_points = 3;
    d->armor.dmg_to_durability_factor = 0.5;
    d->land_on_hard_snd_msg = "I hear a thudding sound.";
    add_feature_found_in(*d, Feature_id::cabinet);
    data[size_t(d->id)] = d;

    d = new Item_data_t(Item_id::armor_asb_suit);
    reset_data(*d, Item_type::armor);
    d->base_name = {"Asbestos Suit", "", "an Asbestos Suit"};
    d->base_descr =
    {
        "A one piece overall of asbestos fabric, including a hood, furnace mask, gloves "
        "and shoes. It protects the wearer against fire, acid and electricity, and also "
        "against smoke, fumes and gas.",

        "It is a bit bulky, so sneaking and dodging is slightly more difficult. Also, "
        "because of the hood and mask, aiming and detecting hidden enemies and objects "
        "is somewhat harder."
    };
    d->ability_mods_while_equipped[int(Ability_id::stealth)]    = -20;
    d->ability_mods_while_equipped[int(Ability_id::dodge_att)]   = -20;
    d->ability_mods_while_equipped[int(Ability_id::dodge_trap)]  = -20;
    d->ability_mods_while_equipped[int(Ability_id::melee)]      = -10;
    d->ability_mods_while_equipped[int(Ability_id::ranged)]     = -10;
    d->ability_mods_while_equipped[int(Ability_id::searching)]  = -6;
    d->weight = Item_weight::medium;
    d->clr = clr_red_lgt;
    d->spawn_std_range.lower = 3;
    d->armor.armor_points = 1;
    d->armor.dmg_to_durability_factor = 1.0;
    d->land_on_hard_snd_msg = "";
    add_feature_found_in(*d, Feature_id::cabinet);
    add_feature_found_in(*d, Feature_id::chest);
    data[size_t(d->id)] = d;

    d = new Item_data_t(Item_id::armor_heavy_coat);
    reset_data(*d, Item_type::armor);
    d->base_name = {"Heavy Coat", "", "a Heavy Coat"};
    d->base_descr =
    {
        "A very thick coat. It gives more protection against physical harm than a "
        "leather jacket, and it also protects against cold.",

        "Sneaking and dodging is slightly more difficult due to its higher weight."
    };
    d->ability_mods_while_equipped[int(Ability_id::stealth)]    = -15;
    d->ability_mods_while_equipped[int(Ability_id::dodge_att)]   = -15;
    d->ability_mods_while_equipped[int(Ability_id::dodge_trap)]  = -15;
    d->weight = Item_weight::medium;
    d->clr = clr_blue_lgt;
    d->spawn_std_range.lower = 3;
    d->armor.armor_points = 2;
    d->armor.dmg_to_durability_factor = 1.0;
    d->land_on_hard_snd_msg = "";
    add_feature_found_in(*d, Feature_id::cabinet);
    data[size_t(d->id)] = d;

    d = new Item_data_t(Item_id::armor_mi_go);
    reset_data(*d, Item_type::armor);
    d->base_name = {"Mi-go Bio-armor", "", "a Mi-go Bio-armor"};
    d->base_descr =
    {
        "A self-repairing biological armor created by the Mi-go.",

        "It is very disturbing for a human to wear."
    };
    d->spawn_std_range = Range(-1, -1);
    d->weight = Item_weight::medium;
    d->shock_while_equipped = 15;
    d->clr = clr_magenta;
    d->tile = Tile_id::mi_go_armor;
    d->armor.armor_points = 2;
    d->armor.dmg_to_durability_factor = 1.5;
    d->land_on_hard_snd_msg = "";
    data[size_t(d->id)] = d;

    d = new Item_data_t(Item_id::gas_mask);
    reset_data(*d, Item_type::head_wear);
    d->base_name = {"Gas Mask", "", "a Gas Mask"};
    d->base_descr =
    {
        "Protects the eyes, throat and lungs from smoke and fumes. It has a limited "
        "useful lifespan that is related to the absorbent capacity of the filter.",

        "Due to the small eye windows, aiming is slightly more difficult, and it is "
        "harder to detect sneaking enemies and hidden objects."
    };
    d->ability_mods_while_equipped[int(Ability_id::melee)]      = -10;
    d->ability_mods_while_equipped[int(Ability_id::ranged)]     = -10;
    d->ability_mods_while_equipped[int(Ability_id::searching)]  = -6;
    d->is_stackable = false;
    d->clr = clr_brown;
    d->tile = Tile_id::mask;
    d->glyph = '[';
    d->spawn_std_range = Range(1, DLVL_LAST_EARLY_GAME);
    d->chance_to_include_in_floor_spawn_list = 50;
    d->weight = Item_weight::light;
    d->land_on_hard_snd_msg = "";
    data[size_t(d->id)] = d;

//    d = new Item_data_t(Item_id::hideous_mask);
//    reset_data(*d, Item_type::head_wear);
//    d->base_name = {"Hideous Mask", "", "The Hideous Mask"};
//    d->base_descr =
//    {
//        "[TODO]",
//
//        item_carry_shock_descr
//    };
//    d->is_stackable = false;
//    d->clr = clr_magenta;
//    d->tile = Tile_id::mask;
//    d->glyph = '[';
//    d->spawn_std_range = Range(-1, -1);
//    d->weight = Item_weight::light;
//    d->land_on_hard_snd_msg = "";
//    d->chance_to_include_in_floor_spawn_list = 1;
//    d->value = Item_value::major_treasure;
//    d->shock_while_in_backpack = d->shock_while_equipped = 15;
//    add_feature_found_in(*d, Feature_id::tomb, 8);
//    data[size_t(d->id)] = d;

    d = new Item_data_t(Item_id::scroll_mayhem);
    reset_data(*d, Item_type::scroll);
    d->spell_cast_from_scroll = Spell_id::mayhem;
    data[size_t(d->id)] = d;

    d = new Item_data_t(Item_id::scroll_telep);
    reset_data(*d, Item_type::scroll);
    d->spell_cast_from_scroll = Spell_id::teleport;
    data[size_t(d->id)] = d;

    d = new Item_data_t(Item_id::scroll_pest);
    reset_data(*d, Item_type::scroll);
    d->spell_cast_from_scroll = Spell_id::pest;
    data[size_t(d->id)] = d;

    d = new Item_data_t(Item_id::scroll_slow_mon);
    reset_data(*d, Item_type::scroll);
    d->spell_cast_from_scroll = Spell_id::slow_mon;
    data[size_t(d->id)] = d;

    d = new Item_data_t(Item_id::scroll_terrify_mon);
    reset_data(*d, Item_type::scroll);
    d->spell_cast_from_scroll = Spell_id::terrify_mon;
    data[size_t(d->id)] = d;

    d = new Item_data_t(Item_id::scroll_paral_mon);
    reset_data(*d, Item_type::scroll);
    d->spell_cast_from_scroll = Spell_id::paralyze_mon;
    data[size_t(d->id)] = d;

    d = new Item_data_t(Item_id::scroll_det_items);
    reset_data(*d, Item_type::scroll);
    d->spell_cast_from_scroll = Spell_id::det_items;
    data[size_t(d->id)] = d;

    d = new Item_data_t(Item_id::scroll_det_traps);
    reset_data(*d, Item_type::scroll);
    d->spell_cast_from_scroll = Spell_id::det_traps;
    data[size_t(d->id)] = d;

    d = new Item_data_t(Item_id::scroll_bless);
    reset_data(*d, Item_type::scroll);
    d->spell_cast_from_scroll = Spell_id::bless;
    data[size_t(d->id)] = d;

    d = new Item_data_t(Item_id::scroll_darkbolt);
    reset_data(*d, Item_type::scroll);
    d->spell_cast_from_scroll = Spell_id::darkbolt;
    data[size_t(d->id)] = d;

    d = new Item_data_t(Item_id::scroll_aza_wrath);
    reset_data(*d, Item_type::scroll);
    d->spell_cast_from_scroll = Spell_id::aza_wrath;
    data[size_t(d->id)] = d;

    d = new Item_data_t(Item_id::scroll_opening);
    reset_data(*d, Item_type::scroll);
    d->spell_cast_from_scroll = Spell_id::opening;
    data[size_t(d->id)] = d;

    d = new Item_data_t(Item_id::scroll_sacr_life);
    reset_data(*d, Item_type::scroll);
    d->spell_cast_from_scroll = Spell_id::sacr_life;
    data[size_t(d->id)] = d;

    d = new Item_data_t(Item_id::scroll_sacr_spi);
    reset_data(*d, Item_type::scroll);
    d->spell_cast_from_scroll = Spell_id::sacr_spi;
    data[size_t(d->id)] = d;

    d = new Item_data_t(Item_id::scroll_det_mon);
    reset_data(*d, Item_type::scroll);
    d->spell_cast_from_scroll = Spell_id::det_mon;
    data[size_t(d->id)] = d;

    d = new Item_data_t(Item_id::scroll_elem_res);
    reset_data(*d, Item_type::scroll);
    d->spell_cast_from_scroll = Spell_id::elem_res;
    data[size_t(d->id)] = d;

    d = new Item_data_t(Item_id::scroll_summon_mon);
    reset_data(*d, Item_type::scroll);
    d->spell_cast_from_scroll = Spell_id::summon;
    data[size_t(d->id)] = d;

    d = new Item_data_t(Item_id::scroll_light);
    reset_data(*d, Item_type::scroll);
    d->spell_cast_from_scroll = Spell_id::light;
    data[size_t(d->id)] = d;

    d = new Item_data_t(Item_id::potion_vitality);
    reset_data(*d, Item_type::potion);
    data[size_t(d->id)] = d;

    d = new Item_data_t(Item_id::potion_spirit);
    reset_data(*d, Item_type::potion);
    data[size_t(d->id)] = d;

    d = new Item_data_t(Item_id::potion_blindness);
    reset_data(*d, Item_type::potion);
    data[size_t(d->id)] = d;

    d = new Item_data_t(Item_id::potion_frenzy);
    reset_data(*d, Item_type::potion);
    data[size_t(d->id)] = d;

    d = new Item_data_t(Item_id::potion_fortitude);
    reset_data(*d, Item_type::potion);
    data[size_t(d->id)] = d;

    d = new Item_data_t(Item_id::potion_paralyze);
    reset_data(*d, Item_type::potion);
    data[size_t(d->id)] = d;

    d = new Item_data_t(Item_id::potion_rElec);
    reset_data(*d, Item_type::potion);
    data[size_t(d->id)] = d;

    d = new Item_data_t(Item_id::potion_conf);
    reset_data(*d, Item_type::potion);
    data[size_t(d->id)] = d;

    d = new Item_data_t(Item_id::potion_poison);
    reset_data(*d, Item_type::potion);
    data[size_t(d->id)] = d;

    d = new Item_data_t(Item_id::potion_insight);
    reset_data(*d, Item_type::potion);
    data[size_t(d->id)] = d;

    d = new Item_data_t(Item_id::potion_clairv);
    reset_data(*d, Item_type::potion);
    data[size_t(d->id)] = d;

    d = new Item_data_t(Item_id::potion_rFire);
    reset_data(*d, Item_type::potion);
    data[size_t(d->id)] = d;

    d = new Item_data_t(Item_id::potion_antidote);
    reset_data(*d, Item_type::potion);
    data[size_t(d->id)] = d;

    d = new Item_data_t(Item_id::potion_descent);
    reset_data(*d, Item_type::potion);
    data[size_t(d->id)] = d;

    d = new Item_data_t(Item_id::device_blaster);
    reset_data(*d, Item_type::device);
    d->base_name = {"Blaster Device", "Blaster Devices", "a Blaster Device"};
    d->value = Item_value::minor_treasure;
    d->clr = clr_gray;
    add_feature_found_in(*d, Feature_id::chest, 10);
    add_feature_found_in(*d, Feature_id::tomb, 10);
    add_feature_found_in(*d, Feature_id::cocoon, 10);
    data[size_t(d->id)] = d;

    d = new Item_data_t(Item_id::device_shockwave);
    reset_data(*d, Item_type::device);
    d->base_name =
    {
        "Shock Wave Device", "Shock Wave Devices", "a Shock Wave Device"
    };
    d->value = Item_value::minor_treasure;
    d->clr = clr_gray;
    add_feature_found_in(*d, Feature_id::chest, 10);
    add_feature_found_in(*d, Feature_id::tomb, 10);
    add_feature_found_in(*d, Feature_id::cocoon, 10);
    data[size_t(d->id)] = d;

    d = new Item_data_t(Item_id::device_rejuvenator);
    reset_data(*d, Item_type::device);
    d->base_name =
    {
        "Rejuvenator Device", "Rejuvenator Devices", "a Rejuvenator Device"
    };
    d->value = Item_value::minor_treasure;
    d->clr = clr_gray;
    add_feature_found_in(*d, Feature_id::chest, 10);
    add_feature_found_in(*d, Feature_id::tomb, 10);
    add_feature_found_in(*d, Feature_id::cocoon, 10);
    data[size_t(d->id)] = d;

    d = new Item_data_t(Item_id::device_translocator);
    reset_data(*d, Item_type::device);
    d->base_name =
    {
        "Translocator Device", "Translocator Devices", "a Translocator Device"
    };
    d->value = Item_value::minor_treasure;
    d->clr = clr_gray;
    add_feature_found_in(*d, Feature_id::chest, 10);
    add_feature_found_in(*d, Feature_id::tomb, 10);
    add_feature_found_in(*d, Feature_id::cocoon, 10);
    data[size_t(d->id)] = d;

    d = new Item_data_t(Item_id::device_sentry_drone);
    reset_data(*d, Item_type::device);
    d->base_name =
    {
        "Sentry Drone Device", "Sentry Drone Devices", "a Sentry Drone Device"
    };
    d->value = Item_value::minor_treasure;
    d->clr = clr_gray;
    add_feature_found_in(*d, Feature_id::chest, 10);
    add_feature_found_in(*d, Feature_id::tomb, 10);
    add_feature_found_in(*d, Feature_id::cocoon, 10);
    data[size_t(d->id)] = d;

    d = new Item_data_t(Item_id::electric_lantern);
    reset_data(*d, Item_type::device);
    d->base_name = {"Electric Lantern", "Electric Lanterns", "an Electric Lantern"};
    d->base_descr =
    {
        "A portable light source. It is somewhat unreliable as it tends to flicker and "
        "malfunction often."
    };
    d->spawn_std_range = Range(1, DLVL_LAST_MID_GAME);
    d->spawn_std_range = Range(1, 10);
    d->chance_to_include_in_floor_spawn_list = 50;
    d->is_identified = true;
    d->tile = Tile_id::electric_lantern;
    d->clr = clr_yellow;
    add_feature_found_in(*d, Feature_id::chest);
    add_feature_found_in(*d, Feature_id::cabinet);
    add_feature_found_in(*d, Feature_id::cocoon);
    data[size_t(d->id)] = d;

    d = new Item_data_t(Item_id::medical_bag);
    reset_data(*d, Item_type::general);
    d->base_name = {"Medical Bag", "Medical Bags", "a Medical Bag"};
    d->base_descr =
    {
        "A portable bag of medical supplies."
    };
    d->weight = Item_weight::medium;
    d->spawn_std_range = Range(1, DLVL_LAST_MID_GAME);
    d->is_stackable = false;
    d->glyph = '~';
    d->clr = clr_brown_drk;
    d->tile = Tile_id::medical_bag;
    data[size_t(d->id)] = d;

    d = new Item_data_t(Item_id::star_amulet);
    reset_data(*d, Item_type::amulet);
    d->base_name = {"Star Amulet", "", "a Star Amulet"};
    d->base_name_un_id = d->base_name;
    d->clr = clr_white;
    data[size_t(d->id)] = d;

    d = new Item_data_t(Item_id::skull_amulet);
    reset_data(*d, Item_type::amulet);
    d->base_name = {"Skull Amulet", "", "a Skull Amulet"};
    d->base_name_un_id = d->base_name;
    d->clr = clr_white;
    data[size_t(d->id)] = d;

    d = new Item_data_t(Item_id::spider_amulet);
    reset_data(*d, Item_type::amulet);
    d->base_name = {"Spider Amulet", "", "a Spider Amulet"};
    d->base_name_un_id = d->base_name;
    d->clr = clr_white;
    data[size_t(d->id)] = d;

    d = new Item_data_t(Item_id::eye_amulet);
    reset_data(*d, Item_type::amulet);
    d->base_name = {"Eye Amulet", "", "an Eye Amulet"};
    d->base_name_un_id = d->base_name;
    d->clr = clr_white;
    data[size_t(d->id)] = d;

    d = new Item_data_t(Item_id::moon_amulet);
    reset_data(*d, Item_type::amulet);
    d->base_name = {"Moon Amulet", "", "a Moon Amulet"};
    d->base_name_un_id = d->base_name;
    d->clr = clr_white;
    data[size_t(d->id)] = d;

    d = new Item_data_t(Item_id::bat_amulet);
    reset_data(*d, Item_type::amulet);
    d->base_name = {"Bat Amulet", "", "a Bat Amulet"};
    d->base_name_un_id = d->base_name;
    d->clr = clr_white;
    data[size_t(d->id)] = d;

    d = new Item_data_t(Item_id::scarab_amulet);
    reset_data(*d, Item_type::amulet);
    d->base_name = {"Scarab Amulet", "", "a Scarab Amulet"};
    d->base_name_un_id = d->base_name;
    d->clr = clr_white;
    data[size_t(d->id)] = d;

    d = new Item_data_t(Item_id::dagger_amulet);
    reset_data(*d, Item_type::amulet);
    d->base_name = {"Dagger Amulet", "", "a Dagger Amulet"};
    d->base_name_un_id = d->base_name;
    d->clr = clr_white;
    data[size_t(d->id)] = d;

    d = new Item_data_t(Item_id::golden_ring);
    reset_data(*d, Item_type::ring);
    d->base_name = {"Golden Ring", "", "a Golden Ring"};
    d->base_name_un_id = d->base_name;
    d->clr = clr_yellow;
    data[size_t(d->id)] = d;

    d = new Item_data_t(Item_id::silver_ring);
    reset_data(*d, Item_type::ring);
    d->base_name = {"Silver Ring", "", "a Silver Ring"};
    d->base_name_un_id = d->base_name;
    d->clr = clr_white;
    data[size_t(d->id)] = d;

    d = new Item_data_t(Item_id::carnelian_ring);
    reset_data(*d, Item_type::ring);
    d->base_name = {"Carnelian Ring", "", "a Carnelian Ring"};
    d->base_name_un_id = d->base_name;
    d->clr = clr_red_lgt;
    data[size_t(d->id)] = d;

    d = new Item_data_t(Item_id::garnet_ring);
    reset_data(*d, Item_type::ring);
    d->base_name = {"Garnet Ring", "", "a Garnet Ring"};
    d->base_name_un_id = d->base_name;
    d->clr = clr_red_lgt;
    data[size_t(d->id)] = d;

    d = new Item_data_t(Item_id::iron_ring);
    reset_data(*d, Item_type::ring);
    d->base_name = {"Iron Ring", "", "an Iron Ring"};
    d->base_name_un_id = d->base_name;
    d->clr = clr_gray;
    data[size_t(d->id)] = d;

    d = new Item_data_t(Item_id::jade_ring);
    reset_data(*d, Item_type::ring);
    d->base_name = {"Jade Ring", "", "a Jade Ring"};
    d->base_name_un_id = d->base_name;
    d->clr = clr_green_lgt;
    data[size_t(d->id)] = d;

    d = new Item_data_t(Item_id::moonstone_ring);
    reset_data(*d, Item_type::ring);
    d->base_name = {"Moonstone Ring", "", "a Moonstone Ring"};
    d->base_name_un_id = d->base_name;
    d->clr = clr_blue_lgt;
    data[size_t(d->id)] = d;

    d = new Item_data_t(Item_id::obsidian_ring);
    reset_data(*d, Item_type::ring);
    d->base_name = {"Obsidian Ring", "", "an Obsidian Ring"};
    d->base_name_un_id = d->base_name;
    d->clr = clr_gray;
    data[size_t(d->id)] = d;

    d = new Item_data_t(Item_id::onyx_ring);
    reset_data(*d, Item_type::ring);
    d->base_name = {"Onyx Ring", "", "an Onyx Ring"};
    d->base_name_un_id = d->base_name;
    d->clr = clr_gray;
    data[size_t(d->id)] = d;

    d = new Item_data_t(Item_id::topaz_ring);
    reset_data(*d, Item_type::ring);
    d->base_name = {"Topaz Ring", "", "a Topaz Ring"};
    d->base_name_un_id = d->base_name;
    d->clr = clr_blue_lgt;
    data[size_t(d->id)] = d;

    d = new Item_data_t(Item_id::emerald_ring);
    reset_data(*d, Item_type::ring);
    d->base_name = {"Emerald Ring", "", "an Emerald Ring"};
    d->base_name_un_id = d->base_name;
    d->clr = clr_green_lgt;
    data[size_t(d->id)] = d;
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
    for (size_t i = 0; i < int(Item_id::END); ++i) {delete data[i];}
    TRACE_FUNC_END;
}


void store_to_save_lines(vector<string>& lines)
{
    for (int i = 0; i < int(Item_id::END); ++i)
    {
        lines.push_back(data[i]->is_identified ? "1" : "0");
        lines.push_back(data[i]->allow_spawn   ? "1" : "0");

        if (
            data[i]->type == Item_type::scroll ||
            data[i]->type == Item_type::potion)
        {
            lines.push_back(data[i]->is_tried ? "1" : "0");
        }
    }
}

void setup_from_save_lines(vector<string>& lines)
{
    for (int i = 0; i < int(Item_id::END); ++i)
    {
        data[i]->is_identified = lines.front() == "0" ? false : true;
        lines.erase(begin(lines));

        data[i]->allow_spawn = lines.front()   == "0" ? false : true;
        lines.erase(begin(lines));

        if (
            data[i]->type == Item_type::scroll ||
            data[i]->type == Item_type::potion)
        {
            data[i]->is_tried = lines.front() == "0" ? false : true;
            lines.erase(begin(lines));
        }
    }
}

} //Item_data
