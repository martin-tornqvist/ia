#ifndef HIGH_SCORE_H
#define HIGH_SCORE_H

#include <vector>
#include <string>

#include "CmnData.h"

struct HighScoreEntry {
public:
  HighScoreEntry(std::string dateAndTime, std::string name, int xp, int lvl,
                 int dlvl, int insanity, bool isVictory) :
    dateAndTime_(dateAndTime),
    name_(name),
    xp_(xp),
    lvl_(lvl),
    dlvl_(dlvl),
    ins_(insanity),
    isVictory_(isVictory) {}

  ~HighScoreEntry() {}

  void set(const std::string& dateAndTime, const std::string& name,
           const int XP, const int LVL, const int DLVL, const int INSANITY,
           const bool IS_VICTORY) {
    dateAndTime_  = dateAndTime;
    name_         = name;
    xp_           = XP;
    lvl_          = LVL;
    dlvl_         = DLVL;
    ins_          = INSANITY;
    isVictory_    = IS_VICTORY;
  }

  inline int getScore() const {
    const double DLVL_DB  = double(dlvl_);
    const double XP_DB    = double(xp_);
    const double INS_DB   = double(ins_);
    const double SCORE_DB =
      (XP_DB + (isVictory_ * XP_DB / 5.0)) *
      ((((DLVL_DB + 1.0) / LAST_CAVERN_LVL) / 3.0) + 1.0) *
      ((INS_DB / 3.0) + 1.0);
    return int(SCORE_DB);
  }

  inline std::string  getDateAndTime()  const {return dateAndTime_;}
  inline std::string  getName()         const {return name_;}
  inline int          getXp()           const {return xp_;}
  inline int          getLvl()          const {return lvl_;}
  inline int          getDlvl()         const {return dlvl_;}
  inline int          getInsanity()     const {return ins_;}
  inline bool         isVictoryGame()   const {return isVictory_;}

private:
  std::string dateAndTime_;
  std::string name_;
  int xp_, lvl_, dlvl_, ins_;
  bool isVictory_;
};

namespace HighScore {

void runHighScoreScreen();

void onGameOver(const bool IS_VICTORY);

std::vector<HighScoreEntry> getEntriesSorted();

} //HighScore


#endif
