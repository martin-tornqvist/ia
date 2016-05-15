#ifndef AI_HPP
#define AI_HPP

#include <vector>

#include "cmn.hpp"

class Mon;

namespace ai
{

namespace action
{

bool try_cast_random_spell(Mon& mon);

bool handle_closed_blocking_door(Mon& mon, std::vector<P> path);

bool handle_inventory(Mon& mon);

bool make_room_for_friend(Mon& mon);

bool move_to_random_adj_cell(Mon& mon);

bool move_to_tgt_simple(Mon& mon);

bool step_path(Mon& mon, std::vector<P>& path);

bool step_to_lair_if_los(Mon& mon, const P& lair_p);

} //action

namespace info
{

bool look_become_player_aware(Mon& mon);

void try_set_path_to_lair_if_no_los(Mon& mon, std::vector<P>& path, const P& lair_p);

void try_set_path_to_leader(Mon& mon, std::vector<P>& path);

void try_set_path_to_player(Mon& mon, std::vector<P>& path);

void set_special_blocked_cells(Mon& mon, bool a[MAP_W][MAP_H]);

} //info

} //ai

#endif
