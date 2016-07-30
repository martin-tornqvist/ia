#ifndef FEATURE_DATA_HPP
#define FEATURE_DATA_HPP

#include <functional>
#include "art.hpp"
#include "room.hpp"
#include "map_patterns.hpp"
#include "properties.hpp"

enum class FeatureId
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

struct FeatureRoomSpawnRules
{
public:
    FeatureRoomSpawnRules();

    void reset();

    void set(const int max_nr_in_room,
             const Range& dlvls_allowed,
             const PlacementRule placement_rule,
             std::initializer_list<RoomType> room_types);

    bool is_belonging_to_room_type(const RoomType type) const;
    PlacementRule placement_rule() const;
    int max_nr_in_room() const;
    Range dlvls_allowed() const;

private:
    int max_nr_in_room_;
    Range dlvls_allowed_;
    PlacementRule placement_rule_;
    std::vector<RoomType> room_types_native_;
};

class Actor;

class MoveRules
{
public:
    MoveRules()
    {
        reset();
    }

    ~MoveRules() {}

    void reset()
    {
        can_move_cmn_ = false;

        for (int i = 0; i < (int)PropId::END; ++i)
        {
            can_move_if_have_prop_[i] = false;
        }
    }

    void set_prop_can_move(const PropId id)
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
    bool can_move_if_have_prop_[(size_t)PropId::END];
};

class Feature;

struct FeatureDataT
{
    std::function<Feature*(const P& p)> mk_obj;
    FeatureId id;
    char glyph;
    TileId tile;
    MoveRules move_rules;
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
    FeatureRoomSpawnRules room_spawn_rules;
};

namespace feature_data
{

void init();

const FeatureDataT& data(const FeatureId id);

} //FeatureData

#endif
