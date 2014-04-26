#ifndef HIGH_SCORE_H
#define HIGH_SCORE_H

#include <vector>
#include <string>

#include "CmnData.h"

using namespace std;

struct HighScoreEntry {
public:
  HighScoreEntry(string dateAndTime, string name, int xp, int lvl, int dlvl,
                 int insanity, bool isVictory) :
    dateAndTime_(dateAndTime), name_(name), xp_(xp), lvl_(lvl), dlvl_(dlvl),
    insanity_(insanity), isVictory_(isVictory) {}
  ~HighScoreEntry() {}

  void set(const string& dateAndTime, const string& name,
           const int XP, const int LVL, const int DLVL, const int INSANITY,
           const bool IS_VICTORY) {
    dateAndTime_ = dateAndTime;
    name_ = name;
    xp_ = XP;
    lvl_ = LVL;
    dlvl_ = DLVL;
    insanity_ = INSANITY;
    isVictory_ = IS_VICTORY;
  }

  inline int getScore() const {
    const double DLVL_DB  = double(dlvl_);
    const double XP_DB    = double(xp_);
    const double INS_DB   = double(insanity_);
    const double SCORE_DB =
      (XP_DB + (isVictory_ * XP_DB / 5.0)) *
      ((((DLVL_DB + 1.0) / LAST_CAVERN_LEVEL) / 3.0) + 1.0) *
      ((INS_DB / 3.0) + 1.0);
    return int(SCORE_DB);
  }
  inline string getDateAndTime() const {return dateAndTime_;}
  inline string getName() const {return name_;}
  inline int getXp() const {return xp_;}
  inline int getLvl() const {return lvl_;}
  inline int getDlvl() const {return dlvl_;}
  inline int getInsanity() const {return insanity_;}
  inline bool isVictoryGame() const {return isVictory_;}

private:
  string dateAndTime_;
  string name_;
  int xp_, lvl_, dlvl_, insanity_;
  bool isVictory_;
};

class HighScore {
public:
  HighScore() {}
  ~HighScore() {}

  void gameOver(const bool IS_VICTORY);
  void runHighScoreScreen();
  vector<HighScoreEntry> getEntriesSorted();

private:
  void sortEntries(vector<HighScoreEntry>& entries);
  void writeFile(vector<HighScoreEntry>& entries);
  void readFile(vector<HighScoreEntry>& entries);

  void renderHighScoreScreen(const vector<HighScoreEntry>& entries,
                             const int TOP_ELEMENT) const;

  static bool isEntryHigher(const HighScoreEntry& current,
                            const HighScoreEntry& other);
};


#endif
