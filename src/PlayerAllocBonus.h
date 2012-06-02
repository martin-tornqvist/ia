#ifndef PLAYER_ALLOC_BONUS_H
#define PLAYER_ALLOC_BONUS_H

#include "SDL/SDL.h"

#include "PlayerBonuses.h"
#include "MenuBrowser.h"

class Engine;

class Skills;

class PlayerAllocBonus {
public:
	PlayerAllocBonus(Engine* engine) :
		ELEMENTS_ON_SCREEN(21), eng(engine) {
	}

	void run();

private:
	void readKeys();

	void browseUp(unsigned int& currentPos, unsigned int& topElement, const unsigned int NR_OF_BONUSES_DEFINED);

	void browseDown(unsigned int& currentPos, unsigned int& topElement, const unsigned int NR_OF_BONUSES_DEFINED);

	void setTopElement(unsigned int& currentPos, unsigned int& topElement, const unsigned int NR_OF_BONUSES_DEFINED);

	void draw(const int PICKS_LEFT, const unsigned int CURRENT_POS, const unsigned int TOP_BONUS) const;

	const unsigned int ELEMENTS_ON_SCREEN;

	SDL_Event m_event;
	Uint8* m_keystates;
	Engine* eng;
};

#endif
