#ifndef JAM_WITH_SPIKE_H
#define JAM_WITH_SPIKE_H

#include "FeatureDoor.h"

class Engine;
class Feature;

class JamWithSpike
{
public:
	JamWithSpike(Engine* engine) : eng(engine) {}
	~JamWithSpike() {}

	void playerJam() const;

private:
   friend class Feature;
   void playerJamFeature(Feature* const feature) const;

	Engine* eng;
};

#endif
