#ifndef FEATURE_EXAMINABLE_H
#define FEATURE_EXAMINABLE_H

#include "Feature.h"

#include "FeatureDoor.h"

class Item;

class ItemContainerFeature {
public:
  ItemContainerFeature();

  ~ItemContainerFeature();

  void setRandomItemsForFeature(const Feature_t featureId,
                                const int NR_ITEMS_TO_ATTEMPT,
                                Engine& engine);

  void dropItems(const Pos& pos, Engine& engine);

  void destroySingleFragile(Engine& engine);

  vector<Item*> items_;
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

class Tomb: public FeatureStatic {
public:

  void bump(Actor& actorBumping)  override;
  bool open()                     override;
  void examine()                  override;
//  void bash(Actor& actorTrying)   override;
//  void disarm()                   override;

private:
  friend class FeatureFactory;
  Tomb(Feature_t id, Pos pos, Engine& engine);

  void triggerTrap(Actor& actor) override;

  void trySprainPlayer();

  bool isContentKnown_, isTraitKnown_;

  ItemContainerFeature itemContainer_;

  int pushLidOneInN_;
  TombAppearance_t appearance_;
  TombTraits_t trait_;
};

class Chest: public FeatureStatic {
public:
  ~Chest() {}

  void bump(Actor& actorBumping)  override;
  bool open()                     override;
  void examine()                  override;
  void bash(Actor& actorTrying)   override;
  void disarm()                   override;

private:
  friend class FeatureFactory;
  Chest(Feature_t id, Pos pos, Engine& engine);

  void triggerTrap(Actor& actor) override;

  void trySprainPlayer();

  ItemContainerFeature itemContainer_;

  bool isContentKnown_;
  bool isLocked_, isTrapped_, isTrapStatusKnown_;
};

class Cabinet: public FeatureStatic {
public:
  ~Cabinet() {}

  void bump(Actor& actorBumping) override;
  bool open()                     override;

private:
  friend class FeatureFactory;
  Cabinet(Feature_t id, Pos pos, Engine& engine);

  ItemContainerFeature itemContainer_;
  bool isContentKnown_;
};

enum FountainType_t {
  fountainTypeDry, //This must be kept as first position!
  fountainTypeTepid,
  fountainTypeRefreshing,
  fountainTypeBlessed,
  fountainTypeCursed,
  fountainTypeSpirited,
  fountainTypeVitality,
  fountainTypeDiseased,
  fountainTypePoisoned,
  fountainTypeFrenzy,
  endOfFountainTypes
};

class Fountain: public FeatureStatic {
public:
  ~Fountain() {}

  void bump(Actor& actorBumping)  override;

private:
  friend class FeatureFactory;
  Fountain(Feature_t id, Pos pos, Engine& engine);

  FountainType_t fountainType;
};

class Cocoon: public FeatureStatic {
public:
  ~Cocoon() {}

  void bump(Actor& actorBumping)  override;
  bool open()                     override;

private:
  friend class FeatureFactory;
  Cocoon(Feature_t id, Pos pos, Engine& engine);

  void triggerTrap(Actor& actor) override;

  bool isContentKnown_;

  ItemContainerFeature itemContainer_;
};

//class Altar: public FeatureStatic {
//};

//class CarvedPillar: public FeatureStatic {
//};

//class Barrel: public FeatureStatic {
//};

#endif
