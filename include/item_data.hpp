#ifndef DATA_HPP
#define DATA_HPP

#include <vector>
#include <string>

#include "ability_values.hpp"
#include "spells.hpp"
#include "feature_data.hpp"
#include "audio.hpp"
#include "room.hpp"

enum class SndVol;

enum ItemWeight
{
    none        = 0,
    extra_light = 1,    // E.g. ammo
    light       = 10,   // E.g. dynamite, daggers
    medium      = 50,   // E.g. most firearms
    heavy       = 100,  // E.g. heavy armor, heavy weapons
};

enum class ItemValue
{
    normal,
    minor_treasure,
    major_treasure
};

enum class ItemId
{
    trapez,

    thr_knife,
    rock,
    iron_spike,
    dagger,
    hatchet,
    club,
    hammer,
    machete,
    axe,
    spiked_mace,
    pitch_fork,
    sledge_hammer,

    sawed_off,
    pump_shotgun,
    machine_gun,
    incinerator,
    spike_gun,
    shotgun_shell,
    drum_of_bullets,
    incinerator_ammo,
    pistol,
    pistol_mag,
    flare_gun,

    trap_dart,
    trap_dart_poison,
    trap_spear,
    trap_spear_poison,

    dynamite,
    flare,
    molotov,
    smoke_grenade,

    player_kick,
    player_stomp,
    player_punch,
    player_ghoul_claw,

    rat_bite,
    rat_thing_bite,
    brown_jenkin_bite,
    worm_mass_bite,
    mind_worms_bite,
    green_spider_bite,
    white_spider_bite,
    red_spider_bite,
    shadow_spider_bite,
    leng_spider_bite, leng_spider_bola,
    pit_viper_bite,
    spitting_cobra_bite,
    spitting_cobra_spit,
    black_mamba_bite,
    ghoul_claw,
    void_traveler_rip, elder_void_traveler_rip,
    shadow_claw,
    invis_stalker_claw,
    byakhee_claw,
    giant_mantis_claw,
    giant_locust_bite,
    fire_hound_breath, fire_hound_bite,
    energy_hound_breath, energy_hound_bite,
    zuul_bite,
    raven_peck,
    giant_bat_bite, vampire_bat_bite, abaxu_bite,
    zombie_axe, zombie_claw,
    bloated_zombie_punch, bloated_zombie_spit,
    crawling_intestines_strangle,
    crawling_hand_strangle, thing_strangle,
    floating_skull_bite,
    wolf_bite,
    ghost_claw,
    phantasm_sickle,
    wraith_claw,
    mi_go_gun,
    polyp_tentacle,
    greater_polyp_tentacle,
    mind_leech_sting, life_leech_sting, spirit_leech_sting,
    mummy_maul,
    croc_head_mummy_spear,
    deep_one_spear_att,
    deep_one_javelin_att,
    ape_maul,
    ooze_black_spew_pus,
    ooze_putrid_spew_pus,
    ooze_poison_spew_pus,
    ooze_clear_spew_pus,
    strange_color_touch,
    chthonian_bite,
    death_fiend_claw,
    hunting_horror_bite,
    dust_vortex_engulf, fire_vortex_engulf, energy_vortex_engulf,
    mold_spores,
    mi_go_sting, mi_go_commander_sting,
    the_high_priest_claw,
    high_priest_guard_ghoul_claw,

    armor_leather_jacket,
    armor_iron_suit,
    armor_flak_jacket,
    armor_asb_suit,
    armor_mi_go,

    gas_mask,

    scroll_darkbolt,
    scroll_enfeeble,
    scroll_telep,
    scroll_summon_mon,
    scroll_pest,
    scroll_see_invis,
    scroll_heal,
    scroll_spell_shield,
    scroll_aza_wrath,
    scroll_mayhem,
    scroll_searching,
    scroll_opening,
    scroll_res,
    scroll_light,
    scroll_bless,
    scroll_anim_wpns,
    scroll_transmut,

    potion_vitality,
    potion_spirit,
    potion_blindness,
    potion_fortitude,
    potion_paralyze,
    potion_r_elec,
    potion_conf,
    potion_poison,
    potion_insight,
    potion_r_fire,
    potion_curing,
    potion_descent,
    potion_invis,

    device_blaster,
    device_shockwave,
    device_rejuvenator,
    device_purge_invis,
    device_translocator,
    device_sentry_drone,

    rod_curing,
    rod_opening,
    rod_bless,
    rod_cloud_minds,

    lantern,

    medical_bag,

    pharaoh_staff,
    refl_talisman,
    resurrect_talisman,
    tele_ctrl_talisman,
    horn_of_malice,
    horn_of_deafening,
    horn_of_banishment,
    clockwork,
    spirit_dagger,
    orb_of_life,

    zombie_dust,

    END
};

struct ItemContainerSpawnRule
{
    ItemContainerSpawnRule() :
        feature_id          (FeatureId::END),
        pct_chance_to_incl  (0) {}

    ItemContainerSpawnRule(FeatureId feature_id_, int pct_chance_to_incl_) :
        feature_id          (feature_id_),
        pct_chance_to_incl  (pct_chance_to_incl_) {}

    FeatureId feature_id;
    int pct_chance_to_incl;
};

struct ItemAttProp
{
    ItemAttProp() :
        prop        (nullptr),
        pct_chance  (100) {}

    ItemAttProp(Prop* const prop, int pct_chance = 100) :
        prop        (prop),
        pct_chance  (pct_chance) {}

    Prop* prop;

    int pct_chance;
};

class ItemDataT
{
public:
    ItemDataT();

    ~ItemDataT() {}

    ItemId                      id;
    ItemType                    type;
    bool                        has_std_activate; // E.g. potions and scrolls
    bool                        is_prio_in_backpack_list; // E.g. Medical Bag
    ItemValue                   value;
    int                         weight;
    bool                        is_unique;
    bool                        allow_spawn;
    Range                       spawn_std_range;
    int                         max_stack_at_spawn;
    int                         chance_to_incl_in_spawn_list;
    bool                        is_stackable;
    bool                        is_identified;
    bool                        is_tried;
    bool                        is_found; // Was seen on map or in inventory
    int                         xp_on_found;
    ItemName                    base_name;
    ItemName                    base_name_un_id;
    std::vector<std::string>    base_descr;
    char                        glyph;
    Clr                         clr;
    TileId                      tile;
    AttMode                     main_att_mode;
    SpellId                     spell_cast_from_scroll;
    std::string                 land_on_hard_snd_msg;
    SfxId                       land_on_hard_sfx;
    bool                        is_carry_shocking;
    bool                        is_equiped_shocking;

    std::vector<RoomType>       native_rooms;
    std::vector<FeatureId>      native_containers;

    int ability_mods_while_equipped[(size_t)AbilityId::END];

    bool                        allow_display_dmg;

    struct ItemMeleeData
    {
        ItemMeleeData();

        ~ItemMeleeData();

        bool                    is_melee_wpn;
        //
        // NOTE: The "plus" field is ignored in the melee damage data, since
        //       melee weapons have individual plus damages stored in the weapon
        //       objects.
        //
        Dice                    dmg;
        int                     hit_chance_mod;
        bool                    is_noisy;
        ItemAttMsgs             att_msgs;
        ItemAttProp             prop_applied;
        DmgType                 dmg_type;
        DmgMethod               dmg_method;
        bool                    knocks_back;
        bool                    att_corpse;
        bool                    att_rigid;
        SfxId                   hit_small_sfx;
        SfxId                   hit_medium_sfx;
        SfxId                   hit_hard_sfx;
        SfxId                   miss_sfx;
    } melee;

    struct ItemRangedData
    {
        ItemRangedData();

        ~ItemRangedData();

        bool                    is_ranged_wpn;
        bool                    is_throwable_wpn;
        bool                    is_machine_gun;
        bool                    is_shotgun;
        //
        // NOTE: This should be set on ranged weapons (using ammo) AND mags:
        //
        int                     max_ammo;
        Dice                    dmg;
        //
        // NOTE: "Pure" melee weapons do not have to (should not) set this value
        //       they do throw damage based on their melee damage instead.
        //
        Dice                    throw_dmg;
        int                     hit_chance_mod;
        int                     throw_hit_chance_mod;
        bool                    always_break_on_throw;
        int                     effective_range;
        int                     max_range;
        bool                    knocks_back;
        ItemId                  ammo_item_id;
        DmgType                 dmg_type;
        bool                    has_infinite_ammo;
        char                    projectile_glyph;
        TileId                  projectile_tile;
        Clr                     projectile_clr;
        bool                    projectile_leaves_trail;
        bool                    projectile_leaves_smoke;
        ItemAttMsgs             att_msgs;
        std::string             snd_msg;
        SndVol                  snd_vol;
        bool                    makes_ricochet_snd;
        SfxId                   att_sfx;
        SfxId                   reload_sfx;
        ItemAttProp             prop_applied;
    } ranged;

    struct ItemArmorData
    {
        ItemArmorData();

        int                     armor_points;
        double                  dmg_to_durability_factor;
    } armor;
};

class Item;

namespace item_data
{

extern ItemDataT data[(size_t)ItemId::END];

void init();
void cleanup();

void save();
void load();

} // item_data

#endif
