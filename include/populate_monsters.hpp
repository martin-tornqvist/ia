#ifndef POPULATE_H
#define POPULATE_H

#include <vector>

#include "actor_data.hpp"

namespace populate_mon
{

void try_spawn_due_to_time_passed();

void populate_std_lvl();

void populate_intro_lvl();

void mk_group_at(const Actor_id id, const std::vector<Pos>& sorted_free_cells_vector,
               bool blocked[MAP_W][MAP_H], const bool IS_ROAMING_ALLOWED);

void mk_sorted_free_cells_vector(const Pos& origin,
                             const bool blocked[MAP_W][MAP_H],
                             std::vector<Pos>& vector_ref);

} //Populate_mon

#endif
