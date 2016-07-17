#ifndef DUNGEON_MASTER_HPP
#define DUNGEON_MASTER_HPP

#include <vector>
#include <string>

struct Time_data;
struct Actor_data_t;
class Actor;

struct History_event
{
    History_event(const std::string msg, const int turn) :
        msg     (msg),
        turn    (turn) {}

    const std::string msg;
    const int turn;
};

namespace dungeon_master
{

void init();

void save();
void load();

int clvl();
int xp();
Time_data start_time();

void on_mon_killed(Actor& actor);
void on_mon_seen(Actor& actor);

void win_game();

void set_time_started_to_now();

void player_lose_xp_percent(const int percent);

int mon_tot_xp_worth(const Actor_data_t& d);

int xp_to_next_lvl();

void incr_player_xp(const int xp_gained);

void add_history_event(const std::string msg);

const std::vector<History_event>& history();

} //Dungeon_master

#endif
