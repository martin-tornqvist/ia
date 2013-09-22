#ifndef FEATURE_EXAMINABLE_H
#define FEATURE_EXAMINABLE_H

#include "Feature.h"

#include "FeatureDoor.h"

enum EventRegularity_t {
  eventRegularity_common    = 100,
  eventRegularity_rare      = 10,
  eventRegularity_veryRare  = 1
};

class Item;

class FeatureExaminable: public FeatureStatic {
public:
  ~FeatureExaminable() {}
  void examine();

protected:
  virtual void featureSpecific_examine() = 0;

  EventRegularity_t getEventRegularity();

  friend class FeatureFactory;
  FeatureExaminable(Feature_t id, Pos pos, Engine* engine);
};

class ExaminableItemContainer {
public:
  ExaminableItemContainer();

  ~ExaminableItemContainer();

  void setRandomItemsForFeature(const Feature_t featureId,
                                const int NR_ITEMS_TO_ATTEMPT,
                                Engine* const engine);

  void dropItems(const Pos& pos, Engine* const engine);

  void destroySingleFragile(Engine* const engine);

  vector<Item*> items_;
};

enum TombAction_t {
  tombAction_pushLid,
  tombAction_smashLidWithSledgehammer,
  tombAction_searchExterior,
  tombAction_carveCurseWard,
  tombAction_leave
};

enum TombTraits_t {
  tombTrait_stench,                 //Fumes, Ooze-type monster
  tombTrait_auraOfUnrest,           //Ghost-type monster
  tombTrait_forebodingCarvedSigns,  //Cursed
  endOfTombTraits
};

enum TombAppearance_t {
  tombAppearance_common,
  tombAppearance_impressive,  //Good items
  tombAppearance_marvelous,   //Excellent items
  endOfTombAppearance
};

class Tomb: public FeatureExaminable {
public:
  ~Tomb() {}
  void featureSpecific_examine();

private:
  friend class FeatureFactory;

  Tomb(Feature_t id, Pos pos, Engine* engine);

  bool openFeature();
  void triggerTrap();
  void getChoiceLabels(const vector<TombAction_t>& possibleActions,
                       vector<string>& actionLabels) const;
  void doAction(const TombAction_t action);
  void getPossibleActions(vector<TombAction_t>& possibleActions) const;
  void getDescr(string& descr) const;
  bool isContentKnown_, isTraitKnown_;

  ExaminableItemContainer itemContainer_;

  void getTraitDescr(string& descr) const;
  int chanceToPushLid_;
  TombAppearance_t appearance_;
  TombTraits_t trait_;
};

enum CabinetAction_t {
  cabinetAction_open,
  cabinetAction_leave
};

class Cabinet: public FeatureExaminable {
public:
  ~Cabinet() {}
  void featureSpecific_examine();
private:
  friend class FeatureFactory;
  Cabinet(Feature_t id, Pos pos, Engine* engine);

  void triggerTrap();
  bool openFeature();
  void getChoiceLabels(const vector<CabinetAction_t>& possibleActions,
                       vector<string>& actionLabels) const;
  void getPossibleActions(vector<CabinetAction_t>& possibleActions) const;
  void doAction(const CabinetAction_t action);
  void getDescr(string& descr) const;
  ExaminableItemContainer itemContainer_;
  bool isContentKnown_;
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
  ~Chest() {}
  void featureSpecific_examine();

//  string getDescr(const bool DEFINITE_ARTICLE) const;

//  SDL_Color getColor() const;

private:
  friend class FeatureFactory;
  Chest(Feature_t id, Pos pos, Engine* engine);

  void triggerTrap();
  bool openFeature();
  void getChoiceLabels(const vector<ChestAction_t>& possibleActions,
                       vector<string>& actionLabels) const;
  void getPossibleActions(vector<ChestAction_t>& possibleActions) const;
  void doAction(const ChestAction_t action);
  void getDescr(string& descr) const;
  ExaminableItemContainer itemContainer_;

  bool isContentKnown_;
  bool isLocked_, isTrapped_, isTrapStatusKnown_;
};

enum FountainType_t {
  fountainTypeDry, //This must be kept as first position!
  fountainTypeTepid,
  fountainTypeRefreshing,
  fountainTypeBlessed,
  fountainTypeCursed,
  fountainTypeSpirited,
  fountainTypeVitality,
//  fountainTypeTainted,
  fountainTypeDiseased,
  fountainTypePoisoned,
  fountainTypeBerserk,
//  fountainTypeVisions,
  endOfFountainTypes
};

class Fountain: public FeatureExaminable {
public:
  ~Fountain() {}
  void featureSpecific_examine();

private:
  friend class FeatureFactory;
  Fountain(Feature_t id, Pos pos, Engine* engine);

  void drink();

  FountainType_t fountainType;
};

enum CocoonAction_t {
  cocoonAction_open,
  cocoonAction_leave
};

class Cocoon: public FeatureExaminable {
public:
  ~Cocoon() {}
  void featureSpecific_examine();
private:
  friend class FeatureFactory;
  Cocoon(Feature_t id, Pos pos, Engine* engine);

  void triggerTrap();
  bool openFeature();
  void getChoiceLabels(const vector<CocoonAction_t>& possibleActions,
                       vector<string>& actionLabels) const;
  void getPossibleActions(vector<CocoonAction_t>& possibleActions) const;
  void doAction(const CocoonAction_t action);
  void getDescr(string& descr) const;
  ExaminableItemContainer itemContainer_;
  bool isContentKnown_;
};

//class Altar: public FeatureExaminable {
//public:
//  ~Altar() {}
//  void featureSpecific_examine();
//private:
//  friend class FeatureFactory;
//  Altar(Feature_t id, Pos pos, Engine* engine);
//};

//class CarvedPillar: public FeatureExaminable {
//public:
//  ~CarvedPillar() {}
//  void featureSpecific_examine();
//private:
//  friend class FeatureFactory;
//  CarvedPillar(Feature_t id, Pos pos, Engine* engine);
//};

//class Barrel: public FeatureExaminable {
//public:
//  ~Barrel() {}
//  void featureSpecific_examine();
//private:
//  friend class FeatureFactory;
//  Barrel(Feature_t id, Pos pos, Engine* engine);
//
//  ExaminableItemContainer itemContainer;
//};

#endif
