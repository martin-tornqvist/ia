#ifndef ACTOR_DATA_H
#define ACTOR_DATA_H

#include <string>
#include <vector>

#include "Converters.h"
#include "Ability_values.h"
#include "Cmn_data.h"
#include "Properties.h"
#include "Audio.h"
#include "Room.h"

enum class Actor_id
{
    player,
    zombie, zombie_axe, bloated_zombie,
    cultist, cultist_electric, cultist_spike_gun,
    cultist_priest,
    rat, rat_thing,
    green_spider, white_spider, red_spider, shadow_spider,
    leng_spider,
    mi_go, mi_go_commander,
    flying_polyp, greater_polyp,
    ghoul,
    shadow,
    wolf, fire_hound, frost_hound, zuul,
    ghost, phantasm, wraith,
    giant_bat, byakhee,
    giant_mantis, locust,
    mummy, croc_head_mummy, khephren, nitokris,
    deep_one,
    ape,
    keziah_mason, brown_jenkin,
    leng_elder,
    major_clapham_lee, dean_halsey,
    worm_mass,
    dust_vortex, fire_vortex, frost_vortex,
    ooze_black, ooze_clear, ooze_putrid, ooze_poison,
    colour_oOSpace,
    chthonian,
    hunting_horror,
    sentry_drone,
    mold,
    gas_spore,
    the_high_priest, the_high_priest_cpy,

    END
};

enum class Mon_group_size
{
    alone,
    few,
    group,
    horde,
    swarm
};

enum class Actor_speed
{
    sluggish,
    slow,
    normal,
    fast,
    fastest,
    END
};

enum class Actor_erratic_freq
{
    never    = 0,
    rare     = 8,
    somewhat = 25,
    very     = 50
};

enum class Actor_size {none, floor, humanoid, giant};

enum class Mon_shock_lvl {none, unsettling, scary, terrifying, mind_shattering, END};

enum class Ai_id
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

struct Actor_data_t
{
    Actor_data_t() {reset();}

    void reset();

    Actor_id id;
    std::string name_a;
    std::string name_the;
    std::string corpse_name_a;
    std::string corpse_name_the;
    Tile_id tile;
    char glyph;
    Clr color;
    Mon_group_size group_size;
    int hp, spi, dmg_melee, dmg_ranged;
    Actor_speed speed;
    Ability_vals ability_vals;
    bool intr_props[size_t(Prop_id::END)];
    int ranged_cooldown_turns, spell_cooldown_turns;
    bool ai[int(Ai_id::END)];
    int nr_turns_aware;
    int spawn_min_dLVL, spawn_max_dLVL;
    Actor_size actor_size;
    bool is_humanoid;
    bool is_auto_descr_allowed;
    std::string death_msg_override;
    int nr_kills;
    bool can_open_doors, can_bash_doors;
    bool can_see_in_darkness;
    //NOTE: Knockback may be prevented even if this is false, e.g. if monster is ethereal
    bool prevent_knockback;
    int nr_left_allowed_to_spawn;
    bool is_unique;
    bool is_auto_spawn_allowed;
    std::string spell_cast_msg;
    Actor_erratic_freq erratic_move_pct;
    Mon_shock_lvl mon_shock_lvl;
    bool is_rat, is_canine, is_spider, is_undead, is_ghost;
    bool can_be_summoned;
    bool can_bleed;
    bool can_leave_corpse;
    std::vector<Room_type> native_rooms;
    std::string descr;
    std::string aggro_text_mon_seen;
    std::string aggro_text_mon_hidden;
    Sfx_id aggro_sfx_mon_seen;
    Sfx_id aggro_sfx_mon_hidden;
};

namespace Actor_data
{

extern Actor_data_t data[int(Actor_id::END)];

void init();

void store_to_save_lines(std::vector<std::string>& lines);
void setup_from_save_lines(std::vector<std::string>& lines);

} //Actor_data

#endif
