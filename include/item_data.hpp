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
    pitch_fork,
    sledge_hammer,

    pharaoh_staff,

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
    rat_bite_diseased,
    rat_thing_bite,
    brown_jenkin_bite,
    worm_mass_bite,
    mind_worms_bite,
    green_spider_bite,
    white_spider_bite,
    red_spider_bite,
    shadow_spider_bite,
    leng_spider_bite,
    pit_viper_bite,
    spitting_cobra_bite,
    spitting_cobra_spit,
    black_mamba_bite,
    ghoul_claw,
    void_traveler_rip,
    shadow_claw,
    invis_stalker_claw,
    byakhee_claw,
    giant_mantis_claw,
    giant_locust_bite,
    fire_hound_breath, fire_hound_bite, zuul_bite,
    raven_peck,
    giant_bat_bite,
    zombie_axe, zombie_claw, zombie_claw_diseased,
    bloated_zombie_punch, bloated_zombie_spit,
    crawling_intestines_strangle,
    crawling_hand_strangle, thing_strangle,
    floating_skull_bite,
    wolf_bite,
    ghost_claw,
    phantasm_sickle,
    wraith_claw,
    mi_go_gun,
    mi_go_gun_ammo,
    polyp_tentacle,
    greater_polyp_tentacle,
    mummy_maul,
    croc_head_mummy_spear,
    deep_one_spear_att,
    deep_one_javelin_att,
    ape_maul,
    ooze_black_spew_pus,
    ooze_putrid_spew_pus,
    ooze_poison_spew_pus,
    ooze_clear_spew_pus,
    color_oo_space_touch,
    chthonian_bite,
    death_fiend_claw,
    hunting_horror_bite,
    dust_vortex_engulf, fire_vortex_engulf,
    mold_spores,
    mi_go_sting, mi_go_commander_sting,
    the_high_priest_claw,

    armor_leather_jacket,
    armor_iron_suit,
    armor_flack_jacket,
    armor_asb_suit,
    armor_mi_go,

    gas_mask,

    scroll_darkbolt,
    scroll_pest,
    scroll_telep,
    scroll_enfeeble_mon,
    scroll_det_traps,
    scroll_det_items,
    scroll_det_mon,
    scroll_bless,
    scroll_mayhem,
    scroll_aza_wrath,
    scroll_opening,
    scroll_sacr_life,
    scroll_sacr_spi,
    scroll_res,
    scroll_summon_mon,
    scroll_light,
    scroll_anim_wpns,

    potion_vitality,
    potion_spirit,
    potion_blindness,
    potion_fortitude,
    potion_paralyze,
    potion_r_elec,
    potion_conf,
    potion_poison,
    potion_insight,
    potion_clairv,
    potion_r_fire,
    potion_curing,
    potion_descent,
    potion_invis,
    potion_see_invis,

    device_blaster,
    device_shockwave,
    device_rejuvenator,
    device_translocator,
    device_sentry_drone,

    rod_purge_invis,
    rod_curing,
    rod_opening,
    rod_bless,
    rod_cloud_minds,

    lantern,

    medical_bag,

    star_amulet,
    skull_amulet,
    spider_amulet,
    eye_amulet,
    moon_amulet,
    scarab_amulet,
    dagger_amulet,
    bat_winged_amulet,
    golden_amulet,
    silver_amulet,
    obsidian_amulet,
    jade_amulet,
    rune_amulet,

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

class ItemDataT
{
public:
    ItemDataT();

    ~ItemDataT() {}

    ItemId                      id;
    ItemType                    type;
    bool                        has_std_activate; //E.g. potions and scrolls
    bool                        is_prio_in_backpack_list; //E.g. Medical Bag
    ItemValue                   value;
    ItemWeight                  weight;
    bool                        allow_spawn;
    Range                       spawn_std_range;
    int                         max_stack_at_spawn;
    int                         chance_to_incl_in_spawn_list;
    bool                        is_stackable;
    bool                        is_identified;
    bool                        is_tried;
    int                         xp_on_identify;
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
    bool                        is_ins_raied_while_carried;
    bool                        is_ins_raied_while_equiped;

    std::vector<RoomType>       native_rooms;
    std::vector<FeatureId>      native_containers;

    int ability_mods_while_equipped[(size_t)AbilityId::END];

    struct ItemMeleeData
    {
        ItemMeleeData();
        ~ItemMeleeData();

        bool                    is_melee_wpn;
        // NOTE: The "plus" field is ignored in the melee damage data, since
        //       melee weapons have individual plus damages stored in the weapon
        //       objects.
        DiceParam               dmg;
        int                     hit_chance_mod;
        bool                    is_noisy;
        ItemAttMsgs             att_msgs;
        Prop*                   prop_applied;
        DmgType                 dmg_type;
        bool                    knocks_back;
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
        // NOTE: This should be set on ranged weapons (using ammo) AND mags:
        int                     max_ammo;
        DiceParam               dmg;
        // NOTE: "Pure" melee weapons do not have to (should not) set this value
        //       they do throw damage based on their melee damage instead.
        DiceParam               throw_dmg;
        int                     hit_chance_mod;
        int                     throw_hit_chance_mod;
        int                     effective_range;
        bool                    knocks_back;
        std::string             dmg_info_override;
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
        Prop*                   prop_applied;
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

extern ItemDataT data[size_t(ItemId::END)];

void init();
void cleanup();

void save();
void load();

} //ItemData

#endif
