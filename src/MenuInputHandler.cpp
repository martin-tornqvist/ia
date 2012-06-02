#include "MenuInputHandler.h"

#include "SDL/SDL.h"

#include "Engine.h"

MenuAction_t MenuInputHandler::getAction(MenuBrowser& browser) {
	bool done = false;
	while(done == false) {
		while(SDL_PollEvent(&m_event)) {
			if(m_event.type == SDL_KEYDOWN) {
				Uint16 key = static_cast<Uint16> (m_event.key.keysym.sym);

				if(key == SDLK_RIGHT || key == SDLK_KP6 || key == SDLK_6) {
					browser.navigate(direction_right);
					return menuAction_browsed;
				}

				if(key == SDLK_LEFT || key == SDLK_KP4 || key == SDLK_4) {
					browser.navigate(direction_left);
					return menuAction_browsed;
				}

				if(key == SDLK_UP || key == SDLK_KP8 || key == SDLK_8) {
					browser.navigate(direction_up);
					return menuAction_browsed;
				}

				if(key == SDLK_DOWN || key == SDLK_KP2 || key == SDLK_2) {
					browser.navigate(direction_down);
					return menuAction_browsed;
				}

				if(key == SDLK_RETURN || key == SDLK_KP_ENTER) {
					key = browser.enter();
				}

				if(key == SDLK_SPACE || key == SDLK_ESCAPE) {
					return menuAction_canceled;
				}

				const int SIZE_OF_FIRST_LIST = browser.getNrOfItemsInFirstList();
				const int SIZE_OF_SECOND_LIST = browser.getNrOfItemsInSecondList();

				if(key >= 'a' && key < 'a' + static_cast<char> (SIZE_OF_FIRST_LIST + SIZE_OF_SECOND_LIST)) {
					browser.navigate(key);
					return menuAction_selected;
				}
			}
			SDL_Delay(1);
		}
	}
	return menuAction_canceled;
}

