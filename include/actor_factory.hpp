#ifndef ACTOR_FACTORY
#define ACTOR_FACTORY

#include <vector>

#include "actor_data.hpp"

class Actor;
class Mon;

namespace actor_factory
{

void delete_all_mon();

Actor* mk(const Actor_id id, const P& pos);

void summon(const P& origin,
            const std::vector<Actor_id>& monster_ids,
            const bool MAKE_MONSTERS_AWARE,
            Actor* const actor_to_set_as_leader = nullptr,
            std::vector<Mon*>* monsters_ret = nullptr);

} //Actor_factory

#endif
