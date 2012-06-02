#ifndef POSTMORTEM_H
#define POSTMORTEM_H

#include "SDL/SDL.h"

#include <vector>
#include <string>

#include "MenuBrowser.h"

using namespace std;

class Engine;

class Postmortem {
public:
	Postmortem(Engine* engine) :
		causeOfDeath("Unknown"), eng(engine) {
	}

	void run(bool* const quitGame);

	void setCauseOfDeath(const string cause) {
		causeOfDeath = cause;
	}

private:
	struct StringAndColor {
		StringAndColor(const string str_, const SDL_Color clr_) :
			str(str_), clr(clr_) {
		}
		StringAndColor() {
		}
		string str;
		SDL_Color clr;
	};

	vector<StringAndColor> postmortemLines;

	void readKeysMenu(bool* const quitGame);

	void renderMenu(const MenuBrowser& browser);

	void runInfo();

	void renderInfo(const int TOP_ELEMENT);

	void makeMemorialFile(const string path);

	void makeInfoLines();

	string causeOfDeath;

	Engine* eng;
};

#endif
