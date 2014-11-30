#ifndef HIGH_SCORE_H
#define HIGH_SCORE_H

#include <vector>
#include <string>

#include "CmnData.h"

class HighScoreEntry
{
public:
  HighScoreEntry(std::string dateAndTime, std::string name, int xp, int lvl, int dlvl,
                 int insanity, bool isWin);

  ~HighScoreEntry() {}

  void set(const std::string& dateAndTime, const std::string& name,
           const int XP, const int LVL, const int DLVL, const int INSANITY,
           const bool IS_VICTORY)
  {
    dateAndTime_  = dateAndTime;
    name_         = name;
    xp_           = XP;
    lvl_          = LVL;
    dlvl_         = DLVL;
    ins_          = INSANITY;
    isWin_        = IS_VICTORY;
  }

  std::string  getDateAndTime() const {return dateAndTime_;}
  std::string  getName()        const {return name_;}
  int          getXp()          const {return xp_;}
  int          getLvl()         const {return lvl_;}
  int          getDlvl()        const {return dlvl_;}
  int          getInsanity()    const {return ins_;}
  bool         isWin()          const {return isWin_;}

private:
  std::string dateAndTime_, name_;
  int xp_, lvl_, dlvl_, ins_;
  bool isWin_;
};

namespace HighScore
{

void runHighScoreScreen();

void onGameOver(const bool IS_VICTORY);

std::vector<HighScoreEntry> getEntriesSorted();

} //HighScore


#endif
