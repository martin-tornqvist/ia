#ifndef HIGH_SCORE_H
#define HIGH_SCORE_H

#include <vector>
#include <string>

#include "CmnData.h"
#include "PlayerBon.h"

class HighScoreEntry
{
public:
    HighScoreEntry(std::string dateAndTime, std::string name, int xp, int lvl, int dlvl,
                   int insanity, bool isWin, Bg bg);

    ~HighScoreEntry() {}

    int          getScore()       const;
    std::string  getDateAndTime() const {return dateAndTime_;}
    std::string  getName()        const {return name_;}
    int          getXp()          const {return xp_;}
    int          getLvl()         const {return lvl_;}
    int          getDlvl()        const {return dlvl_;}
    int          getInsanity()    const {return ins_;}
    bool         isWin()          const {return isWin_;}
    Bg           getBg()          const {return bg_;}

private:
    std::string dateAndTime_, name_;
    int xp_, lvl_, dlvl_, ins_;
    bool isWin_;
    Bg bg_;
};

namespace HighScore
{

void runHighScoreScreen();

void onGameOver(const bool IS_WIN);

std::vector<HighScoreEntry> getEntriesSorted();

} //HighScore


#endif
