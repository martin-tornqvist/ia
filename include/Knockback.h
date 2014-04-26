#ifndef KNOCKBACK_H
#define KNOCKBACK_H

#include "CmnTypes.h"

class Actor;
class AttackData;

class KnockBack {
public:
  KnockBack() {}

  void tryKnockBack(Actor& defender, const Pos& attackedFromPos,
                    const bool IS_SPIKE_GUN,
                    const bool IS_MSG_ALLOWED = true);
};

#endif
