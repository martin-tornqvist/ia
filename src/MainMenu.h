#ifndef MAIN_MENU_H
#define MAIN_MENU_H

#include "SDL/SDL.h"
#include "MenuBrowser.h"
#include "ConstTypes.h"

class Engine;

class MainMenu {
public:
	MainMenu(Engine* engine) {
		eng = engine;
		pickHplQuote();
	}
	GameEntry_t run(bool *quit);

private:
	Engine* eng;
	void draw(const MenuBrowser& browser);
	
	void pickHplQuote();

	string hplQuote;
};

#endif
