#ifndef LOOK_H
#define LOOK_H

#include "cmn_types.hpp"

class Actor;
class Item;
class Mob;
class Rigid;
class Feature;

namespace Auto_descr_actor
{

void add_auto_description_lines(const Actor& actor, std::string& line);

} //Auto_descr_actor

namespace Look
{

void print_location_info_msgs(const Pos& pos);

void print_detailed_actor_descr(const Actor& actor);

} //Look

#endif
