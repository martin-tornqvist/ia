#ifndef EXAMINE_H
#define EXAMINE_H

#include <vector>
#include <string>

using namespace std;

class Engine;

class Examine {
public:
	Examine(Engine* engine) : eng(engine) {
	}

	~Examine() {
	}

	void playerExamine() const;

private:

	Engine* eng;
};

#endif

