#ifndef RELOAD_H
#define RELOAD_H

class Actor;

enum class Reload_result
{
    success,
    no_ammo,
    not_carrying_wpn,
    already_full,
    wpn_not_using_ammo,
    fumble
};

namespace reload
{

bool reload_wielded_wpn(Actor& actor_reloading);

} //Reload

#endif
