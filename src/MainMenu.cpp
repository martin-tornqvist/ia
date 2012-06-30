#include "MainMenu.h"

#include <string>

#include "Engine.h"
#include "Colors.h"
#include "Render.h"
#include "MenuInputHandler.h"
#include "SaveHandler.h"
#include "Highscore.h"
#include "Manual.h"

using namespace std;

void MainMenu::draw(const MenuBrowser& browser) {
	int x_pos = MAP_X_CELLS / 2;
	int y_pos = 3;
	const int X_POS_LEFT = x_pos - 11;

	eng->renderer->clearRenderArea(renderArea_screen);

	SDL_Color clr = clrRedLight;
	SDL_Color clrDark = clrRed;
	SDL_Color clrBright = clrWhite;

	eng->renderer->drawTextCentered(eng->config->GAME_TITLE + eng->config->GAME_VERSION, renderArea_screen, x_pos, y_pos, clr);
	y_pos += 1;

	eng->renderer->drawTextCentered("(c) 2011-2012 Martin Tornqvist", renderArea_screen, x_pos, y_pos, clrDark);
	y_pos += 1;

	eng->renderer->drawTextCentered("m.tornq@gmail.com", renderArea_screen, x_pos, y_pos, clrDark);
	y_pos += 2;

	eng->renderer->drawTextCentered("http://infraarcana.wikispaces.com", renderArea_screen, x_pos, y_pos, clrDark);
	y_pos += 2;

	eng->renderer->drawTextCentered("You can set options by editing config.txt", renderArea_screen, x_pos, y_pos, clrDark);
	y_pos += 1;
	eng->renderer->drawTextCentered("(switch between tile/ASCII mode, skip intro level, etc)", renderArea_screen, x_pos, y_pos, clrDark);
	y_pos += 5;

	eng->renderer->drawText("a) Start a new game", renderArea_screen, X_POS_LEFT, y_pos, browser.isPosAtKey('a') ? clrBright : clr);
	y_pos += 1;

	eng->renderer->drawText("b) Continue a game", renderArea_screen, X_POS_LEFT, y_pos, browser.isPosAtKey('b') ? clrBright : clr);
	y_pos += 1;

	eng->renderer->drawText("c) View the High Score", renderArea_screen, X_POS_LEFT, y_pos, browser.isPosAtKey('c') ? clrBright : clr);
	y_pos += 1;

	eng->renderer->drawText("d) Read the manual", renderArea_screen, X_POS_LEFT, y_pos, browser.isPosAtKey('d') ? clrBright : clr);
	y_pos += 1;

	eng->renderer->drawText("e) Quit", renderArea_screen, X_POS_LEFT, y_pos, browser.isPosAtKey('e') ? clrBright : clr);
	y_pos += 1;


	eng->renderer->drawTextCentered("Tile set provided by Oryx (www.oryxdesignlab.com)", renderArea_characterLines, x_pos, 1, clr);


	eng->renderer->flip();
}

GameEntry_t MainMenu::run(bool* quit) {
	MenuBrowser browser(5, 0);

	draw(browser);

	bool proceed = false;
	while(proceed == false) {
		const MenuAction_t action = eng->menuInputHandler->getAction(browser);
		switch(action) {
		case menuAction_browsed: {
			draw(browser);
		}
		break;
		case menuAction_canceled: {
		}
		break;
		case menuAction_selected: {
			if(browser.isPosAtKey('a')) {
				proceed = true;
				return gameEntry_new;
			}
			if(browser.isPosAtKey('b')) {
				const bool GAME_LOADED = eng->saveHandler->playerChooseLoad();
				if(GAME_LOADED == true) {
					proceed = true;
					return gameEntry_load;
				} else {
					draw(browser);
				}
			}
			if(browser.isPosAtKey('c')) {
				eng->highScore->runHighScoreScreen();
				draw(browser);
			}
			if(browser.isPosAtKey('d')) {
				eng->manual->run();
				draw(browser);
			}
			if(browser.isPosAtKey('e')) {
				proceed = true;
				*quit = true;
			}
		}
		break;
		default: {
		}
		break;
		}
	}
	return gameEntry_new;
}

