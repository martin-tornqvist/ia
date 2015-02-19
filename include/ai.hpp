#ifndef AI_H
#define AI_H

#include <vector>

#include "cmn_types.hpp"
#include "cmn_data.hpp"

class Mon;

namespace Ai
{

namespace Action
{

bool try_cast_random_spell(Mon& mon);

bool handle_closed_blocking_door(Mon& mon, std::vector<Pos> path);

bool handle_inventory(Mon& mon);

bool make_room_for_friend(Mon& mon);

bool move_to_random_adj_cell(Mon& mon);

bool move_to_tgt_simple(Mon& mon);

bool step_path(Mon& mon, std::vector<Pos>& path);

bool step_to_lair_if_los(Mon& mon, const Pos& lair_cell);

} //Action

namespace Info
{

bool look_become_player_aware(Mon& mon);

void set_path_to_lair_if_no_los(Mon& mon, std::vector<Pos>& path, const Pos& lair_cell);

void set_path_to_leader_if_no_los_toleader(Mon& mon, std::vector<Pos>& path);

void set_path_to_player_if_aware(Mon& mon, std::vector<Pos>& path);

void set_special_blocked_cells(Mon& mon, bool a[MAP_W][MAP_H]);

} //Info

} //Ai

#endif
