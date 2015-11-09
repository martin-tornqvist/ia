#ifndef RELOAD_H
#define RELOAD_H

class Actor;
class Item;

namespace reload
{

void try_reload(Actor& actor, Item* const item_to_reload);

void player_arrange_pistol_mags();

} //reload

#endif
