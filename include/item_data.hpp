#ifndef DATA_HPP
#define DATA_HPP

#include <vector>
#include <string>

#include "ability_values.hpp"
#include "spells.hpp"
#include "feature_data.hpp"
#include "audio.hpp"
#include "room.hpp"
#include "item_att_property.hpp"

enum class SndVol;

enum class ItemId
{
        trapez,

        // Melee weapons and thrown weapons
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
        thr_knife,
        zombie_dust,

        // Ranged weapons, ammo
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
        mi_go_gun,

        // Trap weapons
        trap_dart,
        trap_dart_poison,
        trap_spear,
        trap_spear_poison,

        // Explosives
        dynamite,
        flare,
        molotov,
        smoke_grenade,

        // Player attacks
        player_kick,
        player_stomp,
        player_punch,
        player_ghoul_claw,

        // Intrinsic attacks for monsters
        // NOTE: There is a string -> id map below for these entries
        intr_bite,
        intr_claw,
        intr_punch,
        intr_zombie_axe,
        intr_acid_spit,
        intr_snake_venom_spit,
        intr_fire_breath,
        intr_energy_breath,
        intr_raven_peck,
        intr_vampiric_bite,
        intr_strangle,
        intr_ghost_claw,
        intr_ghost_sickle,
        intr_sting,
        intr_mind_leech_sting,
        intr_life_leech_sting,
        intr_spirit_leech_sting,
        intr_spear_thrust, // TODO: Should be a real item
        intr_javelin_thrust, // TODO: Should be a real item
        intr_javelin_throw, // TODO: Should be a real item
        intr_maul,
        intr_pus_spew,
        intr_touch,
        intr_acid_touch,
        intr_dust_engulf,
        intr_fire_engulf,
        intr_energy_engulf,
        intr_spores,
        intr_web_bola,

        // Armor
        armor_leather_jacket,
        armor_iron_suit,
        armor_flak_jacket,
        armor_asb_suit,
        armor_mi_go,

        gas_mask,

        // Scrolls
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

        // Potions
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

        // Strange Devices
        device_blaster,
        device_rejuvenator,
        device_translocator,
        device_sentry_drone,
        device_deafening,

        lantern,

        // Rods
        rod_curing,
        rod_opening,
        rod_bless,
        rod_cloud_minds,
        rod_shockwave,

        // Medical bag
        medical_bag,

        // Artifacts
        pharaoh_staff,
        refl_talisman,
        resurrect_talisman,
        tele_ctrl_talisman,
        horn_of_malice,
        horn_of_banishment,
        clockwork,
        spirit_dagger,
        orb_of_life,

        END
};

const std::unordered_map<std::string, ItemId> str_to_intr_item_id_map =
{
        {"bite", ItemId::intr_bite},
        {"claw", ItemId::intr_claw},
        {"punch", ItemId::intr_punch},
        {"zombie_axe", ItemId::intr_zombie_axe},
        {"acid_spit", ItemId::intr_acid_spit},
        {"snake_venom_spit", ItemId::intr_snake_venom_spit},
        {"fire_breath", ItemId::intr_fire_breath},
        {"energy_breath", ItemId::intr_energy_breath},
        {"raven_peck", ItemId::intr_raven_peck},
        {"vampiric_bite", ItemId::intr_vampiric_bite},
        {"strangle", ItemId::intr_strangle},
        {"ghost_claw", ItemId::intr_ghost_claw},
        {"ghost_sickle", ItemId::intr_ghost_sickle},
        {"sting", ItemId::intr_sting},
        {"mind_leech_sting", ItemId::intr_mind_leech_sting},
        {"life_leech_sting", ItemId::intr_life_leech_sting},
        {"spirit_leech_sting", ItemId::intr_spirit_leech_sting},
        {"spear_thrust", ItemId::intr_spear_thrust},
        {"javelin_thrust", ItemId::intr_javelin_thrust},
        {"javelin_throw", ItemId::intr_javelin_throw},
        {"maul", ItemId::intr_maul},
        {"pus_spew", ItemId::intr_pus_spew},
        {"touch", ItemId::intr_touch},
        {"acid_touch", ItemId::intr_acid_touch},
        {"dust_engulf", ItemId::intr_dust_engulf},
        {"fire_engulf", ItemId::intr_fire_engulf},
        {"energy_engulf", ItemId::intr_energy_engulf},
        {"spores", ItemId::intr_spores},
        {"web_bola", ItemId::intr_web_bola},
};

enum class ItemValue
{
        normal,
        minor_treasure,
        rare_treasure,
        supreme_treasure
};

enum class ItemSetId
{
        minor_treasure,
        rare_treasure,
        supreme_treasure,
        firearm,
        spike_gun,
        priest_dagger,
        mi_go_gun,
        mi_go_armor,
        high_priest_guard_war_vet,
        high_priest_guard_rogue
};

const std::unordered_map<std::string, ItemSetId> str_to_item_set_id_map =
{
        {"minor_treasure", ItemSetId::minor_treasure},
        {"rare_treasure", ItemSetId::rare_treasure},
        {"supreme_treasure", ItemSetId::supreme_treasure},
        {"firearm", ItemSetId::firearm},
        {"spike_gun", ItemSetId::spike_gun},
        {"priest_dagger", ItemSetId::priest_dagger},
        {"mi_go_gun", ItemSetId::mi_go_gun},
        {"mi_go_armor", ItemSetId::mi_go_armor},
        {"high_priest_guard_war_vet", ItemSetId::high_priest_guard_war_vet},
        {"high_priest_guard_rogue", ItemSetId::high_priest_guard_rogue}
};

enum ItemWeight
{
        none            = 0,
        extra_light     = 1,    // E.g. ammo
        light           = 10,   // E.g. dynamite, daggers
        medium          = 50,   // E.g. most firearms
        heavy           = 100,  // E.g. heavy armor, heavy weapons
};

struct ItemContainerSpawnRule
{
        ItemContainerSpawnRule(FeatureId feature_id_, int pct_chance_to_incl_) :
                feature_id(feature_id_),
                pct_chance_to_incl(pct_chance_to_incl_) {}

        FeatureId feature_id = FeatureId::END;
        int pct_chance_to_incl = 0;
};

class ItemData
{
public:
        ItemData();

        ~ItemData() {}

        ItemId id;
        ItemType type;
        bool is_intr;
        bool has_std_activate; // E.g. potions and scrolls
        bool is_prio_in_backpack_list; // E.g. Medical Bag
        ItemValue value;
        int weight;
        bool is_unique;
        bool allow_spawn;
        Range spawn_std_range;
        int max_stack_at_spawn;
        int chance_to_incl_in_spawn_list;
        bool is_stackable;
        bool is_identified;
        bool is_alignment_known; // Used for Potions
        bool is_tried;
        bool is_found; // Was seen on map or in inventory
        int xp_on_found;
        ItemName base_name;
        ItemName base_name_un_id;
        std::vector<std::string> base_descr;
        char character;
        Color color;
        TileId tile;
        AttMode main_att_mode;
        SpellId spell_cast_from_scroll;
        std::string land_on_hard_snd_msg;
        SfxId land_on_hard_sfx;
        bool is_carry_shocking;
        bool is_equiped_shocking;

        std::vector<RoomType> native_rooms;
        std::vector<FeatureId> native_containers;

        int ability_mods_while_equipped[(size_t)AbilityId::END];

        bool allow_display_dmg;

        struct MeleeData
        {
                MeleeData();

                ~MeleeData();

                bool is_melee_wpn;
                // NOTE: The "plus" field is ignored in the melee damage data,
                // melee weapons have individual plus damages per class instance
                Dice dmg;
                int hit_chance_mod;
                bool is_noisy;
                ItemAttMsgs att_msgs;
                ItemAttProp prop_applied;
                DmgType dmg_type;
                DmgMethod dmg_method;
                bool knocks_back;
                bool att_corpse;
                bool att_rigid;
                SfxId hit_small_sfx;
                SfxId hit_medium_sfx;
                SfxId hit_hard_sfx;
                SfxId miss_sfx;
        } melee;

        struct RangedData
        {
                RangedData();

                ~RangedData();

                bool is_ranged_wpn;
                bool is_throwable_wpn;
                bool is_machine_gun;
                bool is_shotgun;
                // NOTE: This should be set on ranged weapons AND magazines
                int max_ammo;
                Dice dmg;
                // NOTE: "Pure" melee weapons should not set this value - they
                // do throw damage based on their melee damage instead
                Dice throw_dmg;
                int hit_chance_mod;
                int throw_hit_chance_mod;
                bool always_break_on_throw;
                int effective_range;
                int max_range;
                bool knocks_back;
                ItemId ammo_item_id;
                DmgType dmg_type;
                bool has_infinite_ammo;
                char projectile_character;
                TileId projectile_tile;
                Color projectile_color;
                bool projectile_leaves_trail;
                ItemAttMsgs att_msgs;
                std::string snd_msg;
                SndVol snd_vol;
                bool makes_ricochet_snd;
                SfxId att_sfx;
                SfxId reload_sfx;
                ItemAttProp prop_applied;
        } ranged;

        struct ArmorData
        {
                ArmorData();

                int armor_points;
                double dmg_to_durability_factor;
        } armor;
};

class Item;

namespace item_data
{

extern ItemData data[(size_t)ItemId::END];

void init();
void cleanup();

void save();
void load();

} // item_data

#endif
