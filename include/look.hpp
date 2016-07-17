#ifndef LOOK_HPP
#define LOOK_HPP

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

} //AutoDescrActor

namespace look
{

void print_location_info_msgs(const P& pos);

void print_detailed_actor_descr(const Actor& actor);

} //look

#endif
