#ifndef FOV_H
#define FOV_H

#include "Map.h"

class Actor;

class Fov {
public:
	Fov(Engine* engine) {
		eng = engine;
	}

	//There is NO blocking array-free version of checkOneCell,
	//this is to reduce the risk of recreating the array every step in a loop.
	bool checkOneCell(bool obstructions[MAP_X_CELLS][MAP_Y_CELLS], const int checkX, const int checkY,
			const int _originX, const int _originY, const bool AFFECTED_BY_DARKNESS);

	void runPlayerFov(bool obstructions[MAP_X_CELLS][MAP_Y_CELLS], const int originX, const int originY);

	void runFovOnArray(bool obstructions[MAP_X_CELLS][MAP_Y_CELLS], int originX, int originY,
			bool array[MAP_X_CELLS][MAP_Y_CELLS], const bool AFFECTED_BY_DARKNESS);

private:
	void allUnseen(bool array[MAP_X_CELLS][MAP_Y_CELLS]);

	void performCheck(bool obstructions[MAP_X_CELLS][MAP_Y_CELLS], int _checkX, int _checkY, int _originX, int _originY,
			bool array[MAP_X_CELLS][MAP_Y_CELLS], const bool AFFECTED_BY_DARKNESS);

	//        int deltaX, deltaY;
	//        double hypote, x_incr, y_incr, incrInX, incrInY;

	Engine* eng;
};

#endif
