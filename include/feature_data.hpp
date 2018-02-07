#ifndef FEATURE_DATA_HPP
#define FEATURE_DATA_HPP

#include <functional>
#include "gfx.hpp"
#include "map_patterns.hpp"
#include "property_data.hpp"

enum class FeatureId
{
    floor,
    bridge,
    wall,
    tree,
    grass,
    bush,
    vines,
    chains,
    grate,
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
    bookshelf,
    alchemist_bench,
    fountain,
    monolith,
    pylon,
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
    event_rat_cave_discovery,

    END
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
        can_move_common_ = false;

        props_allow_move_.clear();
    }

    void set_prop_can_move(const PropId id)
    {
        props_allow_move_.push_back(id);
    }

    void set_can_move_common()
    {
        can_move_common_ = true;
    }

    bool can_move_common() const
    {
        return can_move_common_;
    }

    bool can_move(Actor& actor) const;

private:
    bool can_move_common_;
    std::vector<PropId> props_allow_move_;
};

class Feature;

struct FeatureDataT
{
    std::function<Feature*(const P& p)> mk_obj;
    FeatureId id;
    char character;
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
    FeaturePlacement auto_spawn_placement;
};

namespace feature_data
{

void init();

const FeatureDataT& data(const FeatureId id);

} // feature_data

#endif
