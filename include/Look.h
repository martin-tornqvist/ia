#ifndef LOOK_H
#define LOOK_H

#include "CommonTypes.h"

class Engine;
class Actor;
class Item;
class FeatureMob;
class FeatureStatic;
class Feature;

enum EntityType_t {
  entityType_actor, entityType_item, entityType_featureStatic,
  entityType_featureMob
};

struct Entity {
public:
  Entity() :
    actor(NULL), item(NULL), feature(NULL),
    entityType(entityType_featureStatic) {}

  Entity(Actor* actor_) :
    actor(actor_), entityType(entityType_actor) {
  }

  Entity(Item* item_) :
    item(item_), entityType(entityType_item) {
  }

  Entity(FeatureMob* feature_);
  Entity(FeatureStatic* feature_);

  const Actor* actor;
  Item* item;
  const Feature* feature;

  EntityType_t entityType;
};

class Look {
public:
  Look(Engine* engine) :
    eng(engine) {
  }

  void markerAtPos(const Pos& pos, const MarkerTask_t markerTask,
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

  Engine* eng;
};

#endif
