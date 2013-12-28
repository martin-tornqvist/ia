#ifndef LOOK_H
#define LOOK_H

#include "CommonTypes.h"

class Engine;
class Actor;
class Item;
class FeatureMob;
class FeatureStatic;
class Feature;

struct Entity {
public:
  Entity() :
    actor(NULL), item(NULL), feature(NULL),
    entityType(entityFeatureStatic) {}

  Entity(Actor* actor_) :
    actor(actor_), entityType(entityActor) {
  }

  Entity(Item* item_) :
    item(item_), entityType(entityItem) {
  }

  Entity(FeatureMob* feature_);
  Entity(FeatureStatic* feature_);

  const Actor* actor;
  Item* item;
  const Feature* feature;

  Entity_t entityType;
};

class Look {
public:
  Look(Engine& engine) :
    eng(engine) {
  }

  //Returns true if a message is printed (redraws and updates everything)
  bool markerAtPos(const Pos& pos, const MarkerTask_t markerTask,
                   const Item* const itemThrown);
  void printExtraActorDescription(const Pos& pos) const;

private:
  Entity entityDescribed;

  void describeBriefActor(const Actor& actor, const MarkerTask_t markerTask,
                          const Item* const itemThrown) const;
  void describeBriefFeatureMob(const Feature& feature) const;
  void describeBriefFeatureStatic(const Feature& feature) const;
  void describeBriefItem(const Item& item) const;

  Entity getEntityToDescribe(const Pos pos);

  Engine& eng;
};

#endif
