#ifndef ACTOR_DATA_HPP
#define ACTOR_DATA_HPP

#include <string>
#include <vector>
#include <unordered_map>

#include "ability_values.hpp"
#include "global.hpp"
#include "property_data.hpp"
#include "audio.hpp"
#include "room.hpp"
#include "spells.hpp"
#include "audio.hpp"

enum class ActorId
{
    player,
    zombie,
    zombie_axe,
    bloated_zombie,
    major_clapham_lee,
    dean_halsey,
    crawling_intestines,
    crawling_hand,
    thing,
    floating_skull,
    cultist,
    bog_tcher,
    keziah_mason,
    brown_jenkin,
    leng_elder,
    cultist_priest,
    cultist_wizard,
    cultist_arch_wizard,
    green_spider,
    white_spider,
    red_spider,
    shadow_spider,
    leng_spider,
    rat,
    rat_thing,
    pit_viper,
    spitting_cobra,
    black_mamba,
    fire_hound,
    energy_hound,
    zuul,
    ghost,
    phantasm,
    wraith,
    mi_go,
    mi_go_commander,
    flying_polyp,
    greater_polyp,
    mind_leech,
    spirit_leech,
    life_leech,
    ghoul,
    shadow,
    invis_stalker,
    wolf,
    void_traveler,
    elder_void_traveler,
    raven,
    giant_bat,
    vampire_bat,
    abaxu, // Unique bat
    byakhee,
    giant_mantis,
    locust,
    mummy,
    croc_head_mummy,
    khephren,
    nitokris,
    deep_one,
    ape,
    worm_mass,
    mind_worms,
    dust_vortex,
    fire_vortex,
    energy_vortex,
    ooze_black,
    ooze_clear,
    ooze_putrid,
    ooze_poison,
    strange_color,
    chthonian,
    hunting_horror,
    sentry_drone,
    animated_wpn,
    mold,
    gas_spore,
    death_fiend,
    the_high_priest,
    high_priest_guard_war_vet,
    high_priest_guard_rogue,
    high_priest_guard_ghoul,

    END
};

enum class MonGroupSize
{
    alone,
    few,
    pack,
    swarm
};

const std::unordered_map<std::string, MonGroupSize> str_to_group_size_map = {
    {"alone", MonGroupSize::alone},
    {"few", MonGroupSize::few},
    {"pack", MonGroupSize::pack},
    {"swarm", MonGroupSize::swarm}
};

const std::unordered_map<MonGroupSize, std::string> group_size_to_str_map = {
    {MonGroupSize::alone, "alone"},
    {MonGroupSize::few, "few"},
    {MonGroupSize::pack, "pack"},
    {MonGroupSize::swarm, "swarm"}
};

// Each actor data entry has a list of this struct, this is used for choosing
// group sizes when spawning monsters. The size of the group spawned is
// determined by a weighted random choice (so that a certain monster could for
// example usually spawn alone, but on some rare occasions spawn in big groups).
struct MonGroupSpawnRule
{
    MonGroupSpawnRule() :
        group_size  (MonGroupSize::alone),
        weight      (1) {}

    MonGroupSpawnRule(MonGroupSize group_size, int weight) :
        group_size  (group_size),
        weight      (weight) {}

    MonGroupSize group_size;
    int weight;
};

enum class ActorSpeed
{
    sluggish    = 30,
    slow        = 80,
    normal      = 100,
    fast        = 150,
    fastest     = 300,
    END
};

enum class ActorSize
{
    undefined,
    floor,
    humanoid,
    giant
};

const std::unordered_map<std::string, ActorSize> str_to_actor_size_map = {
    {"undefined", ActorSize::undefined},
    {"floor", ActorSize::floor},
    {"humanoid", ActorSize::humanoid},
    {"giant", ActorSize::giant}
};

const std::unordered_map<ActorSize, std::string> actor_size_to_str_map = {
    {ActorSize::undefined, "undefined"},
    {ActorSize::floor, "floor"},
    {ActorSize::humanoid, "humanoid"},
    {ActorSize::giant, "giant"}
};

enum class AiId
{
    looks,
    avoids_blocking_friend,
    attacks,
    paths_to_target_when_aware,
    moves_to_target_when_los,
    moves_to_lair,
    moves_to_leader,
    moves_randomly_when_unaware,
    END
};

const std::unordered_map<std::string, AiId> str_to_ai_id_map = {
    {"looks", AiId::looks},
    {"avoids_blocking_friend", AiId::avoids_blocking_friend},
    {"attacks", AiId::attacks},
    {"paths_to_target_when_aware", AiId::paths_to_target_when_aware},
    {"moves_to_target_when_los", AiId::moves_to_target_when_los},
    {"moves_to_lair", AiId::moves_to_lair},
    {"moves_to_leader", AiId::moves_to_leader},
    {"moves_randomly_when_unaware", AiId::moves_randomly_when_unaware}
};

const std::unordered_map<AiId, std::string> ai_id_to_str_map = {
    {AiId::looks, "looks"},
    {AiId::avoids_blocking_friend, "avoids_blocking_friend"},
    {AiId::attacks, "attacks"},
    {AiId::paths_to_target_when_aware, "paths_to_target_when_aware"},
    {AiId::moves_to_target_when_los, "moves_to_target_when_los"},
    {AiId::moves_to_lair, "moves_to_lair"},
    {AiId::moves_to_leader, "moves_to_leader"},
    {AiId::moves_randomly_when_unaware, "moves_randomly_when_unaware"}
};

struct ActorDataT
{
    ActorDataT()
    {
        reset();
    }

    void reset();

    ActorId id;
    std::string name_a;
    std::string name_the;
    std::string corpse_name_a;
    std::string corpse_name_the;
    TileId tile;
    char character;
    Color color;
    std::vector<MonGroupSpawnRule> group_sizes;
    int hp;
    int spi;
    int dmg_melee;
    int dmg_ranged;
    int speed_pct;
    AbilityVals ability_vals;
    bool natural_props[(size_t)PropId::END];
    int ranged_cooldown_turns;
    bool ai[(size_t)AiId::END];
    int nr_turns_aware;
    int spawn_min_dlvl, spawn_max_dlvl;
    ActorSize actor_size;
    bool is_humanoid;
    bool allow_generated_descr;
    int nr_kills;
    bool has_player_seen;
    bool can_open_doors, can_bash_doors;
    // NOTE: Knockback may also be prevented by other soucres, e.g. if
    // the monster is ethereal
    bool prevent_knockback;
    int nr_left_allowed_to_spawn;
    bool is_unique;
    bool is_auto_spawn_allowed;
    std::string descr;
    std::string wary_msg;
    std::string aware_msg_mon_seen;
    std::string aware_msg_mon_hidden;
    SfxId aware_sfx_mon_seen;
    SfxId aware_sfx_mon_hidden;
    std::string spell_cast_msg;
    int erratic_move_pct;
    ShockLvl mon_shock_lvl;
    bool is_rat;
    bool is_canine;
    bool is_spider;
    bool is_undead;
    bool is_ghost;
    bool is_snake;
    bool is_reptile;
    bool is_amphibian;
    bool can_be_summoned_by_mon;
    bool can_bleed;
    bool can_leave_corpse;
    bool prio_corpse_bash;
    std::vector<RoomType> native_rooms;
};

namespace actor_data
{

extern ActorDataT data[(size_t)ActorId::END];

void init();

void save();
void load();

} // actor_data

#endif // ACTOR_DATA_HPP
