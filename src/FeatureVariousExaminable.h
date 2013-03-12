#ifndef FEATURE_VARIOUS_EXAMINABLE_H
#define FEATURE_VARIOUS_EXAMINABLE_H

#include "Feature.h"

class FeatureExaminable: public FeatureStatic {
public:
  ~FeatureExaminable() {
  }

  void examine();

//  sf::Color getColorBg() const;

  bool isExaminableFurther() const {
    return isExaminableFurther_;
  }

protected:
  virtual void featureSpecific_examine() = 0;

  bool isExaminableFurther_;

  friend class FeatureFactory;
  FeatureExaminable(Feature_t id, coord pos, Engine* engine);
};

class Tomb: public FeatureExaminable {
public:
  ~Tomb() {
  }
  void featureSpecific_examine();

private:
  friend class FeatureFactory;
  Tomb(Feature_t id, coord pos, Engine* engine);
};

class Cabinet: public FeatureExaminable {
public:
  ~Cabinet() {
  }
  void featureSpecific_examine();
private:
  friend class FeatureFactory;
  Cabinet(Feature_t id, coord pos, Engine* engine);
};

class Chest: public FeatureExaminable {
public:
  ~Chest() {
  }
  void featureSpecific_examine();
private:
  friend class FeatureFactory;
  Chest(Feature_t id, coord pos, Engine* engine);
};

class Cocoon: public FeatureExaminable {
public:
  ~Cocoon() {
  }
  void featureSpecific_examine();
private:
  friend class FeatureFactory;
  Cocoon(Feature_t id, coord pos, Engine* engine);
};

class Altar: public FeatureExaminable {
public:
  ~Altar() {
  }
  void featureSpecific_examine();
private:
  friend class FeatureFactory;
  Altar(Feature_t id, coord pos, Engine* engine);
};

class CarvedPillar: public FeatureExaminable {
public:
  ~CarvedPillar() {
  }
  void featureSpecific_examine();
private:
  friend class FeatureFactory;
  CarvedPillar(Feature_t id, coord pos, Engine* engine);
};

class Barrel: public FeatureExaminable {
public:
  ~Barrel() {
  }
  void featureSpecific_examine();
private:
  friend class FeatureFactory;
  Barrel(Feature_t id, coord pos, Engine* engine);
};

#endif
