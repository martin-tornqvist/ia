#ifndef THROWER_H
#define THROWER_H

#include <iostream>
#include <vector>
#include <iostream>
#include <math.h>

#include "ConstTypes.h"

using namespace std;

class Engine;

class Actor;

struct ThrownObject {
	double xIncr, yIncr, xPos_prec, yPos_prec, hypot;
	int xPos, yPos, cellPotential, aimX, aimY;
	vector<coord> flightPath;
	//	coord blockedByCell;
};

class Thrower {
public:
	Thrower(Engine* engine) :
		eng(engine) {
	}

	//"Number" means slotnumber. If number is larger than the equipment slots,
	//it corresponds to an item in general inventory.
	void throwMissile(Actor* const actorThrowing, const coord aim);

	//This is the main throwing function. It can be called from outside to get the result
	//of any kind of throw (useful for throwing lit dynamite - which is not an item, but
	//a status effect- then a dungeon feature when landed)
	vector<coord> getFlightPath(int originX, int originY, int aimX, int aimY, const bool INCLUDE_BLOCKED_CELL,
			const bool CONTINUE_UNTUL_BLOCKED);

	void playerThrowLitExplosive(const coord aimCell);

private:
	void setLineUntilSolidAndDeductCells(ThrownObject &obj, const bool INCLUDE_BLOCKED_CELL, const bool CONTINUE_UNTIL_BLOCKED);

	Engine* eng;
};

#endif
