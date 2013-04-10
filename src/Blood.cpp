#include "Blood.h"

#include "Engine.h"
#include "Map.h"

void Gore::makeBlood(const coord& origin) {
	for(int dx = -1; dx <= 1; dx++) {
		for(int dy = -1; dy <= 1; dy++) {
			const coord c = origin + coord(dx, dy);
			if(eng->map->featuresStatic[c.x][c.y]->canHaveBlood()) {
				if(eng->dice.percentile() > 66) {
					eng->map->featuresStatic[c.x][c.y]->setHasBlood(true);
				}
			}
		}
	}
}

void Gore::makeGore(const coord& origin) {
	for(int dx = -1; dx <= 1; dx++) {
		for(int dy = -1; dy <= 1; dy++) {
			const coord c = origin + coord(dx, dy);
			if(eng->dice.percentile() > 66) {
				eng->map->featuresStatic[c.x][c.y]->setGoreIfPossible();
			}
		}
	}
}
