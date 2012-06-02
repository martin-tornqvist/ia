#include "TrimTravelVector.h"

#include "Engine.h"

vector<bool> TrimTravelVector::trimVector(const vector<coord>& path) {
	vector<bool> elementsToUse;

	const int PATH_SIZE = static_cast<int> (path.size());

	elementsToUse.resize(PATH_SIZE);

	for(int i = 0; i < PATH_SIZE; i++) {
		elementsToUse.at(i) = true;
	}

	for(int i = 0; i < PATH_SIZE; i++) {

		if(i < PATH_SIZE - 2) {
			const coord coordCurrent = path.at(i);
			const coord coordJumpTwo = path.at(i + 2);

			if(eng->mapTests->isCellsNeighbours(coordCurrent, coordJumpTwo, false)) {
				elementsToUse.at(i + 1) = false;
			}

		} else {
			return elementsToUse;
		}

	}

	return elementsToUse;
}
