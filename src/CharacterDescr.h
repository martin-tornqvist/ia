#ifndef CHARACTER_DESCR_H
#define CHARACTER_DESCR_H

#include <vector>

#include "CommonTypes.h"

class Engine;

class CharacterDescr {
public:
	CharacterDescr(Engine* engine) : eng(engine) {}
	void run();
private:
    void drawInterface();
    void makeLines();
    vector<StringAndClr> lines;
    Engine* eng;
};

#endif
