#ifndef EXPLOSION_H
#define EXPLOSION_H

#include <vector>

#include "ConstTypes.h"
#include "StatusEffects.h"
#include "Config.h"
#include "Colors.h"

using namespace std;

class Engine;
class TimedEntity;

class ExplosionMaker {
public:
	ExplosionMaker(Engine* engine) :
		width(5), height(5), eng(engine) {
	}

	ExplosionMaker& operator=(const ExplosionMaker& other) {
		(void)other;
		return *this;
	}

	void runExplosion(const coord& origin, const bool DO_EXPLOSION_DMG = true, StatusEffect* const effect = NULL,
	                  const bool OVERRIDE_EXPLOSION_RENDERING = false, const sf::Color colorOverride = clrWhite);

	void runSmokeExplosion(const coord& origin, const bool SMALL_RADIUS = false);

private:

	const int width;
	const int height;

	struct BasicData {
		BasicData(const coord& origin, const int width, const int height) :
			x0(origin.x - (width - 1) / 2), y0(origin.y - (height - 1) / 2),
			x1(origin.x + (width - 1) / 2), y1(origin.y + (height - 1) / 2) {
		}

		BasicData& operator=(const BasicData& other) {
			(void)other;
			return *this;
		}

		const int x0;
		const int y0;
		const int x1;
		const int y1;
	};

	void renderExplosion(const BasicData* data, bool reach[MAP_X_CELLS][MAP_Y_CELLS]);

	void renderExplosionWithColorOverride(const BasicData* data, const sf::Color clr, bool reach[MAP_X_CELLS][MAP_Y_CELLS]);

	Engine* eng;
};

#endif
