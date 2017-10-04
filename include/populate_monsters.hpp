#ifndef POPULATE_MON_HPP
#define POPULATE_MON_HPP

#include <vector>

#include "actor_data.hpp"

enum class AllowSpawnUniqueMon
{
    no,
    yes
};

namespace populate_mon
{

void mk_group_at(const ActorId id,
                 const std::vector<P>& sorted_free_cells,
                 bool blocked_out[map_w][map_h],
                 const MonRoamingAllowed roaming_allowed);

//
// TODO: This is a very general function, it should not be here
//
std::vector<P> mk_sorted_free_cells(const P& origin,
                                    const bool blocked[map_w][map_h]);

void try_spawn_due_to_time_passed();

void populate_std_lvl();

} // populate_mon

#endif // POPULATE_MON_HPP
