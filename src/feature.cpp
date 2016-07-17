#include "feature.hpp"

#include "init.hpp"
#include "actor.hpp"
#include "actor_player.hpp"
#include "msg_log.hpp"
#include "render.hpp"
#include "map_parsing.hpp"
#include "game_time.hpp"
#include "map_travel.hpp"
#include "query.hpp"
#include "save_handling.hpp"
#include "popup.hpp"
#include "map.hpp"
#include "feature_data.hpp"

const FeatureDataT& Feature::data() const
{
    return feature_data::data(id());
}

void Feature::bump(Actor& actor_bumping)
{
    if (!can_move(actor_bumping))
    {
        if (actor_bumping.is_player())
        {
            if (map::cells[pos_.x][pos_.y].is_seen_by_player)
            {
                msg_log::add(data().msg_on_player_blocked);
            }
            else //Actor cannot see
            {
                msg_log::add(data().msg_on_player_blocked_blind);
            }
        }
    }
}

void Feature::add_light(bool light[map_w][map_h]) const
{
    (void)light;
}

bool Feature::can_move_cmn() const
{
    return data().move_rules.can_move_cmn();
}

bool Feature::can_move(Actor& actor) const
{
    return data().move_rules.can_move(actor);
}

void Feature::hit(const DmgType dmg_type, const DmgMethod dmg_method, Actor* const actor)
{
    (void)dmg_type; (void)dmg_method; (void)actor;
}

bool Feature::is_sound_passable() const
{
    return data().is_sound_passable;
}

bool Feature::is_los_passable() const
{
    return data().is_los_passable;
}

bool Feature::is_projectile_passable() const
{
    return data().is_projectile_passable;
}

bool Feature::is_smoke_passable() const
{
    return data().is_smoke_passable;
}

bool Feature::is_bottomless() const
{
    return data().is_bottomless;
}

char Feature::glyph() const
{
    return data().glyph;
}

TileId Feature::tile() const
{
    return data().tile;
}

bool Feature::can_have_corpse() const
{
    return data().can_have_corpse;
}

bool Feature::can_have_rigid() const
{
    return data().can_have_rigid;
}

bool Feature::can_have_blood() const
{
    return data().can_have_blood;
}

bool Feature::can_have_gore() const
{
    return data().can_have_gore;
}

bool Feature::can_have_item() const
{
    return data().can_have_item;
}

FeatureId Feature::id() const
{
    return data().id;
}

int Feature::dodge_modifier() const
{
    return data().dodge_modifier;
}

int Feature::shock_when_adj() const
{
    return data().shock_when_adjacent;
}

Matl Feature::matl() const
{
    return data().matl_type;
}
