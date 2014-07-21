#ifndef FEATURE_EXAMINABLE_H
#define FEATURE_EXAMINABLE_H

#include "Feature.h"

#include "FeatureDoor.h"

class Item;

class ItemContainerFeature {
public:
  ItemContainerFeature();

  ~ItemContainerFeature();

  void setRandomItemsForFeature(const FeatureId featureId,
                                const int NR_ITEMS_TO_ATTEMPT);

  void dropItems(const Pos& pos);

  void destroySingleFragile();

  std::vector<Item*> items_;
};

enum class TombTrait {
  stench,                 //Fumes, Ooze-type monster
  auraOfUnrest,           //Ghost-type monster
  forebodingCarvedSigns,  //Cursed
  END
};

enum class TombAppearance {
  common,
  ornate,     //Good items
  marvelous,  //Excellent items
  END
};

class Tomb: public FeatureStatic {
public:
  Tomb(const Pos& pos);
  Tomb() = delete;
  ~Tomb() {}

  FeatureId getId() const override {return FeatureId::tomb;}

  void bump(Actor& actorBumping)  override;
  bool open()                     override;
  void examine()                  override;

  std::string getName(const bool DEFINITE_ARTICLE)  const override;
  Clr getClr()                            const override;

private:
  void triggerTrap(Actor& actor) override;

  void trySprainPlayer();

  bool isContentKnown_, isTraitKnown_;

  ItemContainerFeature itemContainer_;

  int pushLidOneInN_;
  TombAppearance appearance_;
  TombTrait trait_;
};

enum class ChestMatl {wood, iron, END};

class Chest: public FeatureStatic {
public:
  Chest(const Pos& pos);
  Chest() = delete;
  ~Chest() {}

  FeatureId getId() const override {return FeatureId::chest;}

  void bump(Actor& actorBumping)  override;
  bool open()                     override;
  void examine()                  override;
  void disarm()                   override;

  Clr getClr() const override {
    return matl == ChestMatl::wood ? clrBrownDrk : clrGray;
  }

  std::string getName(const bool DEFINITE_ARTICLE) const override {
    const std::string a = DEFINITE_ARTICLE ?
                          "the " : (matl == ChestMatl::wood ? "a " : "an ");

    return a + (matl == ChestMatl::wood ? "wooden " : "iron ") + "chest";
  }

private:
  void triggerTrap(Actor& actor) override;

  void trySprainPlayer();

  ItemContainerFeature itemContainer_;

  bool isContentKnown_;
  bool isLocked_, isTrapped_, isTrapStatusKnown_;
  ChestMatl matl;
};

class Cabinet: public FeatureStatic {
public:
  Cabinet(const Pos& pos);
  Cabinet() = delete;
  ~Cabinet() {}

  FeatureId getId() const override {return FeatureId::cabinet;}

  void bump(Actor& actorBumping) override;
  bool open()                     override;

private:
  ItemContainerFeature itemContainer_;
  bool isContentKnown_;
};

enum class FountainType {
  dry, //This must be at first position!
  tepid,
  refreshing,
  bless,
  curse,
  spirit,
  vitality,
  disease,
  poison,
  frenzy,
  paralyze,
  blind,
  faint,
  rFire,
  rCold,
  rElec,
  rFear,
  rConfusion,
  END
};

enum class FountainMatl {stone, gold};

class Fountain: public FeatureStatic {
public:
  Fountain(const Pos& pos);
  Fountain() = delete;
  ~Fountain() {}

  FeatureId getId() const override {return FeatureId::fountain;}

  void bump(Actor& actorBumping)                          override;
  Clr getClr()                                const override;
  std::string getName(const bool DEFINITE_ARTICLE) const override;

private:
  FountainType fountainType_;
  FountainMatl fountainMatl_;
};

class Cocoon: public FeatureStatic {
public:
  Cocoon(const Pos& pos);
  Cocoon() = delete;
  ~Cocoon() {}

  FeatureId getId() const override {return FeatureId::cocoon;}

  void bump(Actor& actorBumping)  override;
  bool open()                     override;

private:
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
