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
    actor(nullptr), item(nullptr), feature(nullptr), entityType(EntityType::rigid) {}

  Entity(Actor* actor_) : actor(actor_),  entityType(EntityType::actor) {}
  Entity(Item* item_)   : item(item_),    entityType(EntityType::item)  {}

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

void printLocationInfoMsgs(const Pos& pos);

void printDetailedActorDescr(const Pos& pos);

} //Look

#endif
