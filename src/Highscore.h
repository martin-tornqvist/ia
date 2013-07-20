#ifndef HIGH_SCORE_H
#define HIGH_SCORE_H

#include <vector>
#include <string>

using namespace std;

class Engine;

struct HighScoreEntry {
public:
  HighScoreEntry(string dateAndTime, string name, int dlvl, int insanity,
                 bool isVictory) :
    dateAndTime_(dateAndTime), name_(name), dlvl_(dlvl), insanity_(insanity),
    isVictory_(isVictory) {}
  ~HighScoreEntry() {}

  void set(const string& dateAndTime, const string& name, const int DLVL,
           const int INSANITY, const bool IS_VICTORY) {
    dateAndTime_ = dateAndTime;
    name_ = name;
    dlvl_ = DLVL;
    insanity_ = INSANITY;
    isVictory_ = IS_VICTORY;
  }

  inline int getScore() const {return 0 + (isVictory_ ? 0 / 5 : 0);}
  inline string getDateAndTime() const {return dateAndTime_;}
  inline string getName() const {return name_;}
  inline int getDlvl() const {return dlvl_;}
  inline int getInsanity() const {return insanity_;}
  inline bool isVictoryGame() const {return isVictory_;}

private:
  string dateAndTime_;
  string name_;
  int dlvl_, insanity_;
  bool isVictory_;
};

class HighScore {
public:
  HighScore(Engine* engine) : eng(engine) {}
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

  Engine* eng;
};


#endif
