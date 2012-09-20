#include "Postmortem.h"

#include <fstream>
#include <iostream>

#include "Engine.h"
#include "ActorPlayer.h"
#include "DungeonMaster.h"
#include "Map.h"
#include "Log.h"
#include "MenuInputHandler.h"
#include "Highscore.h"

void Postmortem::run(bool* const quitGame) {
	makeInfoLines();

	readKeysMenu(quitGame);
}

void Postmortem::makeInfoLines() {
	postmortemLines.push_back(StringAndColor(" " + eng->player->getNameA(), clrCyanLight));

	postmortemLines.push_back(StringAndColor("   *Advanced to character level " + intToString(eng->dungeonMaster->getLevel()), clrRedLight));
	postmortemLines.push_back(StringAndColor("   *Gained " + intToString(eng->dungeonMaster->getXp()) + " experience points", clrRedLight));
	postmortemLines.push_back(StringAndColor("   *Explored to the depth of dungeon level " + intToString(eng->map->getDungeonLevel()), clrRedLight));
	postmortemLines.push_back(StringAndColor("   *Was " + intToString(eng->player->insanityLong) + "% insane", clrRedLight));

	//Make some sort of insanity class where this info is stored, this shit is fugly as hell***
	if(eng->player->insanityPhobias[insanityPhobia_closedPlace])
		postmortemLines.push_back(StringAndColor("   *Had a phobia of enclosed spaces", clrRedLight));
	if(eng->player->insanityPhobias[insanityPhobia_dog])
		postmortemLines.push_back(StringAndColor("   *Had a phobia of dogs", clrRedLight));
	if(eng->player->insanityPhobias[insanityPhobia_rat])
		postmortemLines.push_back(StringAndColor("   *Had a phobia of rats", clrRedLight));
	if(eng->player->insanityPhobias[insanityPhobia_undead])
		postmortemLines.push_back(StringAndColor("   *Had a phobia of the dead", clrRedLight));
	if(eng->player->insanityPhobias[insanityPhobia_openPlace])
		postmortemLines.push_back(StringAndColor("   *Had a phobia of open places", clrRedLight));
	if(eng->player->insanityPhobias[insanityPhobia_spider])
		postmortemLines.push_back(StringAndColor("   *Had a phobia of spiders", clrRedLight));
	if(eng->player->insanityPhobias[insanityPhobia_deepPlaces])
		postmortemLines.push_back(StringAndColor("   *Had a phobia of deep places", clrRedLight));

	if(eng->player->insanityCompulsions[insanityCompulsion_masochism])
		postmortemLines.push_back(StringAndColor("   *Had a masochistic compulsion", clrRedLight));
	if(eng->player->insanityCompulsions[insanityCompulsion_sadism])
		postmortemLines.push_back(StringAndColor("   *Had a sadistic compulsion", clrRedLight));

	postmortemLines.push_back(StringAndColor(" ", clrRedLight));
	postmortemLines.push_back(StringAndColor(" Cause of death:", clrRedLight));
	postmortemLines.push_back(StringAndColor("   *" + causeOfDeath, clrRedLight));
	postmortemLines.push_back(StringAndColor(" ", clrRedLight));

	postmortemLines.push_back(StringAndColor(" The last messages:", clrRedLight));
	int historyElement = static_cast<unsigned int>(max(0, static_cast<int>(eng->log->history.size()) - 6));
	for(unsigned int i = historyElement; i < eng->log->history.size(); i++) {
		string row = "";
		for(unsigned int ii = 0; ii < eng->log->history.at(i).size(); ii++) {
			row += eng->log->history.at(i).at(ii).str + " ";
		}
		postmortemLines.push_back(StringAndColor("   " + row, clrRedLight));
	}
	postmortemLines.push_back(StringAndColor(" ", clrRedLight));

	postmortemLines.push_back(StringAndColor(" The final moment:", clrRedLight));

	for(int y = 0; y < MAP_Y_CELLS; y++) {
		for(int x = 0; x < MAP_X_CELLS; x++) {
			for(int dx = -1; dx <= 1; dx++) {
				for(int dy = -1; dy <= 1; dy++) {
					if(eng->mapTests->isCellInsideMainScreen(coord(x + dx, y + dy))) {
						if(eng->map->featuresStatic[x + dx][y + dy]->isVisionPassable()) {
							eng->map->playerVision[x][y] = true;
						}
					}
				}
			}
		}
	}
	eng->renderer->drawASCII(); //To set the glyph array
	for(int y = 0; y < MAP_Y_CELLS; y++) {
		string currentRow;
		for(int x = 0; x < MAP_X_CELLS; x++) {
			if(coord(x, y) == eng->player->pos) {
				currentRow.push_back('@');
			} else {
				if(eng->renderer->renderArray[x][y].glyph == ' ' && (y == 0 || x == 0 || y == MAP_Y_CELLS - 1 || x == MAP_X_CELLS - 1)) {
					currentRow.push_back('*');
				} else {
					if(eng->renderer->renderArray[x][y].glyph == eng->featureData->getFeatureDef(feature_stoneWall)->glyph) {
						currentRow.push_back('#');
					} else {
						currentRow.push_back(eng->renderer->renderArray[x][y].glyph);
					}
				}
			}
		}
		postmortemLines.push_back(StringAndColor(currentRow, clrRedLight));
		currentRow.clear();
	}
}

void Postmortem::renderInfo(const int TOP_ELEMENT) {
	eng->renderer->clearRenderArea(renderArea_screen);
	const string decorationLine(MAP_X_CELLS - 2, '-');
	eng->renderer->drawText(decorationLine, renderArea_screen, 1, 1, clrWhite);
	eng->renderer->drawText("Displaying postmortem information", renderArea_screen, 3, 1, clrWhite);
	eng->renderer->drawText(decorationLine, renderArea_characterLines, 1, 1, clrWhite);
	eng->renderer->drawText(" [2/8, Down/Up] to navigate  [Space/Esc] to exit. ", renderArea_characterLines, 3, 1, clrWhite);
	int x = 0;
	int y = 0;
	for(unsigned int i = TOP_ELEMENT; i < postmortemLines.size() && (i - TOP_ELEMENT) <= static_cast<unsigned int>(MAP_Y_CELLS); i++) {
		eng->renderer->drawText(postmortemLines.at(i).str, renderArea_mainScreen, x, y, postmortemLines.at(i).clr);
		y++;
	}

	eng->renderer->flip();
}

void Postmortem::runInfo() {
	int topElement = 0;
	renderInfo(topElement);

	//Read keys
	SDL_Event event;
	bool done = false;
	while(done == false) {
		while(SDL_PollEvent(&event)) {
			switch(event.type) {
			case SDL_KEYDOWN: {
				int key = event.key.keysym.sym;

				switch(key) {
				case SDLK_2:
				case SDLK_KP2:
				case SDLK_DOWN: {
					topElement = max(0, min(topElement + static_cast<int>(MAP_Y_CELLS / 5), static_cast<int>(postmortemLines.size())
					                        - static_cast<int>(MAP_Y_CELLS)));
					renderInfo(topElement);
				}
				break;
				case SDLK_8:
				case SDLK_KP8:
				case SDLK_UP: {
					topElement = max(0, min(topElement - static_cast<int>(MAP_Y_CELLS / 5), static_cast<int>(postmortemLines.size())
					                        - static_cast<int>(MAP_Y_CELLS)));
					renderInfo(topElement);
				}
				break;
				case SDLK_SPACE:
				case SDLK_ESCAPE: {
					done = true;
				}
				break;

				}
			}
			break;
			default: {
			}
			break;
			}
		}
		SDL_Delay(1);
	}
}

void Postmortem::makeMemorialFile(const string path) {
	ofstream file;
	string fname = path;
	file.open(fname.data(), ios::trunc);

	for(unsigned int i = 0; i < postmortemLines.size(); i++) {
		file << postmortemLines.at(i).str << endl;
	}

	file.close();
}

void Postmortem::readKeysMenu(bool* const quitGame) {
	MenuBrowser browser(6, 0);

	renderMenu(browser);

	bool done = false;
	while(done == false) {
		const MenuAction_t action = eng->menuInputHandler->getAction(browser);
		switch(action) {
		case menuAction_browsed: {
			renderMenu(browser);
		}
		break;
		case menuAction_canceled: {
		}
		break;
		case menuAction_selected: {
			if(browser.isPosAtKey('a')) {
				runInfo();
				renderMenu(browser);
			}
			if(browser.isPosAtKey('b')) {
				eng->highScore->runHighScoreScreen();
				renderMenu(browser);
			}
			if(browser.isPosAtKey('c')) {
				eng->log->displayHistory();
				renderMenu(browser);
			}
			if(browser.isPosAtKey('d')) {
				const string memorialFilePath = "memorial/" + eng->player->getNameA() + ".txt";
				makeMemorialFile(memorialFilePath);
				eng->renderer->drawText("Memorial file written at " + memorialFilePath, renderArea_characterLines, 1, 0, clrWhite);
				eng->renderer->flip();
			}
			if(browser.isPosAtKey('e')) {
				done = true;
			}
			if(browser.isPosAtKey('f')) {
				*quitGame = true;
				done = true;
			}
		}
		break;
		}
	}
}

void Postmortem::renderMenu(const MenuBrowser& browser) {
	vector<string> art;

	string curLine;
	ifstream file("ascii_graveyard");

	if(file.is_open()) {
		while(getline(file, curLine)) {
			if(curLine.size() > 0) {
				art.push_back(curLine);
			}
		}
	} else {
		tracer << "[WARNING] Could not open ascii graveyard file, in Postmortem::renderMenu()" << endl;
	}

	file.close();

	eng->renderer->clearRenderArea(renderArea_screen);

	int x = 1;
	int y = 1;

	for(unsigned int i = 0; i < art.size(); i++) {
		eng->renderer->drawText(art.at(i), renderArea_screen, x, y, clrWhiteHigh);
		y += 1;
	}

	x = 45;
	y = 18;
	const string NAME_STR = eng->player->getArchetypeDefinition()->name_a;
	eng->renderer->drawTextCentered(NAME_STR, renderArea_screen, x, y, clrWhiteHigh);

	y += 2;
	const string LVL_STR = "LVL " + intToString(eng->dungeonMaster->getLevel());
	eng->renderer->drawTextCentered(LVL_STR, renderArea_screen, x, y, clrWhiteHigh);

	//Draw command labels
	x = 55;
	y = 14;
	eng->renderer->drawText("a) Information", renderArea_screen, x, y, browser.isPosAtKey('a') ? clrWhite : clrRedLight);
	y += 1;

	eng->renderer->drawText("b) View the High Score", renderArea_screen, x, y, browser.isPosAtKey('b') ? clrWhite : clrRedLight);
	y += 1;

	eng->renderer->drawText("c) View messages", renderArea_screen, x, y, browser.isPosAtKey('c') ? clrWhite : clrRedLight);
	y += 1;

	eng->renderer->drawText("d) Save memorial file", renderArea_screen, x, y, browser.isPosAtKey('d') ? clrWhite : clrRedLight);
	y += 1;

	eng->renderer->drawText("e) Return to main menu", renderArea_screen, x, y, browser.isPosAtKey('e') ? clrWhite : clrRedLight);
	y += 1;

	eng->renderer->drawText("f) Quit the game", renderArea_screen, x, y, browser.isPosAtKey('f') ? clrWhite : clrRedLight);
	y += 1;

	eng->renderer->flip();
}
