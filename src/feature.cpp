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

const Feature_data_t& Feature::get_data() const
{
    return feature_data::get_data(get_id());
}

void Feature::bump(Actor& actor_bumping)
{
    bool props[size_t(Prop_id::END)];
    actor_bumping.get_prop_handler().get_prop_ids(props);

    if (!can_move(props))
    {
        if (actor_bumping.is_player())
        {
            if (map::player->get_prop_handler().allow_see())
            {
                msg_log::add(get_data().msg_on_player_blocked);
            }
            else
            {
                msg_log::add(get_data().msg_on_player_blocked_blind);
            }
        }
    }
}

void Feature::add_light(bool light[MAP_W][MAP_H]) const
{
    (void)light;
}

bool Feature::can_move_cmn() const {return get_data().move_rules.can_move_cmn();}

bool Feature::can_move(const bool actor_prop_ids[size_t(Prop_id::END)]) const
{
    return get_data().move_rules.can_move(actor_prop_ids);
}

void Feature::hit(const Dmg_type dmg_type, const Dmg_method dmg_method, Actor* const actor)
{
    (void)dmg_type; (void)dmg_method; (void)actor;
}

bool      Feature::is_sound_passable()      const {return get_data().is_sound_passable;}
bool      Feature::is_los_passable()     const {return get_data().is_los_passable;}
bool      Feature::is_projectile_passable() const {return get_data().is_projectile_passable;}
bool      Feature::is_smoke_passable()      const {return get_data().is_smoke_passable;}
bool      Feature::is_bottomless()         const {return get_data().is_bottomless;}
char      Feature::get_glyph()             const {return get_data().glyph;}
Tile_id    Feature::get_tile()              const {return get_data().tile;}
bool      Feature::can_have_corpse() const {return get_data().can_have_corpse;}
bool      Feature::can_have_rigid()         const {return get_data().can_have_rigid;}
bool      Feature::can_have_blood()         const {return get_data().can_have_blood;}
bool      Feature::can_have_gore()          const {return get_data().can_have_gore;}
bool      Feature::can_have_item()          const {return get_data().can_have_item;}
Feature_id Feature::get_id()                const {return get_data().id;}
int       Feature::get_dodge_modifier()     const {return get_data().dodge_modifier;}
int       Feature::get_shock_when_adj()      const {return get_data().shock_when_adjacent;}
Matl      Feature::get_matl()              const {return get_data().matl_type;}
