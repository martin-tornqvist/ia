#ifndef FEATURE_DATA_H
#define FEATURE_DATA_H

#include <functional>
#include "art.hpp"
#include "cmn_data.hpp"
#include "room.hpp"
#include "map_patterns.hpp"
#include "properties.hpp"

enum class Feature_id
{
    floor,
    bridge,
    wall,
    tree,
    grass,
    bush,
    stairs,
    lever,
    brazier,
    gravestone,
    tomb,
    church_bench,
    altar,
    carpet,
    rubble_high,
    rubble_low,
    bones,
    statue,
    cocoon,
    chest,
    cabinet,
    fountain,
    pillar,
    monolith,
    stalagmite,
    chasm,
    liquid_shallow,
    liquid_deep,
    door,
    lit_dynamite,
    lit_flare,
    trap,
    smoke,
    event_wall_crumble,
    event_snake_emerge,
    event_rats_in_the_walls_discovery,

    END
};

struct Feature_room_spawn_rules
{
public:
    Feature_room_spawn_rules();

    void reset();

    void set(const int MAX_NR_IN_ROOM, const Range& dlvls_allowed,
             const Placement_rule placement_rule,
             std::initializer_list<Room_type> room_types);

    bool is_belonging_to_room_type(const Room_type type) const;
    Placement_rule placement_rule() const;
    int max_nr_in_room() const;
    Range dlvls_allowed() const;

private:
    int max_nr_in_room_;
    Range dlvls_allowed_;
    Placement_rule placement_rule_;
    std::vector<Room_type> room_types_native_;
};

class Actor;

class Move_rules
{
public:
    Move_rules()
    {
        reset();
    }

    ~Move_rules() {}

    void reset()
    {
        can_move_cmn_ = false;

        for (int i = 0; i < int(Prop_id::END); ++i)
        {
            can_move_if_have_prop_[i] = false;
        }
    }

    void set_prop_can_move(const Prop_id id)
    {
        can_move_if_have_prop_[int(id)] = true;
    }

    void set_can_move_cmn()
    {
        can_move_cmn_ = true;
    }

    bool can_move_cmn() const
    {
        return can_move_cmn_;
    }

    bool can_move(Actor& actor) const;

private:
    bool can_move_cmn_;
    bool can_move_if_have_prop_[size_t(Prop_id::END)];
};

class Feature;

struct Feature_data_t
{
    std::function<Feature*(const P& p)> mk_obj;
    Feature_id id;
    char glyph;
    Tile_id tile;
    Move_rules move_rules;
    bool is_sound_passable;
    bool is_projectile_passable;
    bool is_los_passable;
    bool is_smoke_passable;
    bool can_have_blood;
    bool can_have_gore;
    bool can_have_corpse;
    bool can_have_rigid;
    bool can_have_item;
    bool is_bottomless;
    Matl matl_type;
    std::string msg_on_player_blocked;
    std::string msg_on_player_blocked_blind;
    int dodge_modifier;
    int shock_when_adjacent;
    Feature_room_spawn_rules room_spawn_rules;
};

namespace feature_data
{

void init();

const Feature_data_t& data(const Feature_id id);

} //Feature_data

#endif
