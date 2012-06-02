#ifndef PLAYER_POWERS_HANDLER_H
#define PLAYER_POWERS_HANDLER_H

/*
 * Responsible for providing a user interface for selecting spells to cast,
 * gathered from all sources (scrolls, memorised scrolls, etc), and also for
 * ending the turn when the spell was cast.
 *
 */

#include <vector>

#include "MenuBrowser.h"

using namespace std;

class Engine;
class Scroll;

class PlayerPowersHandler {
public:
	PlayerPowersHandler(Engine* engine);
	~PlayerPowersHandler();

	void run(const bool CAST_FROM_MEMORY);

private:
	void draw(MenuBrowser& browser, const bool DRAW_COMMAND_PROMPT, const bool CAST_FROM_MEMORY,
	          const vector<unsigned int> generalInventorySlotsToShow, const vector<unsigned int> memorizedScrollsToShow);

	vector<Scroll*> scrollsToReadFromPlayerMemory;

	Engine* eng;

};

#endif
