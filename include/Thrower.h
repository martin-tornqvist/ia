#ifndef THROWER_H
#define THROWER_H


class Item;
class Actor;
struct Pos;

namespace Throwing {

void throwItem(Actor& actorThrowing, const Pos& targetCell, Item& itemThrown);

void playerThrowLitExplosive(const Pos& aimCell);

} //Throwing

#endif
