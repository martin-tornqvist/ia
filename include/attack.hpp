#ifndef ATTACK_HPP
#define ATTACK_HPP

#include "global.hpp"

class Actor;
class P;
class Wpn;

namespace attack
{

// NOTE: Attacker origin is needed since attacker may be a null pointer.
void melee(Actor* const attacker,
           const P& attacker_origin,
           Actor& defender,
           Wpn& wpn);

DidAction ranged(Actor* const attacker,
                 const P& origin,
                 const P& aim_pos,
                 Wpn& wpn);

void ranged_hit_chance(const Actor& attacker,
                       const Actor& defender,
                       const Wpn& wpn);

} // attack

#endif // ATTACK_HPP
