#ifndef SOUND_H
#define SOUND_H

#include <string>

#include "ConstTypes.h"
#include "ItemData.h"
#include "ConstDungeonSettings.h"

using namespace std;

class Engine;
class Actor;

const int SOUND_STANDARD_STRENGTH_GUNFIRE = 4;

class Sound {
public:
	Sound(string message, bool isMessageIgnoredIfPlayerSeeCell, coord origin, int strength, bool isAlertingMonsters) :
		message_(message), isMessageIgnoredIfPlayerSeeCell_(isMessageIgnoredIfPlayerSeeCell), origin_(origin), strength_(strength),
		isAlertingMonsters_(isAlertingMonsters) {}

	Sound() {
	}
	~Sound() {
	}

	string getMessage() const {
		return message_;
	}

	bool isMessageIgnoredIfPlayerSeeCell() const {
		return isMessageIgnoredIfPlayerSeeCell_;
	}

	coord getOrigin() const {
		return origin_;
	}

	int getStrength() const {
		return strength_;
	}

	bool isAlertingMonsters() const {
		return isAlertingMonsters_;
	}

	void addDirectionString(const string directionString) {
		message_ += directionString;
	}

private:
	string message_;
	bool isMessageIgnoredIfPlayerSeeCell_;
	coord origin_;
	int strength_;
	bool isAlertingMonsters_;
};

class SoundEmitter {
public:
	SoundEmitter(Engine* engine) :
		eng(engine) {
	}

	void emitSound(Sound sound) const;

private:
	string getPlayerToOriginDirectionString(const int floodValueAtPlayer, const coord origin, int floodFill[MAP_X_CELLS][MAP_Y_CELLS]) const;

	bool isSoundHeardAtRange(const int range, const int soundStrengthRating) const;

	DirectionNames directionNames;

	Engine* eng;
};

#endif

