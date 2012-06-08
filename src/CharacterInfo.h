#ifndef CHARACTER_INFO_H
#define CHARACTER_INFO_H

#include <vector>

#include "ConstTypes.h"

class Engine;

class CharacterInfo {
public:
	CharacterInfo(Engine* engine) : eng(engine) {
	}

	void run();

private:
    void drawInterface();
    void makeLines();

    vector<StringAndColor> lines;

    Engine* eng;
};


#endif
