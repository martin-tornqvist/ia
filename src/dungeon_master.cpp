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

int         xp_for_lvl_[player_max_clvl + 1];
int         clvl_  = 0;
int         xp_    = 0;
Time_data   time_started_;

std::vector<History_event> history_events_;

void player_gain_lvl()
{
    if (map::player->is_alive())
    {
        ++clvl_;

        msg_log::add("Welcome to level " + to_str(clvl_) + "!",
                     clr_green,
                     false,
                     More_prompt_on_msg::yes);

        create_character::pick_new_trait();

        render::draw_map_state();

        map::player->change_max_hp(hp_per_lvl);
        map::player->restore_hp(999, false, Verbosity::silent);

        map::player->change_max_spi(spi_per_lvl);
        map::player->restore_spi(999, false, Verbosity::silent);

        map::player->restore_shock(999, false);
    }
}

void init_xp_array()
{
    xp_for_lvl_[0] = 0;
    xp_for_lvl_[1] = 0;

    for (int lvl = 2; lvl <= player_max_clvl; lvl++)
    {
        xp_for_lvl_[lvl] = xp_for_lvl_[lvl - 1] + (100 * lvl);
    }
}

} //namespace

void init()
{
    clvl_ = 1;
    xp_   = 0;
    init_xp_array();

    history_events_.clear();
}

void save()
{
    save_handling::put_int(clvl_);
    save_handling::put_int(xp_);
    save_handling::put_int(time_started_.year_);
    save_handling::put_int(time_started_.month_);
    save_handling::put_int(time_started_.day_);
    save_handling::put_int(time_started_.hour_);
    save_handling::put_int(time_started_.minute_);
    save_handling::put_int(time_started_.second_);

    save_handling::put_int(history_events_.size());

    for (const History_event& event : history_events_)
    {
        save_handling::put_str(event.msg);
        save_handling::put_int(event.turn);
    }
}

void load()
{
    clvl_                   = save_handling::get_int();
    xp_                     = save_handling::get_int();
    time_started_.year_     = save_handling::get_int();
    time_started_.month_    = save_handling::get_int();
    time_started_.day_      = save_handling::get_int();
    time_started_.hour_     = save_handling::get_int();
    time_started_.minute_   = save_handling::get_int();
    time_started_.second_   = save_handling::get_int();

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
    return xp_;
}
Time_data    start_time()
{
    return time_started_;
}

int mon_tot_xp_worth(const Actor_data_t& d)
{
    //K regulates player xp rate, higher -> more xp per monster
    const double k              = 0.45;

    const double hp             = d.hp;

    const double speed          = double(d.speed);
    const double speed_max      = double(Actor_speed::END);
    const double speed_factor   = 1.0 + ((speed / speed_max) * 0.50);

    const double shock          = double(d.mon_shock_lvl);
    const double shock_max      = double(Mon_shock_lvl::END);
    const double shock_factor   = 1.0 + ((shock / shock_max) * 0.75);

    const double unique_factor  = d.is_unique ? 2.0 : 1.0;

    return ceil(k * hp * speed_factor * shock_factor * unique_factor);
}

void incr_player_xp(const int xp_gained)
{
    if (map::player->is_alive())
    {
        for (int i = 0; i < xp_gained; ++i)
        {
            ++xp_;

            if (clvl_ < player_max_clvl && xp_ >= xp_for_lvl_[clvl_ + 1])
            {
                player_gain_lvl();
            }
        }
    }
}

int xp_to_next_lvl()
{
    if (clvl_ == player_max_clvl)
    {
        return -1;
    }

    return xp_for_lvl_[clvl_ + 1] - xp_;
}

void player_lose_xp_percent(const int percent)
{
    xp_ = (xp_ * (100 - percent)) / 100;
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

    render::draw_text(cmd_label, Panel::screen,
                      P(X0, screen_h - 2),
                      clr_popup_label);

    render::update_screen();

    query::wait_for_confirm();
}

void on_mon_killed(Actor& actor)
{
    Actor_data_t& d = actor.data();

    d.nr_kills += 1;

    const int min_hp_for_sadism_bon = 4;

    if (
        d.hp >= min_hp_for_sadism_bon &&
        insanity::has_sympt(Ins_sympt_id::sadism))
    {
        map::player->shock_ = std::max(0.0, map::player->shock_ - 3.0);
    }

    Mon* const  mon = static_cast<Mon*>(&actor);

    if (!map::player->is_leader_of(mon))
    {
        const int mon_xp_tot    = mon_tot_xp_worth(d);
        const int xp_gained     = mon->has_given_xp_for_spotting_ ?
                                  std::max(1, mon_xp_tot / 2) : mon_xp_tot;
        incr_player_xp(xp_gained);
    }

    if (d.is_unique)
    {
        const std::string name = actor.name_the();

        add_history_event("Defeated " + name + ".");
    }
}

void on_mon_seen(Actor& actor)
{
    Mon* const mon = static_cast<Mon*>(&actor);

    if (!map::player->is_leader_of(mon) && !mon->has_given_xp_for_spotting_)
    {
        mon->has_given_xp_for_spotting_ = true;
        incr_player_xp(mon_tot_xp_worth(mon->data()) / 2);
    }
}

void set_time_started_to_now()
{
    time_started_ = current_time();
}

void add_history_event(const std::string msg)
{
    const int turn_nr = game_time::turn();

    history_events_.push_back({msg, turn_nr});
}

const std::vector<History_event>& history()
{
    return history_events_;
}

} //Dungeon_master
