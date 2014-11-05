#ifndef THROWING_H
#define THROWING_H


class Item;
class Actor;
struct Pos;

namespace Throwing
{

void throwItem(Actor& actorThrowing, const Pos& tgtCell, Item& itemThrown);

void playerThrowLitExplosive(const Pos& aimCell);

} //Throwing

#endif
