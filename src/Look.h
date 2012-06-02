#ifndef LOOK_H
#define LOOK_H

/*---------------------------------------------------------------------
 Purpose: Recieveing a coordinate from the marker, this class prints a
 monster/feature/item-line in the log-space. It can also a command from
 the marker to print a "pop-up" describing the thing in greater detail.
 Some of this information is auto-generated, by the Look class, i.e.
 text of the "It normally appears beneath depth 5"-variety.
 ------------------------------------------------------------------*/

#include "ConstTypes.h"

class Engine;
class Actor;
class Item;
class FeatureMob;
class FeatureStatic;
class Feature;

enum EntityType_t {
	entityType_actor, entityType_item, entityType_featureStatic, entityType_featureMob
};

struct Entity {
public:
	Entity() :
		actor(NULL), item(NULL), feature(NULL), entityType(entityType_featureStatic) {
	}

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

	void markerAtCoord(const coord pos);
	void printExtraActorDescription(const coord pos) const;

private:
	Entity entityDescribed;

	void describeBriefActor(const Actor* const actor) const;
	void describeBriefFeatureMob(const Feature* const feature) const;
	void describeBriefFeatureStatic(const Feature* const feature) const;
	void describeBriefItem(Item* const item) const;

	Entity getEntityToDescribe(const coord pos);

	Engine* eng;
};

#endif
