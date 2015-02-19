#include "Dungeon_master.h"

#include <vector>

#include "Highscore.h"
#include "Text_format.h"
#include "Render.h"
#include "Query.h"
#include "Actor_player.h"
#include "Character_lines.h"
#include "Log.h"
#include "Sdl_wrapper.h"
#include "Map.h"
#include "Utils.h"
#include "Create_character.h"
#include "Actor_mon.h"

using namespace std;

namespace Dungeon_master
{

namespace
{

int       xp_for_lvl_[PLAYER_MAX_CLVL + 1];
int       clvl_  = 0;
int       xp_    = 0;
Time_data  time_started_;

void player_gain_lvl()
{
    if (Map::player->is_alive())
    {
        clvl_++;

        Log::add_msg("Welcome to level " + to_str(clvl_) + "!", clr_green, false, true);

        Create_character::pick_new_trait(false);

        Map::player->restore_hp(999, false);
        Map::player->change_max_hp(HP_PER_LVL, true);
        Map::player->restore_spi(999, false);
        Map::player->change_max_spi(SPI_PER_LVL, true);
        Map::player->restore_shock(999, false);
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
}

void store_to_save_lines(vector<string>& lines)
{
    lines.push_back(to_str(clvl_));
    lines.push_back(to_str(xp_));
    lines.push_back(to_str(time_started_.year_));
    lines.push_back(to_str(time_started_.month_));
    lines.push_back(to_str(time_started_.day_));
    lines.push_back(to_str(time_started_.hour_));
    lines.push_back(to_str(time_started_.minute_));
    lines.push_back(to_str(time_started_.second_));
}

void setup_from_save_lines(vector<string>& lines)
{
    clvl_ = to_int(lines.front());
    lines.erase(begin(lines));
    xp_ = to_int(lines.front());
    lines.erase(begin(lines));
    time_started_.year_ = to_int(lines.front());
    lines.erase(begin(lines));
    time_started_.month_ = to_int(lines.front());
    lines.erase(begin(lines));
    time_started_.day_ = to_int(lines.front());
    lines.erase(begin(lines));
    time_started_.hour_ = to_int(lines.front());
    lines.erase(begin(lines));
    time_started_.minute_ = to_int(lines.front());
    lines.erase(begin(lines));
    time_started_.second_ = to_int(lines.front());
    lines.erase(begin(lines));
}

int         get_cLvl()       {return clvl_;}
int         get_xp()         {return xp_;}
Time_data    get_start_time()  {return time_started_;}

int get_mon_tot_xp_worth(const Actor_data_t& d)
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

void player_gain_xp(const int XP_GAINED)
{
    if (Map::player->is_alive())
    {
        for (int i = 0; i < XP_GAINED; ++i)
        {
            xp_++;
            if (clvl_ < PLAYER_MAX_CLVL)
            {
                if (xp_ >= xp_for_lvl_[clvl_ + 1])
                {
                    player_gain_lvl();
                }
            }
        }
    }
}

int get_xp_to_next_lvl()
{
    if (clvl_ == PLAYER_MAX_CLVL) {return -1;}
    return xp_for_lvl_[clvl_ + 1] - xp_;
}

void player_lose_xp_percent(const int PERCENT)
{
    xp_ = (xp_ * (100 - PERCENT)) / 100;
}

void win_game()
{
    High_score::on_game_over(true);

    Render::cover_panel(Panel::screen);
    Render::update_screen();

    const vector<string> win_msg =
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

    for (const string& section_msg : win_msg)
    {
        vector<string> section_lines;
        Text_format::line_to_lines(section_msg, MAX_W, section_lines);

        for (const string& line : section_lines)
        {
            Render::draw_text(line, Panel::screen, Pos(X0, y), clr_white, clr_black);

            Render::update_screen();
            Sdl_wrapper::sleep(LINE_DELAY);
            ++y;
        }
        ++y;
    }
    ++y;

    const string CMD_LABEL =
        "[space/esc/enter] to record high score and return to main menu";

    Render::draw_text_centered(CMD_LABEL, Panel::screen, Pos(MAP_W_HALF, y), clr_menu_medium,
                             clr_black, true);

    Render::update_screen();

    Query::wait_for_confirm();
}

void on_mon_killed(Actor& actor)
{
    Actor_data_t& d = actor.get_data();

    d.nr_kills += 1;

    if (d.hp >= 3 && Map::player->obsessions[int(Obsession::sadism)])
    {
        Map::player->shock_ = max(0.0, Map::player->shock_ - 3.0);
    }

    Mon* const  mon = static_cast<Mon*>(&actor);

    if (!Map::player->is_leader_of(mon))
    {
        const int MON_XP_TOT    = get_mon_tot_xp_worth(d);
        const int XP_GAINED     = mon->has_given_xp_for_spotting_ ?
                                  max(1, MON_XP_TOT / 2) : MON_XP_TOT;
        player_gain_xp(XP_GAINED);
    }
}

void on_mon_seen(Actor& actor)
{
    Mon* const mon = static_cast<Mon*>(&actor);

    if (!mon->has_given_xp_for_spotting_)
    {
        mon->has_given_xp_for_spotting_ = true;
        player_gain_xp(get_mon_tot_xp_worth(mon->get_data()) / 2);
    }
}

void set_time_started_to_now()
{
    time_started_ = Utils::get_cur_time();
}

} //Dungeon_master
