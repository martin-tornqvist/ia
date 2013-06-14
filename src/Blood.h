#ifndef BLOOD_H
#define BLOOD_H

#include "CommonTypes.h"

class Engine;

class Gore {
public:
	Gore(Engine* engine) :
		eng(engine) {
	}

	void makeBlood(const coord& origin);

	void makeGore(const coord& origin);

private:
	Engine* eng;
};

#endif
