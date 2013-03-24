#ifndef FEATURE_VARIOUS_EXAMINABLE_H
#define FEATURE_VARIOUS_EXAMINABLE_H

#include "Feature.h"

#include "FeatureDoor.h"

enum EventRegularity_t {
  eventRegularity_common = 100,
  eventRegularity_rare = 10,
  eventRegularity_veryRare = 1
};

class Item;

class FeatureExaminable: public FeatureStatic {
public:
  ~FeatureExaminable() {
  }

  void examine();

//  sf::Color getColorBg() const;

//  bool isExaminableFurther() const {
//    return isExaminableFurther_;
//  }

protected:
  virtual void featureSpecific_examine() = 0;

  EventRegularity_t getEventRegularity();

//  bool isExaminableFurther_;

  friend class FeatureFactory;
  FeatureExaminable(Feature_t id, coord pos, Engine* engine);
};

class ExaminableItemContainer {
public:
  ExaminableItemContainer();

  ~ExaminableItemContainer();

  void setRandomItemsForFeature(const Feature_t featureId, const int NR_ITEMS_TO_ATTEMPT, Engine* const engine);

  void dropItems(const coord& pos, Engine* const engine);

  vector<Item*> items_;
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

  ExaminableItemContainer itemContainer;
};

enum ChestAction_t {
  chestAction_open,
  chestAction_searchForTrap,
  chestAction_disarmTrap,
  chestAction_forceLock,
  chestAction_kick,
  chestAction_leave
};

class Chest: public FeatureExaminable {
public:
  ~Chest() {
  }
  void featureSpecific_examine();

//  string getDescription(const bool DEFINITE_ARTICLE) const;

//  sf::Color getColor() const;

private:
  friend class FeatureFactory;
  Chest(Feature_t id, coord pos, Engine* engine);

  void triggerTrap();

  void setChoiceLabelsFromPossibleActions(const vector<ChestAction_t>& possibleActions, vector<string>& actionLabels) const;

  void setDescription(string& description) const;

  bool isContentKnown_, isLocked_, isTrapped_, isTrapStatusKnown_;

  ExaminableItemContainer itemContainer_;
};

class Cocoon: public FeatureExaminable {
public:
  ~Cocoon() {
  }
  void featureSpecific_examine();
private:
  friend class FeatureFactory;
  Cocoon(Feature_t id, coord pos, Engine* engine);

  ExaminableItemContainer itemContainer;
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

  ExaminableItemContainer itemContainer;
};

#endif
