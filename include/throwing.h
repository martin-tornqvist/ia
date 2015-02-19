#ifndef THROWING_H
#define THROWING_H


class Item;
class Actor;
struct Pos;

namespace Throwing
{

void throw_item(Actor& actor_throwing, const Pos& tgt_cell, Item& item_thrown);

void player_throw_lit_explosive(const Pos& aim_cell);

} //Throwing

#endif
