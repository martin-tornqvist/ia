#ifndef HIGH_SCORE_H
#define HIGH_SCORE_H

#include <vector>
#include <string>

using namespace std;

class Engine;

struct HighScoreEntry {
public:
	HighScoreEntry(string name_, int xp_, int lvl_, int dlvl_, int insanity_, bool isVictory_) :
		name(name_), xp(xp_), lvl(lvl_), dlvl(dlvl_), insanity(insanity_), isVictory(isVictory_) {
	}
	HighScoreEntry() :
		name(""), xp(0), isVictory(false) {
	}

	void set(const string NAME, const int XP, const int LVL, const int DLVL, const int INSANITY, const bool IS_VICTORY) {
		name = NAME;
		xp = XP;
		lvl = LVL;
		dlvl = DLVL;
		insanity = INSANITY;
		isVictory = IS_VICTORY;
	}

	int getScore() const {
		return xp + (isVictory ? xp / 5 : 0);
	}
	int getXp() const {
		return xp;
	}
	string getName() const {
		return name;
	}
	int getLvl() const {
		return lvl;
	}
	int getDlvl() const {
		return dlvl;
	}
	int getInsanity() const {
		return insanity;
	}
	bool isVictoryGame() const {
		return isVictory;
	}

private:
	string name;
	int xp, lvl, dlvl, insanity;
	bool isVictory;
};

class HighScore {
public:
	HighScore(Engine* engine) :
		eng(engine) {
	}

	void gameOver(const bool IS_VICTORY);

	void runHighScoreScreen();

private:
	void sortEntries(vector<HighScoreEntry>& entries);

	void writeFile(vector<HighScoreEntry>& entries);

	void readFile(vector<HighScoreEntry>& entries);

	void renderHighScoreScreen(const vector<HighScoreEntry>& entries, const int TOP_ELEMENT) const;

	static bool isEntryHigher(const HighScoreEntry& current, const HighScoreEntry& other);

	Engine* eng;
};


#endif
