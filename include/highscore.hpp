#ifndef HIGHSCORE_HPP
#define HIGHSCORE_HPP

#include <vector>
#include <string>

#include "state.hpp"
#include "player_bon.hpp"
#include "global.hpp"
#include "browser.hpp"

class HighscoreEntry
{
public:
    HighscoreEntry(std::string game_summary_file_path,
                   std::string entry_date,
                   std::string player_name,
                   int player_xp,
                   int player_lvl,
                   int player_dlvl,
                   int player_insanity,
                   IsWin is_win,
                   Bg player_bg);

    ~HighscoreEntry();

    int score() const;

    std::string game_summary_file_path() const
    {
        return game_summary_file_path_;
    }

    std::string date() const
    {
        return date_;
    }

    std::string name() const
    {
        return name_;
    }

    int xp() const
    {
        return xp_;
    }

    int lvl() const
    {
        return lvl_;
    }

    int dlvl() const
    {
        return dlvl_;
    }

    int ins() const
    {
        return ins_;
    }

    IsWin is_win() const
    {
        return is_win_;
    }

    Bg bg() const
    {
        return bg_;
    }

private:
    std::string game_summary_file_path_;
    std::string date_;
    std::string name_;
    int xp_;
    int lvl_;
    int dlvl_;
    int ins_;
    IsWin is_win_;
    Bg bg_;
};

namespace highscore
{

void init();
void cleanup();

//
// NOTE: All this does is construct a HighscoreEntry object, populated with
//       highscore info based on the current game - it has no side effects
//
HighscoreEntry mk_entry_from_current_game_data(
    const std::string game_summary_file_path,
    const IsWin is_win);

void append_entry_to_highscores_file(const HighscoreEntry& entry);

std::vector<HighscoreEntry> entries_sorted();

} // highscore

class BrowseHighscore: public State
{
public:
    BrowseHighscore();

    void on_start() override;

    void draw() override;

    void update() override;

    bool draw_overlayed() const override
    {
        // If there are no entries, we draw an overlayed popup
        return entries_.empty();
    }

    StateId id() override;

private:
    std::vector<HighscoreEntry> entries_;

    MenuBrowser browser_;
};

class BrowseHighscoreEntry: public State
{
public:
    BrowseHighscoreEntry(const std::string& file_path);

    void on_start() override;

    void draw() override;

    void update() override;

    StateId id() override;

private:
    void read_file();

    const std::string file_path_;

    std::vector<std::string> lines_;

    int top_idx_;
};

#endif // HIGHSCORE_HPP
