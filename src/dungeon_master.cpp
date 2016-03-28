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

int         xp_for_lvl_[PLAYER_MAX_CLVL + 1];
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

        render::draw_map_and_interface();

        map::player->change_max_hp(HP_PER_LVL);
        map::player->restore_hp(999, false, Verbosity::silent);

        map::player->change_max_spi(SPI_PER_LVL);
        map::player->restore_spi(999, false, Verbosity::silent);

        map::player->restore_shock(999, false);
    }
}

void init_xp_array()
{
    xp_for_lvl_[0] = 0;
    xp_for_lvl_[1] = 0;

    for (int lvl = 2; lvl <= PLAYER_MAX_CLVL; lvl++)
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
        save_handling::put_int(event.TURN);
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

    const int NR_EVENTS = save_handling::get_int();

    for (int i = 0; i < NR_EVENTS; ++i)
    {
        const std::string   msg     = save_handling::get_str();
        const int           TURN    = save_handling::get_int();

        history_events_.push_back({msg, TURN});
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
    //K regulates player XP rate, higher -> more XP per monster
    const double K              = 0.45;

    const double HP             = d.hp;

    const double SPEED          = double(d.speed);
    const double SPEED_MAX      = double(Actor_speed::END);
    const double SPEED_FACTOR   = 1.0 + ((SPEED / SPEED_MAX) * 0.50);

    const double SHOCK          = double(d.mon_shock_lvl);
    const double SHOCK_MAX      = double(Mon_shock_lvl::END);
    const double SHOCK_FACTOR   = 1.0 + ((SHOCK / SHOCK_MAX) * 0.75);

    const double UNIQUE_FACTOR  = d.is_unique ? 2.0 : 1.0;

    return ceil(K * HP * SPEED_FACTOR * SHOCK_FACTOR * UNIQUE_FACTOR);
}

void incr_player_xp(const int XP_GAINED)
{
    if (map::player->is_alive())
    {
        for (int i = 0; i < XP_GAINED; ++i)
        {
            ++xp_;

            if (clvl_ < PLAYER_MAX_CLVL && xp_ >= xp_for_lvl_[clvl_ + 1])
            {
                player_gain_lvl();
            }
        }
    }
}

int xp_to_next_lvl()
{
    if (clvl_ == PLAYER_MAX_CLVL)
    {
        return -1;
    }

    return xp_for_lvl_[clvl_ + 1] - xp_;
}

void player_lose_xp_percent(const int PERCENT)
{
    xp_ = (xp_ * (100 - PERCENT)) / 100;
}

void win_game()
{
    highscore::on_game_over(true);

    render::cover_panel(Panel::screen);
    render::update_screen();

    const std::vector<std::string> win_msg =
    {
        "As I touch the crystal, there is a jolt of electricity. A surreal glow "
        "suddenly illuminates the area. I feel as if I have stirred something. I notice "
        "a figure observing me from the edge of the light. It approaches me. There is "
        "no doubt in my mind concerning the nature of this entity; it is the Faceless "
        "God who dwells in the depths of the earth, it is the Crawling Chaos - "
        "Nyarlathotep! I panic for a moment. Why is it I find myself here, stumbling "
        "around in darkness? But I soon sense that this is all part of a plan, that I "
        "have proven myself worthy for something. The being beckons me to gaze into the "
        "stone.",

        "In the radiance I see visions beyond eternity, visions of unreal "
        "reality, visions of the brightest light of day and the darkest night of "
        "madness. There is only onward now, I demand to attain everything! So I make a "
        "pact with the Fiend.",

        "I now harness the shadows that stride from world to world to sow death and "
        "madness. The destinies of all things on earth, living and dead, are mine."
    };

    const int X0            = 6;
    const int MAX_W         = MAP_W - (X0 * 2);

    const int LINE_DELAY    = 40;

    int y = 2;

    for (const std::string& section_msg : win_msg)
    {
        std::vector<std::string> section_lines;
        text_format::split(section_msg, MAX_W, section_lines);

        for (const std::string& line : section_lines)
        {
            render::draw_text(line, Panel::screen, P(X0, y), clr_white, clr_black);

            render::update_screen();
            sdl_wrapper::sleep(LINE_DELAY);
            ++y;
        }

        ++y;
    }

    ++y;

    const std::string CMD_LABEL =
        "[space/esc/enter] to record high score and return to main menu";

    render::draw_text_center(CMD_LABEL, Panel::screen, P(MAP_W_HALF, y), clr_menu_medium,
                             clr_black, true);

    render::update_screen();

    query::wait_for_confirm();
}

void on_mon_killed(Actor& actor)
{
    Actor_data_t& d = actor.data();

    d.nr_kills += 1;

    const int MIN_HP_FOR_SADISM_BON = 4;

    if (d.hp >= MIN_HP_FOR_SADISM_BON && insanity::has_sympt(Ins_sympt_id::sadism))
    {
        map::player->shock_ = std::max(0.0, map::player->shock_ - 3.0);
    }

    Mon* const  mon = static_cast<Mon*>(&actor);

    if (!map::player->is_leader_of(mon))
    {
        const int MON_XP_TOT    = mon_tot_xp_worth(d);
        const int XP_GAINED     = mon->has_given_xp_for_spotting_ ?
                                  std::max(1, MON_XP_TOT / 2) : MON_XP_TOT;
        incr_player_xp(XP_GAINED);
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
    time_started_ = cur_time();
}

void add_history_event(const std::string msg)
{
    const int TURN_NR = game_time::turn();

    history_events_.push_back({msg, TURN_NR});
}

const std::vector<History_event>& history()
{
    return history_events_;
}

} //Dungeon_master
