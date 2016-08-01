#ifndef DUNGEON_MASTER_HPP
#define DUNGEON_MASTER_HPP

#include <vector>
#include <string>

#include "global.hpp"

struct TimeData;
struct ActorDataT;
class Actor;

struct HistoryEvent
{
    HistoryEvent(const std::string msg, const int turn) :
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
TimeData start_time();

void on_mon_killed(Actor& actor);

void win_game();

void set_start_time_to_now();

void incr_player_xp(const int xp_gained,
                    const Verbosity verbosity = Verbosity::verbose);

void add_history_event(const std::string msg);

const std::vector<HistoryEvent>& history();

} //dungeon_master

#endif
