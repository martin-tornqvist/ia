#include "SaveHandler.h"

#include "SDL/SDL.h"

#include <fstream>
#include <iostream>

#include "Engine.h"
#include "Log.h"
#include "Render.h"
#include "ActorPlayer.h"
#include "DungeonMaster.h"
#include "ItemScroll.h"
#include "ItemPotion.h"
#include "PlayerBonuses.h"
#include "Map.h"
#include "DungeonClimb.h"
#include "Inventory.h"

bool SaveHandler::save()
{
	vector<string> linesEntire;
	readFile(linesEntire);

	//If too many saves, refuse to save
	vector<unsigned int> savePositions;
	getSavePositions(linesEntire, savePositions);
	if(savePositions.size() >= 8) {
		eng->log->clearLog();
		eng->log->addMessage("Too many games saved. Try removing the save file.");
		eng->renderer->flip();
		return false;
	} else {
		vector<string> linesCurrentSave;
		collectLinesFromGame(linesCurrentSave);

		for(unsigned int i = 0; i < linesCurrentSave.size(); i++) {
			linesEntire.push_back(linesCurrentSave.at(i));
		}
		writeFile(linesEntire);
	}
	return true;
}

void SaveHandler::collectLinesFromGame(vector<string>& linesCurrentSave)
{
	linesCurrentSave.resize(0);
	linesCurrentSave.push_back("@" + eng->player->getNameA());

	eng->dungeonMaster->addSaveLines(linesCurrentSave);
	eng->scrollNameHandler->addSaveLines(linesCurrentSave);
	eng->potionNameHandler->addSaveLines(linesCurrentSave);
	eng->itemData->addSaveLines(linesCurrentSave);
	eng->player->getInventory()->addSaveLines(linesCurrentSave);
	eng->player->addSaveLines(linesCurrentSave);
	eng->playerBonusHandler->addSaveLines(linesCurrentSave);
	eng->map->addSaveLines(linesCurrentSave);
	eng->actorData->addSaveLines(linesCurrentSave);
}

void SaveHandler::setGameParametersFromLines(vector<string>& linesCurrentSave) const
{
	string playerName = linesCurrentSave.front();
	linesCurrentSave.erase(linesCurrentSave.begin() );
	playerName.erase( playerName.begin() );
	eng->player->getInstanceDefinition()->name_a	= playerName;
	eng->player->getInstanceDefinition()->name_the	= playerName;
	eng->player->getArchetypeDefinition()->name_a	= playerName;
	eng->player->getArchetypeDefinition()->name_the	= playerName;

	eng->dungeonMaster->setParametersFromSaveLines(linesCurrentSave);
	eng->scrollNameHandler->setParametersFromSaveLines(linesCurrentSave);
	eng->potionNameHandler->setParametersFromSaveLines(linesCurrentSave);
	eng->itemData->setParametersFromSaveLines(linesCurrentSave);
	eng->player->getInventory()->setParametersFromSaveLines(linesCurrentSave, eng);
	eng->player->setParametersFromSaveLines(linesCurrentSave);
	eng->playerBonusHandler->setParametersFromSaveLines(linesCurrentSave);
	eng->map->setParametersFromSaveLines(linesCurrentSave);
	eng->actorData->setParametersFromSaveLines(linesCurrentSave);

	eng->gameTime->insertActorInLoop(eng->player);

	eng->dungeonClimb->travelDown();
}


bool SaveHandler::playerChooseLoad()
{
	//Read all lines from file
	vector<string> linesEntire;
	readFile(linesEntire);

	//Get the positions that contain save entry starts
	vector<unsigned int> savePositions;
	getSavePositions(linesEntire, savePositions);

	//Read keys
	MenuBrowser browser(savePositions.size() ,0);
	printSaveList(linesEntire, savePositions, browser);

	bool done = false;
		while(done == false)
		{
		while(SDL_PollEvent(&m_event))
		{
			switch(m_event.type)
			{
			case SDL_KEYDOWN:
				{
					Uint16 key = static_cast<Uint16>(m_event.key.keysym.sym);

					if(key == SDLK_RETURN || key == SDLK_KP_ENTER) {
						key = browser.enter();
					}

					if(key == SDLK_UP || key == SDLK_KP8 || key == SDLK_8) {
						browser.navigate(direction_up);
						printSaveList(linesEntire, savePositions, browser);
					}

					if(key == SDLK_DOWN || key == SDLK_KP2 ||key == SDLK_2) {
						browser.navigate(direction_down);
						printSaveList(linesEntire, savePositions, browser);
					}

					//If key corresponds to a save
					if(key >= 'a' && key <= 'a' + static_cast<char>(savePositions.size()-1)) {
						vector<string> linesCurrentSave;
						//Store the save lines belonging to the current save in a separate vector, erase them from the big vector
						getCurrentSaveLines(static_cast<char>(key), linesEntire, savePositions, linesCurrentSave);
						setGameParametersFromLines(linesCurrentSave);
						//Rebuild the external save file (with the current save removed)
						writeFile(linesEntire);
						done = true;
						return true;
					}

					if(key == SDLK_SPACE || key == SDLK_ESCAPE) {
						done = true;
						return false;
					}
				}
			default: {} break;
			}
		}
	}


	return false;
}

void SaveHandler::getCurrentSaveLines(const char key, vector<string>& linesEntire, const vector<unsigned int>& startPositions,
	vector<string>& linesCurrentSave)
{
	linesCurrentSave.resize(0);

	int saveNr = static_cast<int>(key - 'a');

	const unsigned int START_POS_ENTIRE = startPositions.at(saveNr);

	linesCurrentSave.push_back( linesEntire.at(START_POS_ENTIRE) );
	linesEntire.erase(linesEntire.begin() + START_POS_ENTIRE);

	bool done = false;
	while(done == false) {
		if(START_POS_ENTIRE == linesEntire.size()) {
			done = true;
			break;
		}

		const string line = linesEntire.at(START_POS_ENTIRE);
		if(line.size() > 0) {
			if(line.at(0) == '@') {
				done = true;
				break;
			} else {
				linesCurrentSave.push_back(line);
				linesEntire.erase(linesEntire.begin() + START_POS_ENTIRE);
			}
		}
	}
}

void SaveHandler::getSavePositions(const vector<string>& linesEntire, vector<unsigned int>& startPositions)
{
	startPositions.resize(0);

	for(unsigned int i = 0; i < linesEntire.size(); i++) {
		const string line = linesEntire.at(i);
		if(line.size() != 0) {
			if(line.at(0) == '@') {
				startPositions.push_back(i);
			}
		}
	}
}

void SaveHandler::printSaveList(const vector<string>& linesEntire, const vector<unsigned int>& startPositions,
	const MenuBrowser& browser) const
{
	eng->renderer->clearRenderArea(renderArea_screen);

	int xPos = 3;
	int yPos = 3;

	yPos += 2;
	eng->renderer->drawText("Choose save to load:", renderArea_screen, xPos, yPos, clrWhite);

	const unsigned int NR_OF_SAVES = startPositions.size();

	if(NR_OF_SAVES == 0) {
		yPos += 1;
		eng->renderer->drawText("No saves found.", renderArea_screen, xPos, yPos, clrRedLight);
	} else {
		for(unsigned int i = 0; i < NR_OF_SAVES; i++) {
			yPos += 1;

			//Draw index
			string s = "[";
			s.push_back( ('a'+ static_cast<char>(i) ) );
			s += "] " ;

			eng->renderer->drawText(s, renderArea_screen, xPos, yPos, browser.enter() == SDLK_a + i ? clrWhite : clrRedLight);

			xPos += s.size();

			//Save title
			string saveTitle = linesEntire.at( startPositions.at(i) );
			//Remove new-save symbol
			saveTitle.erase(saveTitle.begin());
			eng->renderer->drawText(saveTitle, renderArea_screen, xPos, yPos, browser.enter() == SDLK_a + i ? clrWhite : clrRedLight);

			xPos = 3;
		}
	}

	yPos += 2;
	eng->renderer->drawText("[Space/Esc] Cancel", renderArea_screen, xPos, yPos, clrWhite);

	eng->renderer->flip();
}

void SaveHandler::readFile(vector<string>& linesEntire)
{
	linesEntire.resize(0);

	string curLine;
	ifstream file("save/save");

	if(file.is_open())
	{
		while(getline(file, curLine))
		{
			linesEntire.push_back(curLine);
		}
	}
	else {
		cout << "[WARNING] could not open save file, in SaveHandler::readFile()" << endl;
	}

	file.close();
}

void SaveHandler::writeFile(const vector<string>& linesEntire) const
{
	ofstream file;
	file.open("save/save", ios::trunc);

	for(unsigned int i = 0; i < linesEntire.size(); i++) {
		file << linesEntire.at(i);
		if(i != linesEntire.size()-1) {
			file << endl;
		}
	}

	file.close();
}
