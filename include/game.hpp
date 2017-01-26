#ifndef GAME_HPP
#define GAME_HPP

#include <vector>
#include <string>

#include "global.hpp"
#include "state.hpp"
#include "colors.hpp"
#include "time.hpp"

struct InputData;
class Actor;

struct HistoryEvent
{
    HistoryEvent(const std::string msg, const int turn) :
        msg     (msg),
        turn    (turn) {}

    const std::string msg;
    const int turn;
};

struct CellRenderData
{
    CellRenderData() :
        clr                             (clr_black),
        clr_bg                          (clr_black),
        tile                            (TileId::empty),
        glyph                           (0),
        is_light_fade_allowed           (true),
        mark_lit                        (false),
        is_living_actor_seen_here       (false),
        is_aware_of_hostile_mon_here    (false),
        is_aware_of_allied_mon_here     (false),
        lifebar_length                  (-1) {}

    CellRenderData& operator=(const CellRenderData&) = default;

    Clr clr;
    Clr clr_bg;
    TileId tile;
    char glyph;
    bool is_light_fade_allowed;
    bool mark_lit;
    bool is_living_actor_seen_here;
    bool is_aware_of_hostile_mon_here;
    bool is_aware_of_allied_mon_here;
    int lifebar_length;
};

namespace game
{

extern CellRenderData render_array[map_w][map_h];
extern CellRenderData render_array_no_actors[map_w][map_h];

void init();

void save();
void load();

void handle_player_input(const InputData& input);

int clvl();
int xp();
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
        State       (),
        entry_mode_ (entry_mode) {}

    void on_start() override;

    void draw() override;

    void update() override;

private:
    void query_quit();

    void draw_map();

    const GameEntryMode entry_mode_;
};

#endif // GAME_HPP
