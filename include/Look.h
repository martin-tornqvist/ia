#ifndef LOOK_H
#define LOOK_H

#include "CmnTypes.h"

class Actor;
class Item;
class Mob;
class Rigid;
class Feature;

struct Entity {
public:
  Entity() :
    actor(nullptr), item(nullptr), feature(nullptr),
    entityType(entityRigid) {}

  Entity(Actor* actor_) : actor(actor_), entityType(entityActor) {}

  Entity(Item* item_) : item(item_), entityType(entityItem) {}

  Entity(Mob* feature_);
  Entity(Rigid* feature_);

  const Actor* actor;
  Item* item;
  const Feature* feature;

  EntityType entityType;
};

namespace AutoDescrActor {

void addAutoDescriptionLines(Actor* const actor, std::string& line);

} //AutoDescrActor

namespace Look {

void onMarkerAtPos(const Pos& pos, const MarkerTask markerTask,
                   const Item* const itemThrown);

void printExtraActorDescription(const Pos& pos);

} //Look

#endif
