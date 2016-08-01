#include "dungeon_master.hpp"

#include <vector>

#include "highscore.hpp"
#include "text_format.hpp"
#include "render.hpp"
#include "query.hpp"
#include "actor_player.hpp"
#include "character_lines.hpp"
#include "msg_log.hpp"
#include "sdl_wrapper.hpp"
#include "map.hpp"
#include "create_character.hpp"
#include "actor_mon.hpp"
#include "save_handling.hpp"

namespace dungeon_master
{

namespace
{

int         clvl_   = 0;
int         xp_pct_ = 0;
TimeData    start_time_;

std::vector<HistoryEvent> history_events_;

void player_gain_lvl()
{
    if (map::player->is_alive())
    {
        ++clvl_;

        msg_log::add("Welcome to level " + to_str(clvl_) + "!",
                     clr_green,
                     false,
                     MorePromptOnMsg::yes);

        create_character::pick_new_trait();

        map::player->change_max_hp(hp_per_lvl);

        map::player->change_max_spi(spi_per_lvl);

        render::draw_map_state();
    }
}

} //namespace

void init()
{
    clvl_   = 1;
    xp_pct_ = 0;

    history_events_.clear();
}

void save()
{
    save_handling::put_int(clvl_);
    save_handling::put_int(xp_pct_);
    save_handling::put_int(start_time_.year_);
    save_handling::put_int(start_time_.month_);
    save_handling::put_int(start_time_.day_);
    save_handling::put_int(start_time_.hour_);
    save_handling::put_int(start_time_.minute_);
    save_handling::put_int(start_time_.second_);

    save_handling::put_int(history_events_.size());

    for (const HistoryEvent& event : history_events_)
    {
        save_handling::put_str(event.msg);
        save_handling::put_int(event.turn);
    }
}

void load()
{
    clvl_               = save_handling::get_int();
    xp_pct_             = save_handling::get_int();
    start_time_.year_   = save_handling::get_int();
    start_time_.month_  = save_handling::get_int();
    start_time_.day_    = save_handling::get_int();
    start_time_.hour_   = save_handling::get_int();
    start_time_.minute_ = save_handling::get_int();
    start_time_.second_ = save_handling::get_int();

    const int nr_events = save_handling::get_int();

    for (int i = 0; i < nr_events; ++i)
    {
        const std::string   msg     = save_handling::get_str();
        const int           turn    = save_handling::get_int();

        history_events_.push_back({msg, turn});
    }
}

int clvl()
{
    return clvl_;
}

int xp()
{
    return xp_pct_;
}

TimeData start_time()
{
    return start_time_;
}

void incr_player_xp(const int xp_gained,
                    const Verbosity verbosity)
{
    if (!map::player->is_alive())
    {
        return;
    }

    if (verbosity == Verbosity::verbose)
    {
        msg_log::add("(+" + to_str(xp_gained) + "% XP).");
    }

    xp_pct_ += xp_gained;

    while (xp_pct_ >= 100)
    {
        if (clvl_ < player_max_clvl)
        {
            player_gain_lvl();
        }

        xp_pct_ -= 100;
    }
}

void win_game()
{
    render::cover_panel(Panel::screen);
    render::update_screen();

    highscore::on_game_over(true);

    const std::vector<std::string> win_msg =
    {
        "As I approach the crystal, an eerie glow illuminates the area. "
        "I notice a figure observing me from the edge of the light. "
        "There is no doubt in my mind concerning the nature of this entity; "
        "it is the Faceless God who dwells in the depths of the earth - Nyarlathotep!",

        "I panic. Why is it I find myself here, stumbling around in darkness? "
        "Is this all part of a plan? The being beckons me to gaze into the stone.",

        "In the radiance I see visions beyond eternity, visions of unreal reality, "
        "visions of the brightest light of day and the darkest night of madness. "
        "There is only onward now, I demand to attain everything!",

        "So I make a pact with the Fiend.",

        "I now harness the shadows that stride from world to world to sow death and madness. "
        "The destinies of all things on earth, living and dead, are mine."
    };

    const int padding       = 9;

    const int X0            = padding;
    const int max_w         = map_w - (padding * 2);

    const int line_delay    = 50;

    int y = 2;

    for (const std::string& section_msg : win_msg)
    {
        std::vector<std::string> section_lines;

        text_format::split(section_msg, max_w, section_lines);

        for (const std::string& line : section_lines)
        {
            render::draw_text(line,
                              Panel::screen,
                              P(X0, y),
                              clr_white,
                              clr_black);

            render::update_screen();

            sdl_wrapper::sleep(line_delay);

            ++y;
        }
        ++y;
    }

    ++y;

    const std::string cmd_label =
        "[space/esc/enter] to record high score and return to main menu";

    render::draw_text(cmd_label,
                      Panel::screen,
                      P(X0, screen_h - 2),
                      clr_popup_label);

    render::update_screen();

    query::wait_for_confirm();
}

void on_mon_killed(Actor& actor)
{
    ActorDataT& d = actor.data();

    d.nr_kills += 1;

    const int min_hp_for_sadism_bon = 4;

    if (
        d.hp >= min_hp_for_sadism_bon &&
        insanity::has_sympt(InsSymptId::sadism))
    {
        map::player->shock_ = std::max(0.0, map::player->shock_ - 3.0);
    }

    if (d.is_unique)
    {
        const std::string name = actor.name_the();

        add_history_event("Defeated " + name + ".");
    }
}

void set_start_time_to_now()
{
    start_time_ = current_time();
}

void add_history_event(const std::string msg)
{
    const int turn_nr = game_time::turn();

    history_events_.push_back({msg, turn_nr});
}

const std::vector<HistoryEvent>& history()
{
    return history_events_;
}

} //DungeonMaster
