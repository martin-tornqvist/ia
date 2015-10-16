#ifndef HIGH_SCORE_H
#define HIGH_SCORE_H

#include <vector>
#include <string>

#include "cmn_data.hpp"
#include "player_bon.hpp"

class Highscore_entry
{
public:
    Highscore_entry(std::string entry_date_and_time,
                    std::string player_name,
                    int player_xp,
                    int player_lvl,
                    int player_dlvl,
                    int player_insanity,
                    bool is_win_game,
                    Bg player_bg);

    ~Highscore_entry() {}

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

void run_highscore_screen();

void on_game_over(const bool IS_WIN);

std::vector<Highscore_entry> entries_sorted();

const Highscore_entry* final_score();

} //highscore

#endif
