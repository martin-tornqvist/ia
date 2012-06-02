#ifndef INPUT_H
#define INPUT_H

#include "SDL/SDL.h"

#include "ConstTypes.h"

class Engine;

class Input {
public:
	Input(Engine* engine, bool* quitToMainMenu) : eng(engine), quitToMainMenu_(quitToMainMenu)  {
		m_keystates = SDL_GetKeyState(NULL);
	}

	void read();

	void handleKeyPress(Uint16 key, const bool SHIFT, const bool CTRL);

	void clearKeyEvents() {
		while(SDL_PollEvent(&m_event)) {
		}
	}

private:
	void clearMessages();
	SDL_Event m_event;
	Uint8 *m_keystates;
	int* dungeonLevel_;
	Engine *eng;
	bool* quitToMainMenu_;
};

#endif
