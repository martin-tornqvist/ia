#ifndef CLOSE_H
#define CLOSE_H

#include "ConstTypes.h"

class Engine;
class Actor;
class Feature;

class Close {
public:
	Close(Engine* engine) : eng(engine) {
	}

	~Close() {
	}

	void playerClose() const;

private:
   friend class Feature;
   void playerCloseFeature(Feature* const feature) const;

	Engine* eng;
};

#endif

