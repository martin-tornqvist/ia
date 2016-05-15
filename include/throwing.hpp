#ifndef THROWING_HPP
#define THROWING_HPP


class Item;
class Actor;
class P;

namespace throwing
{

void throw_item(Actor& actor_throwing, const P& tgt_cell, Item& item_thrown);

void player_throw_lit_explosive(const P& aim_cell);

} //Throwing

#endif
