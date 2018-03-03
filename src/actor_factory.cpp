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

    case ActorId::zuul:
        return new Zuul();

    case ActorId::ghost:
        return new Ghost();

    case ActorId::cultist:
        return new Cultist();

    case ActorId::bog_tcher:
        return new BogTcher();

    case ActorId::cultist_priest:
        return new CultistPriest();

    case ActorId::cultist_wizard:
        return new CultistWizard();

    case ActorId::cultist_arch_wizard:
        return new CultistArchWizard();

    case ActorId::keziah_mason:
        return new KeziahMason();

    case ActorId::shadow:
        return new Shadow();

    case ActorId::khephren:
        return new Khephren();

    case ActorId::ape:
        return new Ape();

    case ActorId::strange_color:
        return new StrangeColor();

    case ActorId::animated_wpn:
        return new AnimatedWpn();

    case ActorId::mold:
        return new Mold();

    case ActorId::the_high_priest:
        return new TheHighPriest();

    case ActorId::zombie:
    case ActorId::zombie_axe:
    case ActorId::bloated_zombie:
    case ActorId::major_clapham_lee:
    case ActorId::dean_halsey:
    case ActorId::crawling_intestines:
    case ActorId::crawling_hand:
    case ActorId::thing:
    case ActorId::floating_skull:
    case ActorId::rat:
    case ActorId::rat_thing:
    case ActorId::brown_jenkin:
    case ActorId::green_spider:
    case ActorId::red_spider:
    case ActorId::white_spider:
    case ActorId::shadow_spider:
    case ActorId::leng_spider:
    case ActorId::pit_viper:
    case ActorId::spitting_cobra:
    case ActorId::black_mamba:
    case ActorId::fire_hound:
    case ActorId::energy_hound:
    case ActorId::raven:
    case ActorId::giant_bat:
    case ActorId::vampire_bat:
    case ActorId::abaxu:
    case ActorId::wolf:
    case ActorId::flying_polyp:
    case ActorId::greater_polyp:
    case ActorId::mind_leech:
    case ActorId::spirit_leech:
    case ActorId::life_leech:
    case ActorId::mi_go:
    case ActorId::mi_go_commander:
    case ActorId::ghoul:
    case ActorId::void_traveler:
    case ActorId::elder_void_traveler:
    case ActorId::invis_stalker:
    case ActorId::byakhee:
    case ActorId::giant_mantis:
    case ActorId::locust:
    case ActorId::mummy:
    case ActorId::croc_head_mummy:
    case ActorId::nitokris:
    case ActorId::deep_one:
    case ActorId::worm_mass:
    case ActorId::mind_worms:
    case ActorId::dust_vortex:
    case ActorId::fire_vortex:
    case ActorId::energy_vortex:
    case ActorId::ooze_black:
    case ActorId::ooze_clear:
    case ActorId::ooze_putrid:
    case ActorId::ooze_poison:
    case ActorId::chthonian:
    case ActorId::death_fiend:
    case ActorId::hunting_horror:
    case ActorId::sentry_drone:
    case ActorId::gas_spore:
    case ActorId::high_priest_guard_war_vet:
    case ActorId::high_priest_guard_rogue:
    case ActorId::high_priest_guard_ghoul:
        return new Mon();

    case ActorId::END:
        break;
    }

    return nullptr;
}

std::vector<P> free_spawn_positions(const R& area)
{
    bool blocked[map_w][map_h];

    map_parsers::BlocksMoveCommon(ParseActors::yes)
        .run(blocked,
             MapParseMode::overwrite,
             area);

    const auto free_positions = to_vec(blocked, false, area);

    return free_positions;
}

Mon* spawn_at(const P& pos,
              const ActorId id)
{
    ASSERT(map::is_pos_inside_map(pos, false));

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

    for (auto it = begin(actors); it != end(actors); /* No increment */)
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
