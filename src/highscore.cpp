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

using namespace std;

High_score_entry::High_score_entry(std::string entry_date_and_time, std::string player_name,
                                   int player_xp, int player_lvl, int player_dlvl,
                                   int player_insanity, bool is_win_game, Bg player_bg) :
    date_and_time_  (entry_date_and_time),
    name_           (player_name),
    xp_             (player_xp),
    lvl_            (player_lvl),
    dlvl_           (player_dlvl),
    ins_            (player_insanity),
    is_win_         (is_win_game),
    bg_             (player_bg) {}

int High_score_entry::score() const
{
    const double DLVL_DB      = double(dlvl_);
    const double DLVL_LAST_DB = double(DLVL_LAST);
    const double XP_DB        = double(xp_);

    const double FACTOR_XP    = 1.0 + XP_DB + (is_win_ ? (XP_DB / 5.0) : 0.0);
    const double FACTOR_DLVL  = 1.0 + (DLVL_DB / DLVL_LAST_DB);

    return int(FACTOR_XP * FACTOR_DLVL);
}

namespace high_score
{

namespace
{

const int X_POS_DATE    = 0;
const int X_POS_NAME    = X_POS_DATE  + 19;
const int X_POS_LVL     = X_POS_NAME  + PLAYER_NAME_MAX_LEN + 2;
const int X_POS_DLVL    = X_POS_LVL   + 7;
const int X_POS_INS     = X_POS_DLVL  + 7;
const int X_POS_WIN     = X_POS_INS   + 10;
const int X_POS_SCORE   = X_POS_WIN   + 5;

void sort_entries(vector<High_score_entry>& entries)
{
    auto cmp = [](const High_score_entry & e1, const High_score_entry & e2)
    {
        return e1.score() > e2.score();
    };

    sort(entries.begin(), entries.end(), cmp);
}

void write_file(vector<High_score_entry>& entries)
{
    ofstream file;
    file.open("data/highscores", ios::trunc);

    for (const auto entry : entries)
    {
        const string WIN_STR = entry.is_win() ? "W" : "0";

        file << WIN_STR                 << endl;
        file << entry.date_and_time()   << endl;
        file << entry.name()            << endl;
        file << entry.xp()              << endl;
        file << entry.lvl()             << endl;
        file << entry.dlvl()            << endl;
        file << entry.insanity()        << endl;
        file << int(entry.bg())         << endl;
    }
}

void read_file(vector<High_score_entry>& entries)
{
    ifstream file;
    file.open("data/highscores");

    if (file.is_open())
    {
        string line = "";

        while (getline(file, line))
        {
            bool is_win                = line[0] == 'W';
            getline(file, line);
            const string date_and_time  = line;
            getline(file, line);
            const string name         = line;
            getline(file, line);
            const int XP              = to_int(line);
            getline(file, line);
            const int LVL             = to_int(line);
            getline(file, line);
            const int DLVL            = to_int(line);
            getline(file, line);
            const int INS             = to_int(line);
            getline(file, line);
            Bg bg                     = Bg(to_int(line));

            entries.push_back({date_and_time, name, XP, LVL, DLVL, INS, is_win, bg});
        }

        file.close();
    }
}

void draw(const vector<High_score_entry>& entries, const int TOP_ELEMENT)
{
    TRACE_FUNC_BEGIN;

    render::clear_screen();

    const Panel panel   = Panel::screen;
    const int   X_LABEL = 3;

    const string decoration_line(MAP_W, '-');

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

        const string date_and_time  = entry.date_and_time();
        const string name           = entry.name();
        const string lvl            = to_str(entry.lvl());
        const string dlvl           = to_str(entry.dlvl());
        const string ins            = to_str(entry.insanity());
        const string win            = entry.is_win() ? "Yes" : "No";
        const string score          = to_str(entry.score());

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

void run_high_score_screen()
{
    vector<High_score_entry> entries;
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
                top_nr = min(NR_LINES_TOT - MAX_NR_LINES_ON_SCR, top_nr);
            }
        }

        if (d.key == '8' || d.sdl_key == SDLK_UP || d.key == 'k')
        {
            top_nr = max(0, top_nr - LINE_JUMP);
        }

        if (d.sdl_key == SDLK_SPACE || d.sdl_key == SDLK_ESCAPE)
        {
            break;
        }
    }
}

void on_game_over(const bool IS_WIN)
{
    vector<High_score_entry> entries = entries_sorted();

    High_score_entry cur_player(
        utils::cur_time().time_str(Time_type::minute, true),
        map::player->name_a(),
        dungeon_master::xp(),
        dungeon_master::clvl(),
        map::dlvl,
        map::player->ins(),
        IS_WIN,
        player_bon::bg());

    entries.push_back(cur_player);

    sort_entries(entries);

    write_file(entries);
}

vector<High_score_entry> entries_sorted()
{
    vector<High_score_entry> entries;
    read_file(entries);

    if (!entries.empty())
    {
        sort_entries(entries);
    }

    return entries;
}

} //High_score
