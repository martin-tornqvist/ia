#ifndef ACTOR_FACTORY
#define ACTOR_FACTORY

#include <vector>

#include "actor_data.hpp"

class Actor;
class Mon;

enum class Make_mon_aware
{
  no,
  yes
};

namespace actor_factory
{

void delete_all_mon();

Actor* mk(const Actor_id id, const P& pos);

void summon(const P& origin,
            const std::vector<Actor_id>& monster_ids,
            const Make_mon_aware make_aware = Make_mon_aware::yes,
            Actor* const actor_to_set_as_leader = nullptr,
            std::vector<Mon*>* monsters_ret = nullptr,
            Verbosity verbosity = Verbosity::verbose);

} //actor_factory

#endif
