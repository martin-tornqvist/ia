#include "highscore.hpp"

#include <algorithm>
#include <iostream>
#include <fstream>
#include <vector>

#include "init.hpp"
#include "highscore.hpp"
#include "actor_player.hpp"
#include "game.hpp"
#include "map.hpp"
#include "popup.hpp"
#include "io.hpp"

// -----------------------------------------------------------------------------
// Highscore entry
// -----------------------------------------------------------------------------
HighscoreEntry::HighscoreEntry(std::string entry_date_and_time,
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

HighscoreEntry::~HighscoreEntry() {}

int HighscoreEntry::score() const
{
    const double dlvl_db = double(dlvl_);
    const double dlvl_last_db = double(dlvl_last);
    const double xp_db = double(xp_);

    const double xp_factor = 1.0 + xp_db + (is_win_ ? (xp_db / 5.0) : 0.0);
    const double dlvl_factor = 1.0 + (dlvl_db / dlvl_last_db);

    return (int)(xp_factor * dlvl_factor);
}

// -----------------------------------------------------------------------------
// Highscore
// -----------------------------------------------------------------------------
namespace highscore
{

namespace
{

// Set at game over
HighscoreEntry* final_score_ = nullptr;

void sort_entries(std::vector<HighscoreEntry>& entries)
{
    auto cmp = [](const HighscoreEntry & e1, const HighscoreEntry & e2)
    {
        return e1.score() > e2.score();
    };

    sort(entries.begin(), entries.end(), cmp);
}

void write_file(std::vector<HighscoreEntry>& entries)
{
    std::ofstream file;
    file.open("res/data/highscores", std::ios::trunc);

    for (const auto entry : entries)
    {
        const std::string win_str = entry.is_win() ? "W" : "0";

        file << win_str << std::endl;
        file << entry.date_and_time() << std::endl;
        file << entry.name() << std::endl;
        file << entry.xp() << std::endl;
        file << entry.lvl() << std::endl;
        file << entry.dlvl() << std::endl;
        file << entry.ins() << std::endl;
        file << int(entry.bg()) << std::endl;
    }
}

std::vector<HighscoreEntry> read_file()
{
    TRACE_FUNC_BEGIN;

    std::vector<HighscoreEntry> entries;

    std::ifstream file;
    file.open("res/data/highscores");

    if (!file.is_open())
    {
        return entries;
    }

    std::string line = "";

    while (getline(file, line))
    {
        bool is_win = line[0] == 'W';

        getline(file, line);
        const std::string date_and_time = line;

        getline(file, line);
        const std::string name = line;

        getline(file, line);
        const int xp = to_int(line);

        getline(file, line);
        const int lvl = to_int(line);

        getline(file, line);
        const int dlvl = to_int(line);

        getline(file, line);
        const int ins = to_int(line);

        getline(file, line);
        Bg bg = Bg(to_int(line));

        entries.push_back(
            HighscoreEntry(date_and_time,
                           name,
                           xp,
                           lvl,
                           dlvl,
                           ins,
                           is_win,
                           bg));
    }

    file.close();

    TRACE_FUNC_END;

    return entries;
}

} // namespace

void init()
{
    final_score_ = nullptr;
}

void cleanup()
{
    delete final_score_;

    final_score_ = nullptr;
}

const HighscoreEntry* final_score()
{
    return final_score_;
}

void on_game_over(const bool is_win)
{
    TRACE_FUNC_BEGIN;

    std::vector<HighscoreEntry> entries = entries_sorted();

    const auto time = current_time().time_str(TimeType::minute, true);

    final_score_ = new HighscoreEntry(time,
                                      map::player->name_a(),
                                      game::xp_accumulated(),
                                      game::clvl(),
                                      map::dlvl,
                                      map::player->ins(),
                                      is_win,
                                      player_bon::bg());

    entries.push_back(*final_score_);

    sort_entries(entries);

    write_file(entries);

    TRACE_FUNC_END;
}

std::vector<HighscoreEntry> entries_sorted()
{
    auto entries = read_file();

    if (!entries.empty())
    {
        sort_entries(entries);
    }

    return entries;
}

} // highscore

// -----------------------------------------------------------------------------
// Browse highscore
// -----------------------------------------------------------------------------
namespace
{

const int max_nr_lines_on_scr_ = screen_h - 3;

} // namespace

StateId BrowseHighscore::id()
{
    return StateId::highscore;
}

void BrowseHighscore::on_start()
{
    entries_ = highscore::read_file();

    highscore::sort_entries(entries_);
}

void BrowseHighscore::draw()
{
    if (entries_.empty())
    {
        return;
    }

    const int x_pos_date = 0;
    const int x_pos_name = x_pos_date + 19;
    const int x_pos_lvl = x_pos_name + player_name_max_len + 2;
    const int x_pos_dlvl = x_pos_lvl + 7;
    const int x_pos_ins = x_pos_dlvl + 7;
    const int x_pos_win = x_pos_ins + 10;
    const int x_pos_score = x_pos_win + 5;

    io::draw_info_scr_interface("High Scores",
                                InfScreenType::scrolling);

    int y_pos = 1;

    const Clr& label_clr = clr_white_lgt;

    const Panel panel = Panel::screen;

    io::draw_text("Ended", panel, P(x_pos_date, y_pos), label_clr);
    io::draw_text("Name", panel, P(x_pos_name, y_pos), label_clr);
    io::draw_text("Level", panel, P(x_pos_lvl, y_pos), label_clr);
    io::draw_text("Depth", panel, P(x_pos_dlvl, y_pos), label_clr);
    io::draw_text("Insanity", panel, P(x_pos_ins, y_pos), label_clr);
    io::draw_text("Win", panel, P(x_pos_win, y_pos), label_clr);
    io::draw_text("Score", panel, P(x_pos_score, y_pos), label_clr);

    ++y_pos;

    for (int i = top_idx_;
         (i < (int)entries_.size()) && ((i - top_idx_) < max_nr_lines_on_scr_);
         i++)
    {
        const auto& entry = entries_[i];

        const std::string date_and_time = entry.date_and_time();
        const std::string name = entry.name();
        const std::string lvl = std::to_string(entry.lvl());
        const std::string dlvl = std::to_string(entry.dlvl());
        const std::string ins = std::to_string(entry.ins());
        const std::string win = entry.is_win() ? "Yes" : "No";
        const std::string score = std::to_string(entry.score());

        const Clr& clr = clr_white;

        io::draw_text(date_and_time, panel, P(x_pos_date, y_pos), clr);
        io::draw_text(name, panel, P(x_pos_name, y_pos), clr);
        io::draw_text(lvl, panel, P(x_pos_lvl, y_pos), clr);
        io::draw_text(dlvl, panel, P(x_pos_dlvl, y_pos), clr);
        io::draw_text(ins + "%", panel, P(x_pos_ins, y_pos), clr);
        io::draw_text(win, panel, P(x_pos_win, y_pos), clr);
        io::draw_text(score, panel, P(x_pos_score, y_pos), clr);

        ++y_pos;
    }
}

void BrowseHighscore::update()
{
    if (entries_.empty())
    {
        popup::show_msg("No High Score entries found.");

        //
        // Exit screen
        //
        states::pop();

        return;
    }

    const int line_jump = 3;
    const int nr_lines_tot = entries_.size();

    const auto input = io::get(false);

    switch (input.key)
    {
    case '2':
    case SDLK_DOWN:
    case 'j':
        top_idx_ += line_jump;

        if (nr_lines_tot <= max_nr_lines_on_scr_)
        {
            top_idx_ = 0;
        }
        else
        {
            top_idx_ = std::min(nr_lines_tot - max_nr_lines_on_scr_, top_idx_);
        }
        break;

    case '8':
    case SDLK_UP:
    case 'k':
        top_idx_ = std::max(0, top_idx_ - line_jump);
        break;

    case SDLK_SPACE:
    case SDLK_ESCAPE:
        //
        // Exit screen
        //
        states::pop();
        break;
    }
}
