#include "actor_factory.hpp"

#include <algorithm>

#include "actor.hpp"
#include "actor_data.hpp"
#include "actor_mon.hpp"
#include "actor_player.hpp"
#include "map.hpp"
#include "io.hpp"
#include "map_parsing.hpp"
#include "actor.hpp"
#include "feature_rigid.hpp"
#include "init.hpp"
#include "io.hpp"

MonSpawnResult& MonSpawnResult::set_leader(Actor* const leader)
{
    std::for_each(begin(monsters), end(monsters), [leader](auto mon)
    {
        mon->leader_ = leader;
    });

    return *this;
}

MonSpawnResult& MonSpawnResult::make_aware_of_player()
{
    std::for_each(begin(monsters), end(monsters), [](auto mon)
    {
        mon->aware_of_player_counter_ = mon->data().nr_turns_aware;
    });

    return *this;
}

namespace actor_factory
{

namespace
{

Actor* make_actor_from_id(const ActorId id)
{
    switch (id)
    {
    case ActorId::player:
        return new Player();

    case ActorId::khephren:
        return new Khephren();

    case ActorId::ape:
        return new Ape();

    case ActorId::strange_color:
        return new StrangeColor();

    case ActorId::animated_wpn:
        return new AnimatedWpn();

    default:
        break;
    }

    return new Mon();
}

std::vector<P> free_spawn_positions(const R& area)
{
        Array2<bool> blocked(map::dims());

        map_parsers::BlocksMoveCommon(ParseActors::yes)
                .run(blocked,
                     area,
                     MapParseMode::overwrite);

        const auto free_positions = to_vec(blocked, false, area);

        return free_positions;
}

Mon* spawn_at(const P& pos, const ActorId id)
{
    ASSERT(map::is_pos_inside_outer_walls(pos));

    Actor* const actor = make(id, pos);

    Mon* const mon = static_cast<Mon*>(actor);

    if (map::player->can_see_actor(*mon))
    {
        mon->set_player_aware_of_me();
    }

    return mon;
}

MonSpawnResult spawn_at_positions(const std::vector<P>& positions,
                                  const std::vector<ActorId>& ids)
{
    MonSpawnResult result;

    const size_t nr_to_spawn = std::min(positions.size(), ids.size());

    for (size_t i = 0; i < nr_to_spawn; ++i)
    {
        const P& pos = positions[i];

        const ActorId id = ids[i];

        result.monsters.emplace_back(
            spawn_at(pos, id));
    }

    return result;
}

} // namespace

Actor* make(const ActorId id, const P& pos)
{
    Actor* const actor = make_actor_from_id(id);

    actor->init(pos, actor_data::data[(size_t)id]);

    auto& data = actor->data();

    if (data.nr_left_allowed_to_spawn > 0)
    {
        --data.nr_left_allowed_to_spawn;
    }

    game_time::add_actor(actor);

    actor->properties().on_placed();

    return actor;
}

void delete_all_mon()
{
    std::vector<Actor*>& actors = game_time::actors;

    for (auto it = begin(actors); it != end(actors);)
    {
        Actor* const actor = *it;

        if (actor->is_player())
        {
            ++it;
        }
        else // Is monster
        {
            delete actor;

            it = actors.erase(it);
        }
    }
}

MonSpawnResult spawn(const P& origin,
                     const std::vector<ActorId>& monster_ids,
                     const R& area_allowed)
{
    TRACE_FUNC_BEGIN;

    auto free_positions = free_spawn_positions(area_allowed);

    if (free_positions.empty())
    {
        return MonSpawnResult();
    }

    std::sort(begin(free_positions),
              end(free_positions),
              IsCloserToPos(origin));

    const auto result = spawn_at_positions(free_positions, monster_ids);

    TRACE_FUNC_END;

    return result;
}

MonSpawnResult spawn_random_position(const std::vector<ActorId>& monster_ids,
                                     const R& area_allowed)
{
    TRACE_FUNC_BEGIN;

    auto free_positions = free_spawn_positions(area_allowed);

    if (free_positions.empty())
    {
        return MonSpawnResult();
    }

    rnd::shuffle(free_positions);

    const auto result = spawn_at_positions(free_positions, monster_ids);

    TRACE_FUNC_END;

    return result;
}

} // actor_factory
