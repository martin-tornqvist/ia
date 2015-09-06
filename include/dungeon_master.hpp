#ifndef DUNGEON_MASTER_H
#define DUNGEON_MASTER_H

#include <vector>
#include <string>

struct Time_data;
struct Actor_data_t;
class Actor;

namespace dungeon_master
{

void init();

void store_to_save_lines(std::vector<std::string>& lines);
void setup_from_save_lines(std::vector<std::string>& lines);

int clvl();
int xp();
Time_data start_time();

void on_mon_killed(Actor& actor);
void on_mon_seen(Actor& actor);

void win_game();

void set_time_started_to_now();

void player_lose_xp_percent(const int PERCENT);

int mon_tot_xp_worth(const Actor_data_t& d);

int xp_to_next_lvl();

void incr_player_xp(const int XP_GAINED);

} //Dungeon_master

#endif
