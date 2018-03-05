#ifndef GAME_HPP
#define GAME_HPP

#include <vector>
#include <string>

#include "global.hpp"
#include "state.hpp"
#include "time.hpp"

struct InputData;
class Actor;

struct HistoryEvent
{
    HistoryEvent(const std::string msg, const int turn) :
        msg(msg),
        turn(turn) {}

    const std::string msg;
    const int turn;
};

namespace game
{

void init();

void save();
void load();

void handle_player_input(const InputData& input);

int clvl();
int xp_pct();
int xp_accumulated();
TimeData start_time();

void on_mon_seen(Actor& actor);

void on_mon_killed(Actor& actor);

void win_game();

void set_start_time_to_now();

void incr_player_xp(const int xp_gained,
                    const Verbosity verbosity = Verbosity::verbose);

void incr_clvl();

void add_history_event(const std::string msg);

const std::vector<HistoryEvent>& history();

} // game

// -----------------------------------------------------------------------------
// Game state
// -----------------------------------------------------------------------------
class GameState: public State
{
public:
    GameState(GameEntryMode entry_mode) :
        State(),
        entry_mode_(entry_mode) {}

    void on_start() override;

    void draw() override;

    void update() override;

    StateId id() override;

private:
    void query_quit();

    const GameEntryMode entry_mode_;
};

#endif // GAME_HPP
