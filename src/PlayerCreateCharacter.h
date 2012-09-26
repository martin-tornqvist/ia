#ifndef PLAYER_CHARACTER_HANDLER_H
#define PLAYER_CHARACTER_HANDLER_H

#include <string>

#include "SDL/SDL.h"

using namespace std;

class Engine;

const int CLASS_BONUS_LEVEL_FIRST = 3;
const int CLASS_BONUS_LEVEL_SECOND = 6;
const int CLASS_BONUS_LEVEL_THIRD = 9;

enum PlayerClassBonusRanks_t {
  playerClassBonusRanks_one,
  playerClassBonusRanks_two,
  playerClassBonusRanks_three
};

enum PlayerBackgrounds_t {
	playerBackground_soldier,
	playerBackground_occultScholar,
	playerBackground_tombRaider,
	endOfPlayerBackgrounds
};

class PlayerCreateCharacter {
public:
	PlayerCreateCharacter(Engine* engine) : playerBackground_(static_cast<PlayerBackgrounds_t>(0)), eng(engine) {}

	void run();

	PlayerBackgrounds_t getPlayerBackground() const {
		return playerBackground_;
	}

private:
	void playerPickBackground(int& currentRenderYpos);
	int drawAndReturnLastYpos(const int CURRENT_SELECTED_POS, const int RENDER_Y_POS_START) const;

	PlayerBackgrounds_t playerBackground_;

	SDL_Event m_event;
	Uint8* m_keystates;
	Engine* eng;
};

class PlayerEnterName {
public:
private:
	friend class PlayerCreateCharacter;
	PlayerEnterName(Engine* engine) : eng(engine) {}
	void run(const int RENDER_AT_Y_POS);
	void draw(const string& currentString, const int RENDER_Y_POS);
	void readKeys(string& currentString, bool& done, const int RENDER_Y_POS);
	Engine* eng;
};

#endif
