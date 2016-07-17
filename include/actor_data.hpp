#ifndef ACTOR_DATA_HPP
#define ACTOR_DATA_HPP

#include <string>
#include <vector>

#include "ability_values.hpp"
#include "global.hpp"
#include "properties.hpp"
#include "audio.hpp"
#include "room.hpp"

enum class ActorId
{
    player,
    zombie,
    zombie_axe,
    bloated_zombie,
    crawling_intestines,
    crawling_hand,
    thing,
    floating_skull,
    cultist,
    cultist_electric,
    cultist_spike_gun,
    cultist_priest,
    rat,
    rat_thing,
    green_spider,
    white_spider,
    red_spider,
    shadow_spider,
    leng_spider,
    pit_viper,
    spitting_cobra,
    black_mamba,
    mi_go,
    mi_go_commander,
    flying_polyp,
    greater_polyp,
    ghoul,
    shadow,
    invis_stalker,
    wolf,
    fire_hound,
    zuul,
    ghost,
    phantasm,
    wraith,
    raven,
    giant_bat,
    byakhee,
    giant_mantis,
    locust,
    mummy,
    croc_head_mummy,
    khephren,
    nitokris,
    deep_one,
    ape,
    keziah_mason,
    brown_jenkin,
    leng_elder,
    major_clapham_lee,
    dean_halsey,
    worm_mass,
    mind_worms,
    dust_vortex,
    fire_vortex,
    ooze_black,
    ooze_clear,
    ooze_putrid,
    ooze_poison,
    color_oo_space,
    chthonian,
    hunting_horror,
    sentry_drone,
    animated_wpn,
    mold,
    gas_spore,
    death_fiend,
    the_high_priest,

    END
};

enum class MonGroupSize
{
    alone,
    few,
    pack,
    swarm
};

enum class ActorSpeed
{
    sluggish,
    slow,
    normal,
    fast,
    fastest,
    END
};

enum class ActorErraticFreq
{
    never    = 0,
    rare     = 8,
    somewhat = 25,
    very     = 50
};

enum class ActorSize
{
    none,
    floor,
    humanoid,
    giant
};

enum class MonShockLvl
{
    none,
    unsettling,
    frightening,
    terrifying,
    mind_shattering,
    END
};

enum class AiId
{
    looks,
    makes_room_for_friend,
    attacks,
    paths_to_tgt_when_aware,
    moves_to_tgt_when_los,
    moves_to_lair,
    moves_to_leader,
    moves_to_random_when_unaware,
    END
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
    char glyph;
    Clr color;
    MonGroupSize group_size;
    int hp, spi, dmg_melee, dmg_ranged;
    ActorSpeed speed;
    AbilityVals ability_vals;
    bool natural_props[(size_t)PropId::END];
    int ranged_cooldown_turns, spell_cooldown_turns;
    bool ai[(size_t)AiId::END];
    int nr_turns_aware;
    int spawn_min_dlvl, spawn_max_dlvl;
    ActorSize actor_size;
    bool is_humanoid;
    bool is_infra_visible;
    bool is_auto_descr_allowed;
    int nr_kills;
    bool can_open_doors, can_bash_doors;
    //NOTE: Knockback may be prevented even if this is false, e.g. if monster is ethereal
    bool prevent_knockback;
    int nr_left_allowed_to_spawn;
    bool is_unique;
    bool is_auto_spawn_allowed;
    std::string spell_cast_msg;
    ActorErraticFreq erratic_move_pct;
    MonShockLvl mon_shock_lvl;
    bool is_rat, is_canine, is_spider, is_undead, is_ghost, is_snake, is_reptile, is_amphibian;
    bool can_be_summoned;
    bool can_bleed;
    bool can_leave_corpse;
    bool prio_corpse_bash;
    std::vector<RoomType> native_rooms;
    std::string descr;
    std::string aggro_text_mon_seen;
    std::string aggro_text_mon_hidden;
    SfxId aggro_sfx_mon_seen;
    SfxId aggro_sfx_mon_hidden;
};

namespace actor_data
{

extern ActorDataT data[(size_t)ActorId::END];

void init();

void save();
void load();

} //actor_data

#endif
