#ifndef SAVE_HANDLER
#define SAVE_HANDLER

#include <vector>
#include <string>

#include "MenuBrowser.h"

using namespace std;

class Engine;

class SaveHandler
{
public:
	SaveHandler(Engine* engine) : eng(engine) {}

	bool playerChooseLoad();

	bool save();

private:
	void getSavePositions(const vector<string>& linesEntire, vector<unsigned int>& startPositions);

	void printSaveList(const vector<string>& linesEntire, const vector<unsigned int>& startPositions, const MenuBrowser& browser) const;

	void collectLinesFromGame(vector<string>& linesCurrentSave);

	void setGameParametersFromLines(vector<string>& linesCurrentSave) const;

	void readFile(vector<string>& linesEntire);

	void writeFile(const vector<string>& linesEntire) const;

	void getCurrentSaveLines(const char key, vector<string>& linesEntire, const vector<unsigned int>& startPositions,
		vector<string>& linesCurrentSave);

	Engine* eng;

	SDL_Event m_event;
};

#endif