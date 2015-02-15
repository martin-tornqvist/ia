#ifndef LOOK_H
#define LOOK_H

#include "CmnTypes.h"

class Actor;
class Item;
class Mob;
class Rigid;
class Feature;

namespace AutoDescrActor
{

void addAutoDescriptionLines(const Actor& actor, std::string& line);

} //AutoDescrActor

namespace Look
{

void printLocationInfoMsgs(const Pos& pos);

void printDetailedActorDescr(const Actor& actor);

} //Look

#endif
