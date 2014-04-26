#ifndef JAM_WITH_SPIKE_H
#define JAM_WITH_SPIKE_H

#include "FeatureDoor.h"


class Feature;

class JamWithSpike {
public:
  JamWithSpike() {}
  ~JamWithSpike() {}

  void playerJam() const;

private:
  friend class Feature;
  void playerJamFeature(Feature* const feature) const;


};

#endif
