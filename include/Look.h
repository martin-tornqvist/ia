#ifndef LOOK_H
#define LOOK_H

#include "CmnTypes.h"

class Actor;
class Item;
class Mob;
class Rigid;
class Feature;

namespace AutoDescrActor {

void addAutoDescriptionLines(Actor* const actor, std::string& line);

} //AutoDescrActor

namespace Look {

void printLocationInfoMsgs(const Pos& pos);

void printDetailedActorDescr(const Pos& pos);

} //Look

#endif
