#ifndef TOP_MENU_H
#define TOP_MENU_H

/*
 * MenuInputHandler is responsible for the I/O-side of menu handling.
 * A menu-using application creates a MenuBrowser which is used for
 * communication between the input handler and the application to store
 * and modify the position in the menu.
 *
 */

#include <vector>
#include <string>

#include "MenuBrowser.h"

using namespace std;

class Engine;

enum MenuAction_t {
	menuAction_browsed, menuAction_selected, menuAction_canceled
};

//struct MenuInputHandlerReturnData {
//	MenuInputHandlerReturnData(MenuAction action, unsigned int elementSelected = 0) :
//		action_(action), elementSelected_(elementSelected) {
//	}
//
//	const MenuAction action_;
//	const unsigned int elementSelected_;
//};

class MenuInputHandler {
public:
	MenuAction_t getAction(MenuBrowser& browser);

private:
	SDL_Event m_event;
	Uint8* m_keystates;

	Engine* eng;

	friend class Engine;
	MenuInputHandler(Engine* engine) :
		eng(engine) {
	}
};

#endif
