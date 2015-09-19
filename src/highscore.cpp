#include "highscore.hpp"

#include <algorithm>
#include <iostream>
#include <fstream>
#include <vector>

#include "init.hpp"
#include "converters.hpp"
#include "highscore.hpp"
#include "actor_player.hpp"
#include "dungeon_master.hpp"
#include "map.hpp"
#include "popup.hpp"
#include "input.hpp"
#include "render.hpp"
#include "utils.hpp"

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
    const double DLVL_DB      = double(dlvl_);
    const double DLVL_LAST_DB = double(DLVL_LAST);
    const double XP_DB        = double(xp_);

    const double XP_FACTOR    = 1.0 + XP_DB + (is_win_ ? (XP_DB / 5.0) : 0.0);
    const double DLVL_FACTOR  = 1.0 + (DLVL_DB / DLVL_LAST_DB);

    return int(XP_FACTOR * DLVL_FACTOR);
}

namespace highscore
{

namespace
{

//Set at game over
Highscore_entry* final_score_ = nullptr;

const int X_POS_DATE    = 0;
const int X_POS_NAME    = X_POS_DATE  + 19;
const int X_POS_LVL     = X_POS_NAME  + PLAYER_NAME_MAX_LEN + 2;
const int X_POS_DLVL    = X_POS_LVL   + 7;
const int X_POS_INS     = X_POS_DLVL  + 7;
const int X_POS_WIN     = X_POS_INS   + 10;
const int X_POS_SCORE   = X_POS_WIN   + 5;

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
        const std::string WIN_STR = entry.is_win() ? "W" : "0";

        file << WIN_STR                 << std::endl;
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
            const int XP                    = to_int(line);
            getline(file, line);
            const int LVL                   = to_int(line);
            getline(file, line);
            const int DLVL                  = to_int(line);
            getline(file, line);
            const int INS                   = to_int(line);
            getline(file, line);
            Bg bg                           = Bg(to_int(line));

            entries.push_back({date_and_time, name, XP, LVL, DLVL, INS, is_win, bg});
        }

        file.close();
    }
}

void draw(const std::vector<Highscore_entry>& entries, const int TOP_ELEMENT)
{
    TRACE_FUNC_BEGIN;

    render::clear_screen();

    const Panel panel   = Panel::screen;
    const int   X_LABEL = 3;

    const std::string decoration_line(MAP_W, '-');

    render::draw_text(decoration_line, panel, Pos(0, 0), clr_gray);

    render::draw_text(" Displaying High Scores ", panel, Pos(X_LABEL, 0), clr_gray);

    render::draw_text(decoration_line, panel, Pos(0, SCREEN_H - 1), clr_gray);

    render::draw_text(info_scr_cmd_info, panel, Pos(X_LABEL, SCREEN_H - 1), clr_gray);

    int y_pos = 1;

    const Clr& label_clr = clr_white_high;

    render::draw_text("Ended",       panel, Pos(X_POS_DATE,    y_pos), label_clr);
    render::draw_text("Name",        panel, Pos(X_POS_NAME,    y_pos), label_clr);
    render::draw_text("Level",       panel, Pos(X_POS_LVL,     y_pos), label_clr);
    render::draw_text("Depth",       panel, Pos(X_POS_DLVL,    y_pos), label_clr);
    render::draw_text("Insanity",    panel, Pos(X_POS_INS,     y_pos), label_clr);
    render::draw_text("Win",         panel, Pos(X_POS_WIN,     y_pos), label_clr);
    render::draw_text("Score",       panel, Pos(X_POS_SCORE,   y_pos), label_clr);

    y_pos++;

    const int MAX_NR_LINES_ON_SCR = SCREEN_H - 3;

    for (
        int i = TOP_ELEMENT;
        i < int(entries.size()) && (i - TOP_ELEMENT) < MAX_NR_LINES_ON_SCR;
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

        render::draw_text(date_and_time,    panel, Pos(X_POS_DATE,    y_pos), clr);
        render::draw_text(name,             panel, Pos(X_POS_NAME,    y_pos), clr);
        render::draw_text(lvl,              panel, Pos(X_POS_LVL,     y_pos), clr);
        render::draw_text(dlvl,             panel, Pos(X_POS_DLVL,    y_pos), clr);
        render::draw_text(ins + "%",        panel, Pos(X_POS_INS,     y_pos), clr);
        render::draw_text(win,              panel, Pos(X_POS_WIN,     y_pos), clr);
        render::draw_text(score,            panel, Pos(X_POS_SCORE,   y_pos), clr);
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

    const int LINE_JUMP           = 3;
    const int NR_LINES_TOT        = entries.size();
    const int MAX_NR_LINES_ON_SCR = SCREEN_H - 3;

    //Read keys
    while (true)
    {
        draw(entries, top_nr);

        const Key_data& d = input::input();

        if (d.key == '2' || d.sdl_key == SDLK_DOWN || d.key == 'j')
        {
            top_nr += LINE_JUMP;

            if (NR_LINES_TOT <= MAX_NR_LINES_ON_SCR)
            {
                top_nr = 0;
            }
            else
            {
                top_nr = std::min(NR_LINES_TOT - MAX_NR_LINES_ON_SCR, top_nr);
            }
        }

        if (d.key == '8' || d.sdl_key == SDLK_UP || d.key == 'k')
        {
            top_nr = std::max(0, top_nr - LINE_JUMP);
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

void on_game_over(const bool IS_WIN)
{
    std::vector<Highscore_entry> entries = entries_sorted();

    final_score_ = new Highscore_entry(utils::cur_time().time_str(Time_type::minute, true),
                                       map::player->name_a(),
                                       dungeon_master::xp(),
                                       dungeon_master::clvl(),
                                       map::dlvl,
                                       map::player->ins(),
                                       IS_WIN,
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
