#ifndef HIGHSCORE_HPP
#define HIGHSCORE_HPP

#include <vector>
#include <string>

#include "state.hpp"
#include "player_bon.hpp"

class HighscoreEntry
{
public:
    HighscoreEntry(std::string entry_date_and_time,
                   std::string player_name,
                   int player_xp,
                   int player_lvl,
                   int player_dlvl,
                   int player_insanity,
                   bool is_win_game,
                   Bg player_bg);

    ~HighscoreEntry() {}

    int score() const;

    std::string date_and_time() const
    {
        return date_and_time_;
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

    bool is_win() const
    {
        return is_win_;
    }

    Bg bg() const
    {
        return bg_;
    }

private:
    std::string date_and_time_, name_;
    int xp_, lvl_, dlvl_, ins_;
    bool is_win_;
    Bg bg_;
};

namespace highscore
{

void init();
void cleanup();

void on_game_over(const bool is_win);

std::vector<HighscoreEntry> entries_sorted();

const HighscoreEntry* final_score();

} // highscore

class BrowseHighscore: public State
{
public:
    BrowseHighscore() :
        State       (),
        entries_    (),
        top_idx_    (0) {}

    void on_start() override;

    void draw() override;

    void update() override;

    bool draw_overlayed() const override
    {
        // If there are no entries, we draw an overlayed popup
        return entries_.empty();
    }

private:
    std::vector<HighscoreEntry> entries_;

    int top_idx_;
};

#endif // HIGHSCORE_HPP
