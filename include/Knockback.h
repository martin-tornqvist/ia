#ifndef KNOCKBACK_H
#define KNOCKBACK_H

#include "CommonTypes.h"

class Engine;
class Actor;
struct AttackData;

class KnockBack {
public:
  KnockBack(Engine& engine) : eng(engine) {}

  void tryKnockBack(Actor& defender, const Pos& attackedFromPos,
                    const bool IS_SPIKE_GUN,
                    const bool IS_MSG_ALLOWED = true);

private:
  Engine& eng;

};

#endif
