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

class Cabinet: public FeatureStatic {
public:
  ~Cabinet() {
  }
  void examine();
private:
  friend class FeatureFactory;
  Cabinet(Feature_t id, coord pos, Engine* engine);
};

class Chest: public FeatureStatic {
public:
  ~Chest() {
  }
  void examine();
private:
  friend class FeatureFactory;
  Chest(Feature_t id, coord pos, Engine* engine);
};

class Cocoon: public FeatureStatic {
public:
  ~Cocoon() {
  }
  void examine();
private:
  friend class FeatureFactory;
  Cocoon(Feature_t id, coord pos, Engine* engine);
};

class Altar: public FeatureStatic {
public:
  ~Altar() {
  }
  void examine();
private:
  friend class FeatureFactory;
  Altar(Feature_t id, coord pos, Engine* engine);
};

class Pillar: public FeatureStatic {
public:
  ~Pillar() {
  }
  void examine();
private:
  friend class FeatureFactory;
  Pillar(Feature_t id, coord pos, Engine* engine);
};

class CarvedPillar: public FeatureStatic {
public:
  ~CarvedPillar() {
  }
  void examine();
private:
  friend class FeatureFactory;
  CarvedPillar(Feature_t id, coord pos, Engine* engine);
};

class Barrel: public FeatureStatic {
public:
  ~Barrel() {
  }
  void examine();
private:
  friend class FeatureFactory;
  Barrel(Feature_t id, coord pos, Engine* engine);
};

#endif
