#ifndef BLOOD_H
#define BLOOD_H

#include "CommonTypes.h"

class Engine;

class Gore {
public:
	Gore(Engine* engine) :
		eng(engine) {
	}

	void makeBlood(const Pos& origin);
	void makeGore(const Pos& origin);

private:
	Engine* eng;
};

#endif
