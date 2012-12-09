#ifndef FEATURE_GRAVE_H
#define FEATURE_GRAVE_H

#include "Feature.h"

class Grave: public FeatureStatic {
public:
  ~Grave() {
  }

  string getDescription(const bool DEFINITE_ARTICLE) const;

  void setInscription(const string& str) {
    inscription_ = str;
  }

private:
  string inscription_;

  friend class FeatureFactory;
  Grave(Feature_t id, coord pos, Engine* engine);
};

#endif
