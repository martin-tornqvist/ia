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
#include "utils.hpp"
#include "map.hpp"
#include "feature_data.hpp"

using namespace std;

const Feature_data_t& Feature::data() const
{
    return feature_data::data(id());
}

void Feature::bump(Actor& actor_bumping)
{
    bool props[size_t(Prop_id::END)];
    actor_bumping.prop_handler().prop_ids(props);

    if (!can_move(props))
    {
        if (actor_bumping.is_player())
        {
            if (map::player->prop_handler().allow_see())
            {
                msg_log::add(data().msg_on_player_blocked);
            }
            else
            {
                msg_log::add(data().msg_on_player_blocked_blind);
            }
        }
    }
}

void Feature::add_light(bool light[MAP_W][MAP_H]) const
{
    (void)light;
}

bool Feature::can_move_cmn() const {return data().move_rules.can_move_cmn();}

bool Feature::can_move(const bool actor_prop_ids[size_t(Prop_id::END)]) const
{
    return data().move_rules.can_move(actor_prop_ids);
}

void Feature::hit(const Dmg_type dmg_type, const Dmg_method dmg_method, Actor* const actor)
{
    (void)dmg_type; (void)dmg_method; (void)actor;
}

bool      Feature::is_sound_passable()      const {return data().is_sound_passable;}
bool      Feature::is_los_passable()     const {return data().is_los_passable;}
bool      Feature::is_projectile_passable() const {return data().is_projectile_passable;}
bool      Feature::is_smoke_passable()      const {return data().is_smoke_passable;}
bool      Feature::is_bottomless()         const {return data().is_bottomless;}
char      Feature::glyph()             const {return data().glyph;}
Tile_id    Feature::tile()              const {return data().tile;}
bool      Feature::can_have_corpse() const {return data().can_have_corpse;}
bool      Feature::can_have_rigid()         const {return data().can_have_rigid;}
bool      Feature::can_have_blood()         const {return data().can_have_blood;}
bool      Feature::can_have_gore()          const {return data().can_have_gore;}
bool      Feature::can_have_item()          const {return data().can_have_item;}
Feature_id Feature::id()                const {return data().id;}
int       Feature::dodge_modifier()     const {return data().dodge_modifier;}
int       Feature::shock_when_adj()      const {return data().shock_when_adjacent;}
Matl      Feature::matl()              const {return data().matl_type;}
