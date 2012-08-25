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

	vector<string> logo;

   logo.push_back("        ___  __                __  __                  ");
	logo.push_back("| |\\  | |   |  )  /\\      /\\  |  )/    /\\  |\\  |  /\\   ");
	logo.push_back("| | \\ | +-- +--  ____    ____ +-- -   ____ | \\ | ____  ");
	logo.push_back("| |  \\| |   | \\ /    \\  /    \\| \\ \\__/    \\|  \\|/    \\ ");
	logo.push_back("               \\                 \\                      ");

	int x_pos = MAP_X_CELLS / 2;
	int y_pos = 3;
	const int X_POS_LEFT = x_pos - 11;

	eng->renderer->clearRenderArea(renderArea_screen);

   const int NR_X_CELLS = eng->config->SCREEN_WIDTH / eng->config->CELL_WIDTH_TEXT;
   const int NR_Y_CELLS = eng->config->SCREEN_HEIGHT / eng->config->CELL_HEIGHT_TEXT;

   for(int y = 0; y < NR_Y_CELLS; y++) {
      for(int x = 0; x < NR_X_CELLS; x++) {
         char cha = ' ';
         if(eng->dice.coinToss()) {
            cha = 'a' + eng->dice.getInRange(0, 25);
         }
         SDL_Color bgClr = clrGray;
         const int BG_BRIGHTNESS = eng->dice.getInRange(6, 15);
         bgClr.r = bgClr.g = bgClr.b = BG_BRIGHTNESS;
         eng->renderer->drawCharacter(cha, renderArea_screen, x, y, bgClr);
      }
   }

	SDL_Color clrGeneral = clrRedLight;
	SDL_Color clrDark = clrRed;
	SDL_Color clrBright = clrWhite;

   const int LOGO_X_POS_LEFT = (MAP_X_CELLS - logo.at(0).size()) / 2;

	for(unsigned int i = 0; i < logo.size(); i++) {
      x_pos = LOGO_X_POS_LEFT;
      for(unsigned int ii = 0; ii < logo.at(i).size(); ii++) {
         if(logo.at(i).at(ii) != ' ') {
            SDL_Color clr = clrBlueLight;
            clr.b += eng->dice.getInRange(-110, 0);
            eng->renderer->drawCharacter(logo.at(i).at(ii), renderArea_screen, x_pos, y_pos, clr);
         }
         x_pos++;
      }
		y_pos += 1;
	}
	x_pos = MAP_X_CELLS / 2;

	eng->renderer->drawTextCentered(eng->config->GAME_VERSION, renderArea_screen, x_pos, y_pos, clrGeneral);
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
	y_pos += 3;

	eng->renderer->drawText("a) Start a new game", renderArea_screen, X_POS_LEFT, y_pos, browser.isPosAtKey('a') ? clrBright : clrGeneral);
	y_pos += 1;

	eng->renderer->drawText("b) Continue a game", renderArea_screen, X_POS_LEFT, y_pos, browser.isPosAtKey('b') ? clrBright : clrGeneral);
	y_pos += 1;

	eng->renderer->drawText("c) View the High Score", renderArea_screen, X_POS_LEFT, y_pos, browser.isPosAtKey('c') ? clrBright : clrGeneral);
	y_pos += 1;

	eng->renderer->drawText("d) Read the manual", renderArea_screen, X_POS_LEFT, y_pos, browser.isPosAtKey('d') ? clrBright : clrGeneral);
	y_pos += 1;

	eng->renderer->drawText("e) Quit", renderArea_screen, X_POS_LEFT, y_pos, browser.isPosAtKey('e') ? clrBright : clrGeneral);
	y_pos += 1;


	eng->renderer->drawTextCentered("Tile set provided by Oryx (www.oryxdesignlab.com)", renderArea_characterLines, x_pos, 1, clrGeneral);


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

