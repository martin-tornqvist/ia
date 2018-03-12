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
#include "panel.hpp"

// -----------------------------------------------------------------------------
// Highscore entry
// -----------------------------------------------------------------------------
HighscoreEntry::HighscoreEntry(std::string game_summary_file_path,
                               std::string date,
                               std::string player_name,
                               int player_xp,
                               int player_lvl,
                               int player_dlvl,
                               int turn_count,
                               int player_insanity,
                               IsWin is_win,
                               Bg player_bg) :
        game_summary_file_path_(game_summary_file_path),
        date_(date),
        name_(player_name),
        xp_(player_xp),
        lvl_(player_lvl),
        dlvl_(player_dlvl),
        turn_count_(turn_count),
        ins_(player_insanity),
        is_win_(is_win),
        bg_(player_bg) {}

HighscoreEntry::~HighscoreEntry() {}

int HighscoreEntry::score() const
{
        const double xp_db = (double)xp_;
        const double dlvl_db = (double)dlvl_;
        const double dlvl_last_db = (double)dlvl_last;
        const double turns_db = (double)turn_count_;
        const double ins_db = (double)ins_;
        const bool win = (is_win_ == IsWin::yes);

        auto calc_turns_factor = [](const double turns_db) {
                return std::max(1.0, 3.0 - (turns_db / 10000.0));
        };

        auto calc_sanity_factor = [](const double ins_db) {
                return 2.0 - (ins_db / 100.0);
        };

        const double xp_factor = 1.0 + xp_db;

        const double dlvl_factor = 1.0 + (dlvl_db / dlvl_last_db);

        const double turns_factor = calc_turns_factor(turns_db);

        const double turns_factor_win = win ? calc_turns_factor(0.0) : 1.0;

        const double sanity_factor = calc_sanity_factor(ins_db);

        const double sanity_factor_win = win ? calc_sanity_factor(0.0) : 1.0;

        return (int)(
                xp_factor *
                dlvl_factor *
                turns_factor *
                turns_factor_win *
                sanity_factor *
                sanity_factor_win);
}

// -----------------------------------------------------------------------------
// Highscore
// -----------------------------------------------------------------------------
namespace highscore
{

namespace
{

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
                const std::string win_str =
                        (entry.is_win() == IsWin::yes) ?
                        "1" :
                        "0";

                file << entry.game_summary_file_path() << std::endl;
                file << win_str << std::endl;
                file << entry.date() << std::endl;
                file << entry.name() << std::endl;
                file << entry.xp() << std::endl;
                file << entry.lvl() << std::endl;
                file << entry.dlvl() << std::endl;
                file << entry.turn_count() << std::endl;
                file << entry.ins() << std::endl;
                file << (int)entry.bg() << std::endl;
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
                const std::string game_summary_file = line;

                getline(file, line);

                IsWin is_win =
                        (line[0] == '1') ?
                        IsWin::yes :
                        IsWin::no;

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
                const int turn_count = to_int(line);

                getline(file, line);
                const int ins = to_int(line);

                getline(file, line);
                Bg bg = (Bg)to_int(line);

                entries.push_back(
                        HighscoreEntry(
                                game_summary_file,
                                date_and_time,
                                name,
                                xp,
                                lvl,
                                dlvl,
                                turn_count,
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

}

void cleanup()
{

}

HighscoreEntry make_entry_from_current_game_data(
        const std::string game_summary_file_path,
        const IsWin is_win)
{
        const auto date = current_time().time_str(TimeType::day, true);

        HighscoreEntry entry(
                game_summary_file_path,
                date,
                map::player->name_a(),
                game::xp_accumulated(),
                game::clvl(),
                map::dlvl,
                game_time::turn_nr(),
                map::player->ins(),
                is_win,
                player_bon::bg());

        return entry;
}

void append_entry_to_highscores_file(const HighscoreEntry& entry)
{
        TRACE_FUNC_BEGIN;

        std::vector<HighscoreEntry> entries = entries_sorted();

        entries.push_back(entry);

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

const int top_more_y_ = 1;

const int entries_y0_ = top_more_y_ + 1;

int get_bottom_more_y()
{
        return panels::get_h(Panel::screen) - 1;
}

int get_max_nr_entries_on_screen()
{
        return get_bottom_more_y() - entries_y0_;
}

} // namespace

StateId BrowseHighscore::id()
{
        return StateId::highscore;
}

void BrowseHighscore::on_start()
{
        entries_ = highscore::read_file();

        highscore::sort_entries(entries_);

        browser_.reset(entries_.size(), get_max_nr_entries_on_screen());
}

void BrowseHighscore::draw()
{
        if (entries_.empty())
        {
                return;
        }

        const Panel panel = Panel::screen;

        const std::string title =
                "Browsing high scores [select] to view game summary";

        io::draw_text_center(
                title,
                panel,
                P(map_w_half, 0),
                colors::title(),
                colors::black(),
                true);

        const Color& label_clr = colors::white();

        const int labels_y = 1;

        const int x_date = 0;
        const int x_name = x_date + 12;
        const int x_bg = x_name + player_name_max_len + 1;
        const int x_lvl = x_bg + 13;
        const int x_dlvl = x_lvl + 7;
        const int x_turns = x_dlvl + 7;
        const int x_ins = x_turns + 7;
        const int x_win = x_ins + 6;
        const int x_score = x_win + 5;

        const std::vector< std::pair<std::string, int> > labels
        {
                {"Level", x_lvl},
                {"Depth", x_dlvl},
                {"Turns", x_turns},
                {"Ins", x_ins},
                {"Win", x_win},
                {"Score", x_score}
        };

        for (const auto& label : labels)
        {
                io::draw_text(label.first,
                              panel,
                              P(label.second, labels_y),
                              label_clr);
        }

        const int browser_y = browser_.y();

        int y = entries_y0_;

        const Range idx_range_shown = browser_.range_shown();

        for (int i = idx_range_shown.min; i <= idx_range_shown.max; ++i)
        {
                const auto& entry = entries_[i];

                const std::string date = entry.date();
                const std::string name = entry.name();
                const std::string bg = player_bon::bg_title(entry.bg());
                const std::string lvl = std::to_string(entry.lvl());
                const std::string dlvl = std::to_string(entry.dlvl());
                const std::string turns = std::to_string(entry.turn_count());
                const std::string ins = std::to_string(entry.ins());
                const std::string win =
                        (entry.is_win() == IsWin::yes) ?
                        "Yes" :
                        "No";
                const std::string score = std::to_string(entry.score());

                const bool is_idx_marked = browser_y == i;

                const Color& color =
                        is_idx_marked ?
                        colors::menu_highlight():
                        colors::menu_dark();

                io::draw_text(date, panel, P(x_date, y), color);
                io::draw_text(name, panel, P(x_name, y), color);
                io::draw_text(bg, panel, P(x_bg, y), color);
                io::draw_text(lvl, panel, P(x_lvl, y), color);
                io::draw_text(dlvl, panel, P(x_dlvl, y), color);
                io::draw_text(turns, panel, P(x_turns, y), color);
                io::draw_text(ins + "%", panel, P(x_ins, y), color);
                io::draw_text(win, panel, P(x_win, y), color);
                io::draw_text(score, panel, P(x_score, y), color);

                ++y;
        }

        // Draw "more" labels
        if (!browser_.is_on_top_page())
        {
                io::draw_text("(More - Page Up)",
                              Panel::screen,
                              P(0, top_more_y_),
                              colors::light_white());
        }

        if (!browser_.is_on_btm_page())
        {
                io::draw_text("(More - Page Down)",
                              Panel::screen,
                              P(0, get_bottom_more_y()),
                              colors::light_white());
        }
}

void BrowseHighscore::update()
{
        if (entries_.empty())
        {
                popup::show_msg("No high score entries found.");

                //
                // Exit screen
                //
                states::pop();

                return;
        }

        const auto input = io::get(false);

        const MenuAction action =
                browser_.read(input,
                              MenuInputMode::scrolling);

        switch (action)
        {
        case MenuAction::selected:
        case MenuAction::selected_shift:
        {
                const int browser_y = browser_.y();

                ASSERT(browser_y < (int)entries_.size());

                const auto& entry_marked = entries_[browser_y];

                const std::string file_path =
                        entry_marked.game_summary_file_path();

                auto state = std::unique_ptr<BrowseHighscoreEntry>(
                        new BrowseHighscoreEntry(file_path));

                states::push(std::move(state));
        }
        break;

        case MenuAction::space:
        case MenuAction::esc:
        {
                // Exit screen
                states::pop();
        }
        break;

        default:
                break;
        }
}

// -----------------------------------------------------------------------------
// Browse highscore entry game summary file
// -----------------------------------------------------------------------------
BrowseHighscoreEntry::BrowseHighscoreEntry(const std::string& file_path) :
        InfoScreenState(),
        file_path_(file_path),
        lines_(),
        top_idx_(0) {}

StateId BrowseHighscoreEntry::id()
{
        return StateId::browse_highscore_entry;
}

void BrowseHighscoreEntry::on_start()
{
        read_file();
}

void BrowseHighscoreEntry::draw()
{
        draw_interface();

        const int nr_lines_tot = lines_.size();

        int btm_nr =
                std::min(top_idx_ + max_nr_lines_on_screen() - 1,
                         nr_lines_tot - 1);

        int screen_y = 1;

        for (int i = top_idx_; i <= btm_nr; ++i)
        {
                io::draw_text(lines_[i],
                              Panel::screen,
                              P(0, screen_y),
                              colors::text());

                ++screen_y;
        }
}

void BrowseHighscoreEntry::update()
{
        const int line_jump = 3;

        const int nr_lines_tot = lines_.size();

        const auto input = io::get(false);

        switch (input.key)
        {
        case '2':
        case SDLK_DOWN:
        case 'j':
                top_idx_ += line_jump;

                if (nr_lines_tot <= max_nr_lines_on_screen())
                {
                        top_idx_ = 0;
                }
                else
                {
                        top_idx_ = std::min(
                                nr_lines_tot - max_nr_lines_on_screen(),
                                top_idx_);
                }
                break;

        case '8':
        case SDLK_UP:
        case 'k':
                top_idx_ = std::max(0, top_idx_ - line_jump);
                break;

        case SDLK_SPACE:
        case SDLK_ESCAPE:
                // Exit screen
                states::pop();
                break;

        default:
                break;
        }
}

void BrowseHighscoreEntry::read_file()
{
        lines_.clear();

        std::ifstream file(file_path_);

        if (!file.is_open())
        {
                popup::show_msg(
                        "Path: \"" + file_path_ + "\"",
                        "Game summary file could not be opened",
                        SfxId::END,
                        20);

                states::pop();

                return;
        }

        std::string current_line;

        std::vector<std::string> formatted;

        while (getline(file, current_line))
        {
                lines_.push_back(current_line);
        }

        file.close();
}
