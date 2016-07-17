#include "highscore.hpp"

#include <algorithm>
#include <iostream>
#include <fstream>
#include <vector>

#include "init.hpp"
#include "highscore.hpp"
#include "actor_player.hpp"
#include "dungeon_master.hpp"
#include "map.hpp"
#include "popup.hpp"
#include "input.hpp"
#include "render.hpp"

Highscore_entry::Highscore_entry(std::string entry_date_and_time,
                                 std::string player_name,
                                 int player_xp,
                                 int player_lvl,
                                 int player_dlvl,
                                 int player_insanity,
                                 bool is_win_game,
                                 Bg player_bg) :
    date_and_time_  (entry_date_and_time),
    name_           (player_name),
    xp_             (player_xp),
    lvl_            (player_lvl),
    dlvl_           (player_dlvl),
    ins_            (player_insanity),
    is_win_         (is_win_game),
    bg_             (player_bg) {}

int Highscore_entry::score() const
{
    const double dlvl_db      = double(dlvl_);
    const double dlvl_last_db = double(dlvl_last);
    const double xp_db        = double(xp_);

    const double xp_factor    = 1.0 + xp_db + (is_win_ ? (xp_db / 5.0) : 0.0);
    const double dlvl_factor  = 1.0 + (dlvl_db / dlvl_last_db);

    return int(xp_factor * dlvl_factor);
}

namespace highscore
{

namespace
{

//Set at game over
Highscore_entry* final_score_ = nullptr;

const int x_pos_date    = 0;
const int x_pos_name    = x_pos_date  + 19;
const int x_pos_lvl     = x_pos_name  + player_name_max_len + 2;
const int x_pos_dlvl    = x_pos_lvl   + 7;
const int x_pos_ins     = x_pos_dlvl  + 7;
const int x_pos_win     = x_pos_ins   + 10;
const int x_pos_score   = x_pos_win   + 5;

void sort_entries(std::vector<Highscore_entry>& entries)
{
    auto cmp = [](const Highscore_entry & e1, const Highscore_entry & e2)
    {
        return e1.score() > e2.score();
    };

    sort(entries.begin(), entries.end(), cmp);
}

void write_file(std::vector<Highscore_entry>& entries)
{
    std::ofstream file;
    file.open("data/highscores", std::ios::trunc);

    for (const auto entry : entries)
    {
        const std::string win_str = entry.is_win() ? "W" : "0";

        file << win_str                 << std::endl;
        file << entry.date_and_time()   << std::endl;
        file << entry.name()            << std::endl;
        file << entry.xp()              << std::endl;
        file << entry.lvl()             << std::endl;
        file << entry.dlvl()            << std::endl;
        file << entry.ins()             << std::endl;
        file << int(entry.bg())         << std::endl;
    }
}

void read_file(std::vector<Highscore_entry>& entries)
{
    std::ifstream file;
    file.open("data/highscores");

    if (file.is_open())
    {
        std::string line = "";

        while (getline(file, line))
        {
            bool is_win                     = line[0] == 'W';
            getline(file, line);
            const std::string date_and_time = line;
            getline(file, line);
            const std::string name          = line;
            getline(file, line);
            const int xp                    = to_int(line);
            getline(file, line);
            const int lvl                   = to_int(line);
            getline(file, line);
            const int dlvl                  = to_int(line);
            getline(file, line);
            const int ins                   = to_int(line);
            getline(file, line);
            Bg bg                           = Bg(to_int(line));

            entries.push_back({date_and_time, name, xp, lvl, dlvl, ins, is_win, bg});
        }

        file.close();
    }
}

void draw(const std::vector<Highscore_entry>& entries, const int top_element)
{
    TRACE_FUNC_BEGIN;

    render::clear_screen();

    render::draw_info_scr_interface("High Scores",
                                    Inf_screen_type::scrolling);

    int y_pos = 1;

    const Clr& label_clr = clr_white_high;

    const Panel panel = Panel::screen;

    render::draw_text("Ended",       panel, P(x_pos_date,    y_pos), label_clr);
    render::draw_text("Name",        panel, P(x_pos_name,    y_pos), label_clr);
    render::draw_text("Level",       panel, P(x_pos_lvl,     y_pos), label_clr);
    render::draw_text("Depth",       panel, P(x_pos_dlvl,    y_pos), label_clr);
    render::draw_text("Insanity",    panel, P(x_pos_ins,     y_pos), label_clr);
    render::draw_text("Win",         panel, P(x_pos_win,     y_pos), label_clr);
    render::draw_text("Score",       panel, P(x_pos_score,   y_pos), label_clr);

    y_pos++;

    const int max_nr_lines_on_scr = screen_h - 3;

    for (
        int i = top_element;
        i < int(entries.size()) && (i - top_element) < max_nr_lines_on_scr;
        i++)
    {
        const auto entry = entries[i];

        const std::string date_and_time  = entry.date_and_time();
        const std::string name           = entry.name();
        const std::string lvl            = to_str(entry.lvl());
        const std::string dlvl           = to_str(entry.dlvl());
        const std::string ins            = to_str(entry.ins());
        const std::string win            = entry.is_win() ? "Yes" : "No";
        const std::string score          = to_str(entry.score());

        const Clr& clr = clr_white;

        render::draw_text(date_and_time,    panel, P(x_pos_date,    y_pos), clr);
        render::draw_text(name,             panel, P(x_pos_name,    y_pos), clr);
        render::draw_text(lvl,              panel, P(x_pos_lvl,     y_pos), clr);
        render::draw_text(dlvl,             panel, P(x_pos_dlvl,    y_pos), clr);
        render::draw_text(ins + "%",        panel, P(x_pos_ins,     y_pos), clr);
        render::draw_text(win,              panel, P(x_pos_win,     y_pos), clr);
        render::draw_text(score,            panel, P(x_pos_score,   y_pos), clr);
        y_pos++;
    }

    render::update_screen();

    TRACE_FUNC_END;
}

} //namespace

void init()
{
    final_score_ = nullptr;
}

void cleanup()
{
    delete final_score_;

    final_score_ = nullptr;
}

void run_highscore_screen()
{
    std::vector<Highscore_entry> entries;
    read_file(entries);

    if (entries.empty())
    {
        popup::show_msg("No High Score entries found.", false);
        return;
    }

    sort_entries(entries);

    int top_nr = 0;
    draw(entries, top_nr);

    const int line_jump           = 3;
    const int nr_lines_tot        = entries.size();
    const int max_nr_lines_on_scr = screen_h - 3;

    //Read keys
    while (true)
    {
        draw(entries, top_nr);

        const Key_data& d = input::input();

        if (d.key == '2' || d.sdl_key == SDLK_DOWN || d.key == 'j')
        {
            top_nr += line_jump;

            if (nr_lines_tot <= max_nr_lines_on_scr)
            {
                top_nr = 0;
            }
            else
            {
                top_nr = std::min(nr_lines_tot - max_nr_lines_on_scr, top_nr);
            }
        }

        if (d.key == '8' || d.sdl_key == SDLK_UP || d.key == 'k')
        {
            top_nr = std::max(0, top_nr - line_jump);
        }

        if (d.sdl_key == SDLK_SPACE || d.sdl_key == SDLK_ESCAPE)
        {
            break;
        }
    }
}

const Highscore_entry* final_score()
{
    return final_score_;
}

void on_game_over(const bool is_win)
{
    std::vector<Highscore_entry> entries = entries_sorted();

    final_score_ = new Highscore_entry(current_time().time_str(Time_type::minute, true),
                                       map::player->name_a(),
                                       dungeon_master::xp(),
                                       dungeon_master::clvl(),
                                       map::dlvl,
                                       map::player->ins(),
                                       is_win,
                                       player_bon::bg());

    entries.push_back(*final_score_);

    sort_entries(entries);

    write_file(entries);
}

std::vector<Highscore_entry> entries_sorted()
{
    std::vector<Highscore_entry> entries;
    read_file(entries);

    if (!entries.empty())
    {
        sort_entries(entries);
    }

    return entries;
}

} //highscore
