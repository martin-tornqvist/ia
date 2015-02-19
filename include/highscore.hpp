#ifndef HIGH_SCORE_H
#define HIGH_SCORE_H

#include <vector>
#include <string>

#include "cmn_data.hpp"
#include "player_bon.hpp"

class High_score_entry
{
public:
    High_score_entry(std::string date_and_time, std::string name, int xp, int lvl, int dlvl,
                     int insanity, bool is_win, Bg bg);

    ~High_score_entry() {}

    int          get_score()       const;
    std::string  get_date_and_time() const {return date_and_time_;}
    std::string  get_name()        const {return name_;}
    int          get_xp()          const {return xp_;}
    int          get_lvl()         const {return lvl_;}
    int          get_dlvl()        const {return dlvl_;}
    int          get_insanity()    const {return ins_;}
    bool         is_win()          const {return is_win_;}
    Bg           get_bg()          const {return bg_;}

private:
    std::string date_and_time_, name_;
    int xp_, lvl_, dlvl_, ins_;
    bool is_win_;
    Bg bg_;
};

namespace high_score
{

void run_high_score_screen();

void on_game_over(const bool IS_WIN);

std::vector<High_score_entry> get_entries_sorted();

} //High_score


#endif
