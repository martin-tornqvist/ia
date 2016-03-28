#ifndef LOOK_H
#define LOOK_H

#include <string>

#include "rl_utils.hpp"

class Actor;
class Item;
class Mob;
class Rigid;
class Feature;

namespace auto_descr_actor
{

void add_auto_description_lines(const Actor& actor, std::string& line);

} //Auto_descr_actor

namespace look
{

void print_location_info_msgs(const P& pos);

void print_detailed_actor_descr(const Actor& actor);

} //look

#endif
