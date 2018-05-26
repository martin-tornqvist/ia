#ifndef ACTOR_FACTORY_HPP
#define ACTOR_FACTORY_HPP

#include <vector>

#include "actor_data.hpp"
#include "rl_utils.hpp"

class Actor;
class Mon;

enum class MakeMonAware
{
  no,
  yes
};

class MonSpawnResult
{
public:
    MonSpawnResult() :
        monsters() {}

    MonSpawnResult& set_leader(Actor* const leader);

    MonSpawnResult& make_aware_of_player();

    // Generic for each function to perform any operation on the actors
    template<typename Func>
    MonSpawnResult& for_each(Func const& lambda)
    {
        std::for_each(begin(monsters),
                      end(monsters),
                      lambda);

        return *this;
    }

    std::vector<Mon*> monsters;
};

namespace actor_factory
{

void delete_all_mon();

Actor* make(const ActorId id, const P& pos);

MonSpawnResult spawn(
    const P& origin,
    const std::vector<ActorId>& monster_ids,
    const R& area_allowed);

MonSpawnResult spawn_random_position(
    const std::vector<ActorId>& monster_ids,
    const R& area_allowed);

} // actor_factory

#endif // ACTOR_FACTORY_HPP
