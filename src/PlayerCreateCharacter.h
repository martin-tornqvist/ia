#ifndef PLAYER_CREATE_CHARACTER_H
#define PLAYER_CREATE_CHARACTER_H

#include <string>

#include "SDL/SDL.h"

using namespace std;

class Engine;

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

//	PlayerBackgrounds_t getPlayerBackground() const {
//		return playerBackground_;
//	}

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
	void readKeys(string& currentString, bool& done);
	Engine* eng;
};

#endif
