#ifndef ACTOR_FACTORY_HPP
#define ACTOR_FACTORY_HPP

#include <vector>

#include "actor_data.hpp"

class Actor;
class Mon;

enum class MakeMonAware
{
  no,
  yes
};

namespace actor_factory
{

void delete_all_mon();

Actor* mk(const ActorId id, const P& pos);

std::vector<Mon*> summon(const P& origin,
                         const std::vector<ActorId>& monster_ids,
                         const MakeMonAware make_aware = MakeMonAware::yes,
                         Actor* const actor_to_set_as_leader = nullptr,
                         Verbosity verbosity = Verbosity::verbose);

} // actor_factory

#endif // ACTOR_FACTORY_HPP
