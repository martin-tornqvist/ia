#ifndef FOV_PRECALC_H
#define FOV_PRECALC_H

#include <vector>
#include <iostream>

#include "ConstTypes.h"
#include "ConstDungeonSettings.h"

using namespace std;

class Engine;

class FovPreCalc {
public:
	FovPreCalc(Engine* engine) :
		eng(engine) {
		calcLineTravelVectors();
	}

	void getLineTravelVector(vector<coord>& vectorUsed, const coord& delta) const {
		vectorUsed = lineTravelVectors[delta.x + FOV_RADI_INT][delta.y + FOV_RADI_INT];
	}

private:
	void getLineAngles(const int deltaX, const int deltaY, pair<double, double>& angles) const {
		angles.first = vectorAnglesX[deltaX + FOV_RADI_INT][deltaY + FOV_RADI_INT];
		angles.second = vectorAnglesY[deltaX + FOV_RADI_INT][deltaY + FOV_RADI_INT];
	}

	void calcLineAngles();

	double vectorAnglesX[FOV_WIDTH_INT][FOV_WIDTH_INT];
	double vectorAnglesY[FOV_WIDTH_INT][FOV_WIDTH_INT];

	void calcLineTravelVectors();

	vector<coord> lineTravelVectors[FOV_WIDTH_INT][FOV_WIDTH_INT];

	Engine* eng;
};

#endif
