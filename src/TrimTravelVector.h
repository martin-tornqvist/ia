#ifndef TRIM_TRAVEL_VECTOR_H
#define TRIM_TRAVEL_VECTOR_H

/*-------------------------------------------------------------------
 * Purpose:
 * Given a vector of coordinates, this class returns a bool vector
 * specifying which elements to draw and which to skip, for a better
 * looking travel path (a less "slithery" path)
 -------------------------------------------------------------------*/


/*
 * THIS CLASS IS _UNUSED_ AND _UNTESTED_ !
 */


#include <vector>

#include "CommonTypes.h"

using namespace std;

class Engine;

class TrimTravelVector {
public:
	TrimTravelVector(Engine* engine) :
		eng(engine) {
	}

	vector<bool> trimVector(const vector<coord>& path);

private:
	Engine* eng;
};

#endif

