#ifndef MAPTEMPLATES_H
#define MAPTEMPLATES_H

#include <vector>
#include <string>
#include <iostream>

#include "FeatureData.h"

enum class MapTemplId {church, egypt, END};

struct MapTempl {
  MapTempl() : w(0), h(0) {featureVector.resize(0);}

  std::vector< std::vector<FeatureId> > featureVector;
  int w, h;
};

namespace MapTemplHandling {

void init();

const MapTempl& getTempl(const MapTemplId id);

} //MapTemplHandling

#endif
