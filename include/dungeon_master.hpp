#ifndef DUNGEON_MASTER_H
#define DUNGEON_MASTER_H

#include <vector>
#include <string>

struct Time_data;
struct actor_data_t;
class Actor;

namespace dungeon_master
{

void init();

void store_to_save_lines(std::vector<std::string>& lines);
void setup_from_save_lines(std::vector<std::string>& lines);

int get_cLvl();
int get_xp();
Time_data get_start_time();

void on_mon_killed(Actor& actor);
void on_mon_seen(Actor& actor);

void win_game();

void set_time_started_to_now();

void player_lose_xp_percent(const int PERCENT);

int get_mon_tot_xp_worth(const actor_data_t& d);

int get_xp_to_next_lvl();

void player_gain_xp(const int XP_GAINED);

} //Dungeon_master

#endif
