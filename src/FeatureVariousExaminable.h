#ifndef FEATURE_VARIOUS_EXAMINABLE_H
#define FEATURE_VARIOUS_EXAMINABLE_H

#include "Feature.h"

class Tomb: public FeatureStatic {
public:
  ~Tomb() {
  }
  void examine();
private:
  friend class FeatureFactory;
  Tomb(Feature_t id, coord pos, Engine* engine);
};

#endif
